#include <nds.h>

#include "Cart.h"
#include "Equates.h"
#include "Gfx.h"

#define GG_ACTIVE_X             48
#define GG_ACTIVE_Y             24
#define GG_VERTICAL_SCALE_NUMERATOR       4
#define GG_VERTICAL_SCALE_DENOMINATOR     3
#define GG_FULL_SCREEN_HORIZONTAL_STEP 0xA0
#define GG_EXT_FOREGROUND_TILE_BASE     0
#define GG_EXT_BACKGROUND_TILE_BASE     4
#define GG_EXT_FOREGROUND_MAP_BASE     16
#define GG_EXT_BACKGROUND_MAP_BASE     17
#define GG_EXT_FOREGROUND_TILE_ADDRESS ((vu16 *)((u8 *)BG_GFX + 0x20000))
#define GG_EXT_FOREGROUND_MAP_ADDRESS  ((vu16 *)((u8 *)BG_GFX + 0x28000))
#define GG_EXT_BACKGROUND_MAP_ADDRESS  ((vu16 *)((u8 *)BG_GFX + 0x28800))
#define GG_EXT_BACKGROUND_TILE_ADDRESS ((vu16 *)((u8 *)BG_GFX + 0x30000))
#define GG_SMOOTH_BITMAP_C_BASE        16
#define GG_SMOOTH_BITMAP_D_BASE        24
#define GG_BACKGROUND_TILE_BYTES       0x4000
#define GG_SPRITE_TILE_BYTES           0x8000
#define GG_BACKGROUND_MAP_BYTES        0x1000

#define DMA0_SOURCE_REG         (*(vu32 *)0x040000B0)
#define DMA0_DEST_REG           (*(vu32 *)0x040000B4)
#define DMA0_CONTROL_REG        (*(vu32 *)0x040000B8)
#define MAIN_BG_REG_BASE        ((vu32 *)0x04000010)

// One HBlank transfer covers BG0 scrolling plus the affine registers for BG2
// and BG3. The normal renderer's DMA buffer remains the source of truth for
// per-scanline scrolling; this buffer translates those values to affine form.
static u32 affineDmaBuffer[SCREEN_HEIGHT][12] __attribute__((aligned(4)));
static u32 packedTileCache[0x1000] __attribute__((aligned(4)));
static u16 foregroundExpansion[0x100];
static u16 backgroundExpansion[0x100];
static u16 paletteCache[0x100];
static DTCM_DATA u16 ggSourceLines[2][GAME_WIDTH_GG]
	__attribute__((aligned(4)));
static DTCM_DATA u16 ggHorizontalLines[2][SCREEN_WIDTH]
	__attribute__((aligned(4)));
static u8 smoothedBackgroundTiles[GG_BACKGROUND_TILE_BYTES]
	__attribute__((aligned(32)));
static u8 smoothedSpriteTiles[GG_SPRITE_TILE_BYTES]
	__attribute__((aligned(32)));
static u16 smoothedBackgroundMaps[GG_BACKGROUND_MAP_BYTES / sizeof(u16)]
	__attribute__((aligned(32)));
static bool ggFullscreenWasActive;
static bool expansionTablesReady;
static bool tileCacheValid;
static bool paletteCacheValid;
static bool affineBufferReady;
static volatile int smoothedReadyBitmapBase = -1;
static volatile int smoothedDisplayedBitmapBase = -1;
static bool smoothed2VideoInitialised;
static bool smoothedCacheInitialised;
static bool smoothed2BanksActive;
static bool smoothed2FilterPass;
static bool smoothed2FilteredReady;
static volatile bool smoothed2DrawPending;

static bool ggFullscreenActive(void) {
	u32 tileAddress = (u32)getVDP0BgrTileAddress();
	return (gScalingSet == SCALED_GG_FULL_SCREEN
			|| gScalingSet == SCALED_GG_FULL_SCREEN_SMOOTHED
			|| gScalingSet == SCALED_GG_FULL_SCREEN_SMOOTHED2)
		&& (gEmuFlags & GG_MODE)
		&& tileAddress >= (u32)BG_GFX
		&& tileAddress < (u32)BG_GFX + 0x20000;
}

static bool ggCpuSmoothedMode(void) {
	return gScalingSet == SCALED_GG_FULL_SCREEN_SMOOTHED;
}

static bool ggHardwareSmoothedMode(void) {
	return gScalingSet == SCALED_GG_FULL_SCREEN_SMOOTHED2;
}

static void expandBackgroundTiles(void) {
	const vu32 *src = (const vu32 *)getVDP0BgrTileAddress();
	vu32 *foreground = (vu32 *)GG_EXT_FOREGROUND_TILE_ADDRESS;
	vu32 *background = (vu32 *)GG_EXT_BACKGROUND_TILE_ADDRESS;

	if (!expansionTablesReady) {
		for (int pixels = 0; pixels < 0x100; pixels++) {
			u8 low = pixels & 0x0F;
			u8 high = pixels >> 4;
			foregroundExpansion[pixels] = low | (high << 8);
			backgroundExpansion[pixels] = (low ? low : 16)
				| ((high ? high : 16) << 8);
		}
		expansionTablesReady = true;
	}

	// Mode 4 has 512 8x8 4-bpp tiles (16 KiB). Extended rotation backgrounds
	// require 8-bpp tiles, so expand each packed pair of pixels. The foreground
	// priority layer keeps color 0 transparent. The behind-sprites layer uses
	// index 16 for those pixels so its palette can supply the GG color-0 fill.
	// Cache packed words so static tiles don't consume most of VBlank every
	// frame; games normally update only a small subset of the pattern table.
	for (int i = 0; i < 0x1000; i++) {
		u32 packed = src[i];
		if (tileCacheValid && packedTileCache[i] == packed) {
			continue;
		}
		packedTileCache[i] = packed;
		foreground[i * 2] = foregroundExpansion[packed & 0xFF]
			| ((u32)foregroundExpansion[(packed >> 8) & 0xFF] << 16);
		foreground[i * 2 + 1] = foregroundExpansion[(packed >> 16) & 0xFF]
			| ((u32)foregroundExpansion[packed >> 24] << 16);
		background[i * 2] = backgroundExpansion[packed & 0xFF]
			| ((u32)backgroundExpansion[(packed >> 8) & 0xFF] << 16);
		background[i * 2 + 1] = backgroundExpansion[(packed >> 16) & 0xFF]
			| ((u32)backgroundExpansion[packed >> 24] << 16);
	}
	tileCacheValid = true;

	// The priority map uses tile 1023 as its transparent sentinel.
	vu16 *transparent = GG_EXT_FOREGROUND_TILE_ADDRESS + 1023 * 32;
	for (int i = 0; i < 32; i++) {
		transparent[i] = 0;
	}
}

static void convertBackgroundMaps(void) {
	u32 mapOffset = getVDP0BgrMapOffset() & 0x1F00;
	const u16 *foreground = (const u16 *)((const u8 *)BG_GFX + mapOffset * 8);

	// Copy the 16-bit text maps into bank B so the display can use independent
	// screen and character bases in extended-affine mode. The entries already
	// contain tile-relative indexes; their palette and flip bits have the same
	// layout in both formats. Tile 1023 remains the transparent foreground
	// sentinel prepared by expandBackgroundTiles().
	dmaCopyWords(3, foreground, (void *)GG_EXT_FOREGROUND_MAP_ADDRESS, 0x1000);
}

static void updateExtendedPalettes(void) {
	bool changed = !paletteCacheValid;
	for (int i = 0; i < 0x100; i++) {
		u16 value = EMUPALBUFF[i];
		if (paletteCache[i] != value) {
			paletteCache[i] = value;
			changed = true;
		}
	}
	if (!changed) {
		return;
	}

	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);

	vu16 *slot2 = (vu16 *)VRAM_G;
	vu16 *slot3 = slot2 + 0x1000;

	// BG2 and BG3 use the expanded 4-bpp game tiles. The background copy maps
	// its opaque index 16 to the same dedicated color-0 values S8DS uses in its
	// normal 8-bpp backdrop layer (0x30 for palette 0, 0x40 for palette 1).
	for (int palette = 0; palette < 16; palette++) {
		for (int color = 0; color < 16; color++) {
			u16 value = EMUPALBUFF[palette * 16 + color];
			int index = palette * 256 + color;
			slot2[index] = value;
			slot3[index] = value;
		}
		slot3[palette * 256 + 16] = EMUPALBUFF[(palette & 1) ? 0x40 : 0x30];
	}

	vramSetBankF(VRAM_F_BG_EXT_PALETTE_SLOT01);
	vramSetBankG(VRAM_G_BG_EXT_PALETTE_SLOT23);
	paletteCacheValid = true;
}

static u32 affineXFromTextScroll(u32 packedScroll, int horizontalStep,
								 int outputLeft) {
	u32 x = packedScroll & 0x1FF;
	// Place source x=48 exactly at the output's left edge. Keeping the fractional
	// reference point avoids sampling a hidden GG border column in full-height mode.
	return (((x + GG_ACTIVE_X) << 8) - outputLeft * horizontalStep) & 0x1FFFF;
}

static u32 affineYFromTextScroll(u32 packedScroll, int sourceLine) {
	u32 y = (packedScroll >> 16) & 0x1FF;
	return ((y + sourceLine) & 0x1FF) << 8;
}

static void buildAffineDmaBuffer(int horizontalStep, int outputLeft) {
	static int previousHorizontalStep;
	if (horizontalStep != previousHorizontalStep) {
		affineBufferReady = false;
		previousHorizontalStep = horizontalStep;
	}

	for (int line = 0; line < SCREEN_HEIGHT; line++) {
		int scaledLine = (line * GG_VERTICAL_SCALE_DENOMINATOR)
			/ GG_VERTICAL_SCALE_NUMERATOR;
		int sourceLine = GG_ACTIVE_Y + scaledLine;
		// DMA0Buff contains the normal renderer's per-source-line scroll and its
		// 224-line map-wrap correction. Read the cropped GG line, not the output's
		// zero-based line; otherwise the correction arrives 24 lines too late and
		// exposes the four unused rows at the bottom of the 32-row DS tilemap.
		const u32 *src = &DMA0Buff[sourceLine * 4];
		u32 *dst = affineDmaBuffer[line];

		if (!affineBufferReady) {
			// BG0 and BG1 are unused in mode 5.
			dst[0] = 0;
			dst[1] = 0;
			dst[2] = 0;
			dst[3] = 0;

			// PB, PC and PD are zero because each scanline receives an explicit
			// reference point. PA is the inverse horizontal enlargement.
			dst[4] = horizontalStep;
			dst[5] = 0;
			dst[8] = horizontalStep;
			dst[9] = 0;
		}

		// BG2: foreground-priority tiles.
		dst[6] = affineXFromTextScroll(src[0], horizontalStep, outputLeft);
		dst[7] = affineYFromTextScroll(src[0], sourceLine);

		// BG3: tiles behind sprites.
		dst[10] = affineXFromTextScroll(src[1], horizontalStep, outputLeft);
		dst[11] = affineYFromTextScroll(src[1], sourceLine);
	}
	affineBufferReady = true;
}

static void spriteSize(u16 attr0, u16 attr1, int *width, int *height) {
	static const u8 widths[3][4] = {
		{ 8, 16, 32, 64 },
		{ 16, 32, 32, 64 },
		{ 8, 8, 16, 32 },
	};
	static const u8 heights[3][4] = {
		{ 8, 16, 32, 64 },
		{ 8, 8, 16, 32 },
		{ 16, 32, 32, 64 },
	};
	int shape = (attr0 >> 14) & 3;
	int size = attr1 >> 14;
	*width = shape < 3 ? widths[shape][size] : 8;
	*height = shape < 3 ? heights[shape][size] : 8;
}

static u8 tilePixel(const vu8 *tiles, u16 entry, int x, int y) {
	if (entry & BIT(10)) {
		x = 7 - x;
	}
	if (entry & BIT(11)) {
		y = 7 - y;
	}
	const vu8 *tile = tiles + (entry & 0x3FF) * 32;
	u8 packed = tile[y * 4 + (x >> 1)];
	return (packed >> ((x & 1) * 4)) & 0x0F;
}

static void drawSmoothedBackgroundLine(u16 *dst, int y, const vu16 *map,
									   const vu8 *tiles, const u8 *scrollTMap,
									   int yScroll, int scrollMask,
									   bool foreground) {
	int sourceLine = GG_ACTIVE_Y + y;
	int scrollX = scrollTMap[sourceLine * 2 + 1];
	int mapY = yScroll + sourceLine;
	if (mapY >= scrollMask) {
		mapY -= scrollMask;
	}

	for (int x = 0; x < GAME_WIDTH_GG; x++) {
		int mapX = (scrollX + GG_ACTIVE_X + x) & 0xFF;
		u16 entry = map[(mapY >> 3) * 32 + (mapX >> 3)];
		if (foreground && (entry & 0x3FF) == 0x3FF) {
			continue;
		}

		u8 color = tilePixel(tiles, entry, mapX & 7, mapY & 7);
		if (foreground && color == 0) {
			continue;
		}

		int palette = (entry >> 12) & 0x0F;
		if (!foreground && color == 0) {
			dst[x] = EMUPALBUFF[(palette & 1) ? 0x40 : 0x30];
		}
		else {
			dst[x] = EMUPALBUFF[palette * 16 + color];
		}
	}
}

static void drawSmoothedSpriteLine(u16 *dst, int y, const u16 *oam,
								   const vu8 *tiles) {
	int screenY = GG_ACTIVE_Y + y;

	// Draw in reverse order so lower OAM indexes retain the DS priority rule.
	for (int i = 127; i >= 0; i--) {
		u16 attr0 = oam[i * 4];
		u16 attr1 = oam[i * 4 + 1];
		u16 attr2 = oam[i * 4 + 2];
		if ((attr0 & 0xFF) == SCREEN_HEIGHT || !(attr0 & ATTR0_ROTSCALE)) {
			continue;
		}

		int spriteX = attr1 & 0x1FF;
		if (spriteX >= 256) {
			spriteX -= 512;
		}
		int spriteY = attr0 & 0xFF;
		if (spriteY >= SCREEN_HEIGHT) {
			spriteY -= 256;
		}

		int width;
		int height;
		spriteSize(attr0, attr1, &width, &height);
		int affineIndex = (attr1 >> 9) & 0x1F;
		int zoomShift = (affineIndex & 2) ? 1 : 0;
		int zoom = 1 << zoomShift;
		int outputY = screenY - spriteY;
		if ((unsigned)outputY >= (unsigned)(height * zoom)) {
			continue;
		}
		int baseTile = attr2 & 0x3FF;
		int palette = (attr2 >> 12) & 0x0F;
		int tilesPerRow = width >> 3;
		int sourceY = outputY >> zoomShift;

		for (int outputX = 0; outputX < width * zoom; outputX++) {
			int dstX = spriteX - GG_ACTIVE_X + outputX;
			if ((unsigned)dstX >= GAME_WIDTH_GG) {
				continue;
			}
			int sourceX = outputX >> zoomShift;
			int tileNumber = baseTile + (sourceY >> 3) * tilesPerRow
				+ (sourceX >> 3);
			const vu8 *tile = tiles + tileNumber * 32;
			u8 packed = tile[(sourceY & 7) * 4 + ((sourceX & 7) >> 1)];
			u8 color = (packed >> ((sourceX & 1) * 4)) & 0x0F;
			if (color != 0) {
				dst[dstX] = EMUPALBUFF[0x100 + palette * 16 + color];
			}
		}
	}
}

static inline u16 averageRgb15(u16 first, u16 second) {
	return (first & second) + (((first ^ second) & 0x7BDE) >> 1);
}

static inline u32 averageRgb15Pair(u32 first, u32 second) {
	return (first & second) + (((first ^ second) & 0x7BDE7BDE) >> 1);
}

static inline u16 blendRgb15Eighth(u16 first, u16 second, int weight) {
	if (first == second) {
		return first;
	}

	u16 half = averageRgb15(first, second);
	switch (weight) {
	case 1: {
		u16 quarter = averageRgb15(first, half);
		return averageRgb15(first, quarter);
	}
	case 2:
		return averageRgb15(first, half);
	case 3: {
		u16 quarter = averageRgb15(first, half);
		return averageRgb15(quarter, half);
	}
	case 4:
		return half;
	case 5: {
		u16 threeQuarter = averageRgb15(half, second);
		return averageRgb15(half, threeQuarter);
	}
	case 6:
		return averageRgb15(half, second);
	default: {
		u16 threeQuarter = averageRgb15(half, second);
		return averageRgb15(threeQuarter, second);
	}
	}
}

static void composeSmoothedLine(u16 *source, int sourceY,
								const vu16 *foregroundMap,
								const vu16 *backgroundMap,
								const vu8 *backgroundTiles, const u8 *scrollTMap,
								int yScroll, int scrollMask, const u16 *oam,
								const vu8 *spriteTiles) {
	drawSmoothedBackgroundLine(source, sourceY, backgroundMap, backgroundTiles,
		scrollTMap, yScroll, scrollMask, false);
	drawSmoothedSpriteLine(source, sourceY, oam, spriteTiles);
	drawSmoothedBackgroundLine(source, sourceY, foregroundMap, backgroundTiles,
		scrollTMap, yScroll, scrollMask, true);
}

static void ITCM_CODE prepareSmoothedLine(int sourceY, int slot,
								const vu16 *foregroundMap,
								const vu16 *backgroundMap,
								const vu8 *backgroundTiles, const u8 *scrollTMap,
								int yScroll, int scrollMask, const u16 *oam,
								const vu8 *spriteTiles) {
	u16 *source = ggSourceLines[slot];
	u16 *horizontal = ggHorizontalLines[slot];
	composeSmoothedLine(source, sourceY, foregroundMap, backgroundMap,
		backgroundTiles, scrollTMap, yScroll, scrollMask, oam, spriteTiles);

	// These source coordinates match the nearest-neighbour 5:8 mapping. The
	// fractional remainder supplies the contribution from the adjacent pixel.
	for (int x = 0, sourceX = 0; x < SCREEN_WIDTH; x += 8, sourceX += 5) {
		int nextBlockPixel = sourceX < GAME_WIDTH_GG - 5
			? sourceX + 5 : GAME_WIDTH_GG - 1;
		horizontal[x] = source[sourceX];
		horizontal[x + 1] = blendRgb15Eighth(source[sourceX],
			source[sourceX + 1], 5);
		horizontal[x + 2] = blendRgb15Eighth(source[sourceX + 1],
			source[sourceX + 2], 2);
		horizontal[x + 3] = blendRgb15Eighth(source[sourceX + 1],
			source[sourceX + 2], 7);
		horizontal[x + 4] = blendRgb15Eighth(source[sourceX + 2],
			source[sourceX + 3], 4);
		horizontal[x + 5] = blendRgb15Eighth(source[sourceX + 3],
			source[sourceX + 4], 1);
		horizontal[x + 6] = blendRgb15Eighth(source[sourceX + 3],
			source[sourceX + 4], 6);
		horizontal[x + 7] = blendRgb15Eighth(source[sourceX + 4],
			source[nextBlockPixel], 3);
	}
}

static void ITCM_CODE __attribute__((noinline)) scaleSmoothedFrame(
								vu16 *dst, const vu16 *foregroundMap,
								const vu16 *backgroundMap,
								const vu8 *backgroundTiles, const u8 *scrollTMap,
								int yScroll, int scrollMask, const u16 *oam,
								const vu8 *spriteTiles) {
	int bufferedLine[2] = { -1, -1 };

	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		int fixedY = y * 3;
		int sourceY = fixedY >> 2;
		int nextY = sourceY < GAME_HEIGHT_GG - 1 ? sourceY + 1 : sourceY;
		int firstSlot = sourceY & 1;
		int secondSlot = nextY & 1;
		if (bufferedLine[firstSlot] != sourceY) {
			prepareSmoothedLine(sourceY, firstSlot, foregroundMap, backgroundMap,
				backgroundTiles, scrollTMap, yScroll, scrollMask, oam, spriteTiles);
			bufferedLine[firstSlot] = sourceY;
		}
		if (bufferedLine[secondSlot] != nextY) {
			prepareSmoothedLine(nextY, secondSlot, foregroundMap, backgroundMap,
				backgroundTiles, scrollTMap, yScroll, scrollMask, oam, spriteTiles);
			bufferedLine[secondSlot] = nextY;
		}
		const u16 *first = ggHorizontalLines[firstSlot];
		const u16 *second = ggHorizontalLines[secondSlot];
		const u32 *firstPairs = (const u32 *)first;
		const u32 *secondPairs = (const u32 *)second;
		int weight = fixedY & 3;
		vu32 *output = (vu32 *)(dst + y * SCREEN_WIDTH);
		if (weight == 0) {
			for (int pair = 0; pair < SCREEN_WIDTH / 2; pair++) {
				output[pair] = firstPairs[pair] | 0x80008000;
			}
		}
		else {
			for (int pair = 0; pair < SCREEN_WIDTH / 2; pair++) {
				u32 firstPair = firstPairs[pair];
				u32 secondPair = secondPairs[pair];
				u32 half = averageRgb15Pair(firstPair, secondPair);
				u32 blended = weight == 2 ? half
					: weight == 1 ? averageRgb15Pair(firstPair, half)
					: averageRgb15Pair(half, secondPair);
				output[pair] = blended | 0x80008000;
			}
		}
	}
}

static void renderCpuSmoothedFrame(void) {
	if (!ggCpuSmoothedMode() || !(gEmuFlags & GG_MODE)) {
		return;
	}
	// Do not overwrite a completed frame which VBlank has not displayed yet.
	if (smoothedReadyBitmapBase >= 0) {
		return;
	}

	int destinationBitmapBase = smoothedDisplayedBitmapBase
		== GG_SMOOTH_BITMAP_C_BASE
		? GG_SMOOTH_BITMAP_D_BASE : GG_SMOOTH_BITMAP_C_BASE;
	vu16 *destination = (vu16 *)((u8 *)BG_GFX + destinationBitmapBase * 0x4000);
	u32 mapOffset = getVDP0BgrMapOffset() & 0x1F00;
	const vu16 *vramForegroundMap =
		(const vu16 *)((const u8 *)BG_GFX + mapOffset * 8);
	const vu8 *vramBackgroundTiles = (const vu8 *)getVDP0BgrTileAddress();
	const u8 *scrollTMap = getVDP0ScrollTMapBuffer();
	int yScroll = getVDP0YScroll();
	int scrollMask = getVDP0ScrollMask();
	const u16 *oam = getVDP0OAMBuffer();
	const vu8 *vramSpriteTiles = (const vu8 *)getVDP0SpriteTileAddress();

	if (!smoothedCacheInitialised) {
		DC_FlushRange(smoothedBackgroundTiles, sizeof(smoothedBackgroundTiles));
		DC_FlushRange(smoothedSpriteTiles, sizeof(smoothedSpriteTiles));
		DC_FlushRange(smoothedBackgroundMaps, sizeof(smoothedBackgroundMaps));
		smoothedCacheInitialised = true;
	}
	dmaCopyWords(1, (const void *)vramBackgroundTiles, smoothedBackgroundTiles,
		sizeof(smoothedBackgroundTiles));
	dmaCopyWords(1, (const void *)vramSpriteTiles, smoothedSpriteTiles,
		sizeof(smoothedSpriteTiles));
	dmaCopyWords(1, (const void *)vramForegroundMap, smoothedBackgroundMaps,
		sizeof(smoothedBackgroundMaps));
	DC_InvalidateRange(smoothedBackgroundTiles, sizeof(smoothedBackgroundTiles));
	DC_InvalidateRange(smoothedSpriteTiles, sizeof(smoothedSpriteTiles));
	DC_InvalidateRange(smoothedBackgroundMaps, sizeof(smoothedBackgroundMaps));

	const u16 *foregroundMap = smoothedBackgroundMaps;
	const u16 *backgroundMap = foregroundMap + 0x400;

	scaleSmoothedFrame(destination, foregroundMap, backgroundMap,
		smoothedBackgroundTiles,
		scrollTMap, yScroll, scrollMask, oam, smoothedSpriteTiles);
	smoothedReadyBitmapBase = destinationBitmapBase;
}

static void initialiseSmoothed2Video(void) {
	if (smoothed2VideoInitialised) {
		return;
	}

	glInit();
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0, 0, 0, 31);
	glClearPolyID(63);
	glClearDepth(GL_MAX_DEPTH);
	glViewport(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// v16 vertices only have four integer bits, so use a normalized 0..1
	// coordinate system. Passing pixel coordinates directly silently overflows
	// when converted to v16 and produces a partially drawn texture.
	glOrthof32(0, inttof32(1), inttof32(1), 0,
		-inttof32(1), inttof32(1));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	smoothed2VideoInitialised = true;
}

static void drawSmoothed2Quad(int xOffset, int yOffset, int alpha, int id,
									  bool equalDepth) {
	const v16 pixelX = inttov16(1) / SCREEN_WIDTH;
	const v16 pixelY = inttov16(1) / SCREEN_HEIGHT;
	v16 left = xOffset * pixelX;
	v16 top = yOffset * pixelY;
	v16 right = inttov16(1) + left;
	v16 bottom = inttov16(1) + top;
	u32 format = POLY_ALPHA(alpha) | POLY_CULL_NONE | POLY_ID(id);
	if (equalDepth) {
		format |= POLY_DEPTH_TEST_EQUAL;
	}
	glPolyFmt(format);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex3v16(left, top, 0);
	glTexCoord2i(0, SCREEN_HEIGHT);
	glVertex3v16(left, bottom, 0);
	glTexCoord2i(SCREEN_WIDTH, SCREEN_HEIGHT);
	glVertex3v16(right, bottom, 0);
	glTexCoord2i(SCREEN_WIDTH, 0);
	glVertex3v16(right, top, 0);
	glEnd();
}

static void drawSmoothed2Texture(void) {
	initialiseSmoothed2Video();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor(RGB15(31, 31, 31));

	// The DS texture sampler is nearest-neighbour. A half-alpha copy shifted by
	// one output pixel averages adjacent horizontal samples. Two full-screen
	// texture taps exactly fit the 3D engine's 512-pixel-per-scanline fill limit.
	glBindTexture(0, -1);
	GFX_TEX_FORMAT = (GL_RGBA << 26)
		| (TEXTURE_SIZE_256 << 20)
		| (TEXTURE_SIZE_256 << 23)
		| TEXGEN_OFF;
	drawSmoothed2Quad(0, 0, 31, 1, false);
	drawSmoothed2Quad(-1, 0, 16, 2, true);
	glFlush(GL_TRANS_MANUALSORT);
}

void ggSmoothedRender(void) {
	if (ggCpuSmoothedMode()) {
		renderCpuSmoothedFrame();
	}
}

void ggHardwareSmoothedRender(void) {
	if (!ggHardwareSmoothedMode() || !smoothed2DrawPending) {
		return;
	}

	// Submit the filter while the raw frame is being captured. Texture pixels are
	// only fetched after the geometry buffers swap at the following VBlank, when
	// bank C has been remapped as a texture. This gives the geometry engine a full
	// frame to finish instead of racing display capture at scanline 0.
	smoothed2DrawPending = false;
	drawSmoothed2Texture();
}

static bool presentSmoothedFrame(void) {
	int bitmapBase = smoothedReadyBitmapBase;
	if (bitmapBase >= 0) {
		smoothedDisplayedBitmapBase = bitmapBase;
		smoothedReadyBitmapBase = -1;
	}
	else {
		bitmapBase = smoothedDisplayedBitmapBase;
	}
	if (bitmapBase < 0) {
		return false;
	}

	DMA0_CONTROL_REG = 0;
	videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
	REG_BG2CNT = BG_PRIORITY(0) | BG_BMP16_256x256 | BG_BMP_BASE(bitmapBase);
	REG_BG2PA = 0x100;
	REG_BG2PB = 0;
	REG_BG2PC = 0;
	REG_BG2PD = 0x100;
	REG_BG2X = 0;
	REG_BG2Y = 0;
	return true;
}

static void scaleSprites(int horizontalNumerator, int horizontalDenominator,
						 int outputLeft) {
	vu16 *oam = (vu16 *)0x07000000;

	for (int i = 0; i < 128; i++) {
		u16 attr0 = oam[i * 4];
		u16 attr1 = oam[i * 4 + 1];

		if ((attr0 & 0xFF) == SCREEN_HEIGHT || !(attr0 & ATTR0_ROTSCALE)) {
			continue;
		}

		int x = attr1 & 0x1FF;
		if (x >= 256) {
			x -= 512;
		}
		int y = attr0 & 0xFF;
		if (y >= SCREEN_HEIGHT) {
			y -= 256;
		}
		int width;
		int height;
		spriteSize(attr0, attr1, &width, &height);
		int scaledX = outputLeft
			+ ((x - GG_ACTIVE_X) * horizontalNumerator) / horizontalDenominator
			- width / horizontalDenominator;
		int scaledY = ((y - GG_ACTIVE_Y) * GG_VERTICAL_SCALE_NUMERATOR)
			/ GG_VERTICAL_SCALE_DENOMINATOR
			- height / 3;

		// Affine sprites need the double-size bounding box when enlarged.
		oam[i * 4] = (attr0 & ~0xFF) | (scaledY & 0xFF) | ATTR0_ROTSCALE_DOUBLE;
		oam[i * 4 + 1] = (attr1 & ~0x1FF) | (scaledX & 0x1FF);
	}
}

static void restoreNormalVideoMode(void) {
	vramSetBankC(VRAM_C_MAIN_BG_0x06040000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06060000);
	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);
	videoSetMode(MODE_0_2D
			 | DISPLAY_SPR_1D_LAYOUT
			 | DISPLAY_BG0_ACTIVE
			 | DISPLAY_BG1_ACTIVE
			 | DISPLAY_BG2_ACTIVE
			 | DISPLAY_BG3_ACTIVE
			 | DISPLAY_SPR_ACTIVE
			 | DISPLAY_WIN0_ON
				 | DISPLAY_WIN1_ON);
}

static void resetSmoothed2State(void) {
	REG_DISPCAPCNT = 0;
	if (smoothed2BanksActive) {
		vramSetBankC(VRAM_C_MAIN_BG_0x06040000);
		vramSetBankD(VRAM_D_MAIN_BG_0x06060000);
		smoothed2BanksActive = false;
	}
	smoothed2FilterPass = false;
	smoothed2FilteredReady = false;
	smoothed2DrawPending = false;
	smoothed2VideoInitialised = false;
}

static bool runSmoothed2FilterPass(void) {
	if (!smoothed2FilterPass) {
		initialiseSmoothed2Video();
		vramSetBankC(VRAM_C_LCD);
		vramSetBankD(VRAM_D_LCD);
		smoothed2BanksActive = true;
		smoothed2DrawPending = true;
		return false;
	}
	vramSetBankC(VRAM_C_TEXTURE_SLOT0);
	vramSetBankD(VRAM_D_LCD);
	smoothed2BanksActive = true;
	DMA0_CONTROL_REG = 0;
	// BG0's horizontal offset also shifts the DS 3D output. The normal renderer's
	// HBlank DMA leaves the GG map scroll in this register, which otherwise
	// scrolls the captured texture a second time and exposes a black strip.
	REG_BG0HOFS = 0;
	REG_BG0VOFS = 0;
	REG_DISPCAPCNT = DCAP_BANK(DCAP_BANK_VRAM_D)
		| DCAP_SIZE(DCAP_SIZE_256x192)
		| DCAP_MODE(DCAP_MODE_A)
		| DCAP_SRC_A(DCAP_SRC_A_3DONLY)
		| DCAP_ENABLE;
	// Direct VRAM display is explicitly designed to scan a capture destination.
	// It keeps the last complete filtered frame visible until scanout reaches the
	// lines replaced by this capture, and avoids exposing the live 3D renderer.
	videoSetMode(MODE_VRAM_D);
	smoothed2FilterPass = false;
	smoothed2FilteredReady = true;
	return true;
}

void ggFullscreenVBlank(void) {
	if (!ggFullscreenActive()) {
		smoothedReadyBitmapBase = -1;
		smoothedDisplayedBitmapBase = -1;
		resetSmoothed2State();
		if (ggFullscreenWasActive) {
			restoreNormalVideoMode();
			tileCacheValid = false;
			paletteCacheValid = false;
			ggFullscreenWasActive = false;
		}
		return;
	}
	if (!getVDP0ScreenEnabled() || getVDP0DisplayMode() != 4) {
		// Games disable the VDP display while replacing tiles and maps during
		// transitions, and may briefly leave Mode 4. The normal renderer hides or
		// changes layers in these states; the fullscreen renderer only supports
		// Mode 4, so hide it to avoid exposing partially updated VRAM.
		smoothedReadyBitmapBase = -1;
		smoothedDisplayedBitmapBase = -1;
		resetSmoothed2State();
		DMA0_CONTROL_REG = 0;
		videoSetMode(MODE_0_2D);
		ggFullscreenWasActive = true;
		return;
	}
	if (ggHardwareSmoothedMode()) {
		smoothedReadyBitmapBase = -1;
		smoothedDisplayedBitmapBase = -1;
		if (runSmoothed2FilterPass()) {
			ggFullscreenWasActive = true;
			return;
		}
	}
	else {
		resetSmoothed2State();
	}
	if (ggCpuSmoothedMode()) {
		if (presentSmoothedFrame()) {
			ggFullscreenWasActive = true;
			return;
		}
	}
	else {
		smoothedReadyBitmapBase = -1;
		smoothedDisplayedBitmapBase = -1;
	}

	// The normal VBlank renderer has just restored its text-background setup.
	// Stop its HBlank DMA and reassert the affine display registers first, while
	// the beam is still blanked; data conversion below can then use the rest of
	// VBlank without causing a partial-frame mode switch.
	DMA0_CONTROL_REG = 0;
	REG_BG2CNT = BG_PRIORITY(1) | BG_TILE_BASE(GG_EXT_FOREGROUND_TILE_BASE)
			   | BG_MAP_BASE(GG_EXT_FOREGROUND_MAP_BASE) | BG_RS_32x32 | BG_WRAP_ON;
	REG_BG3CNT = BG_PRIORITY(2) | BG_TILE_BASE(GG_EXT_BACKGROUND_TILE_BASE)
			   | BG_MAP_BASE(GG_EXT_BACKGROUND_MAP_BASE) | BG_RS_32x32 | BG_WRAP_ON;

	u32 displayMode = MODE_5_2D
			 | DISPLAY_SPR_1D_LAYOUT
			 | DISPLAY_BG_EXT_PALETTE
			 | DISPLAY_CHAR_BASE(2)
			 | DISPLAY_SCREEN_BASE(2)
			 | DISPLAY_BG2_ACTIVE
			 | DISPLAY_BG3_ACTIVE
			 | DISPLAY_SPR_ACTIVE;
	videoSetMode(displayMode);

	if (!ggFullscreenWasActive) {
		tileCacheValid = false;
		paletteCacheValid = false;
	}

	expandBackgroundTiles();
	convertBackgroundMaps();
	updateExtendedPalettes();
	buildAffineDmaBuffer(GG_FULL_SCREEN_HORIZONTAL_STEP, 0);

	for (int i = 0; i < 12; i++) {
		MAIN_BG_REG_BASE[i] = affineDmaBuffer[0][i];
	}
	DMA0_SOURCE_REG = (u32)&affineDmaBuffer[1][0];
	DMA0_DEST_REG = 0x04000010;
	DMA0_CONTROL_REG = 0x9660000C;

	scaleSprites(8, 5, 0);
	if (ggHardwareSmoothedMode()) {
		REG_DISPCAPCNT = DCAP_BANK(DCAP_BANK_VRAM_C)
			| DCAP_SIZE(DCAP_SIZE_256x192)
			| DCAP_MODE(DCAP_MODE_A)
			| DCAP_SRC_A(DCAP_SRC_A_COMPOSITED)
			| DCAP_ENABLE;
		if (smoothed2FilteredReady) {
			// Keep the affine layers rendering offscreen for capture while bank D
			// is scanned out. MODE_VRAM_D must replace (not combine with) the normal
			// display-source bits; combining them selects FIFO display and causes
			// the repeated-scanline transition glitch.
			videoSetMode((displayMode & ~DISPLAY_MODE_MASK) | MODE_VRAM_D);
		}
		smoothed2FilterPass = true;
	}
	ggFullscreenWasActive = true;
}
