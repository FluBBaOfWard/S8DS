#include <nds.h>

#include "Cart.h"
#include "Equates.h"
#include "Gfx.h"

#define GG_ACTIVE_X             48
#define GG_ACTIVE_Y             24
#define GG_VERTICAL_SCALE_NUMERATOR       4
#define GG_VERTICAL_SCALE_DENOMINATOR     3
#define GG_FULL_HEIGHT_HORIZONTAL_STEP 0xC0
#define GG_FULL_SCREEN_HORIZONTAL_STEP 0xA0
#define GG_SCALED_WIDTH        214
#define GG_PILLAR_LEFT          21
#define GG_PILLAR_RIGHT        (GG_PILLAR_LEFT + GG_SCALED_WIDTH)
#define GG_EXT_FOREGROUND_TILE_BASE     0
#define GG_EXT_BACKGROUND_TILE_BASE     4
#define GG_EXT_FOREGROUND_MAP_BASE     16
#define GG_EXT_BACKGROUND_MAP_BASE     17
#define GG_EXT_FOREGROUND_TILE_ADDRESS ((vu16 *)((u8 *)BG_GFX + 0x20000))
#define GG_EXT_FOREGROUND_MAP_ADDRESS  ((vu16 *)((u8 *)BG_GFX + 0x28000))
#define GG_EXT_BACKGROUND_MAP_ADDRESS  ((vu16 *)((u8 *)BG_GFX + 0x28800))
#define GG_EXT_BACKGROUND_TILE_ADDRESS ((vu16 *)((u8 *)BG_GFX + 0x30000))
#define GG_WINDOW_LAYERS        ((1 << 2) | (1 << 3) | (1 << 4))

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
static bool ggFullscreenWasActive;
static bool expansionTablesReady;
static bool tileCacheValid;
static bool paletteCacheValid;
static bool affineBufferReady;

static bool ggFullscreenActive(void) {
	u32 tileAddress = (u32)getVDP0BgrTileAddress();
	return (gScalingSet == SCALED_GG_FULLSCREEN
			|| gScalingSet == SCALED_GG_FULL_SCREEN)
		&& (gEmuFlags & GG_MODE)
		&& tileAddress >= (u32)BG_GFX
		&& tileAddress < (u32)BG_GFX + 0x20000;
}

static bool ggFullScreenMode(void) {
	return gScalingSet == SCALED_GG_FULL_SCREEN;
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
		const u32 *src = &DMA0Buff[scaledLine * 4];
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

void ggFullscreenVBlank(void) {
	if (!ggFullscreenActive()) {
		if (ggFullscreenWasActive) {
			restoreNormalVideoMode();
			tileCacheValid = false;
			paletteCacheValid = false;
			ggFullscreenWasActive = false;
		}
		return;
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

	bool fullScreen = ggFullScreenMode();
	int horizontalStep = fullScreen ? GG_FULL_SCREEN_HORIZONTAL_STEP
		: GG_FULL_HEIGHT_HORIZONTAL_STEP;
	int horizontalNumerator = fullScreen ? 8 : 4;
	int horizontalDenominator = fullScreen ? 5 : 3;
	int outputLeft = fullScreen ? 0 : GG_PILLAR_LEFT;
	u32 displayMode = MODE_5_2D
			 | DISPLAY_SPR_1D_LAYOUT
			 | DISPLAY_BG_EXT_PALETTE
			 | DISPLAY_CHAR_BASE(2)
			 | DISPLAY_SCREEN_BASE(2)
			 | DISPLAY_BG2_ACTIVE
			 | DISPLAY_BG3_ACTIVE
			 | DISPLAY_SPR_ACTIVE;

	if (!fullScreen) {
		// The 160x144 viewport becomes 213.33x192. An integer window of 214
		// pixels gives symmetric 21-pixel pillarboxes without aspect stretching.
		REG_WIN0H = (GG_PILLAR_LEFT << 8) | GG_PILLAR_RIGHT;
		REG_WIN0V = SCREEN_HEIGHT;
		REG_WININ = GG_WINDOW_LAYERS;
		REG_WINOUT = 0;
		displayMode |= DISPLAY_WIN0_ON;
	}
	videoSetMode(displayMode);

	if (!ggFullscreenWasActive) {
		tileCacheValid = false;
		paletteCacheValid = false;
	}

	expandBackgroundTiles();
	convertBackgroundMaps();
	updateExtendedPalettes();
	buildAffineDmaBuffer(horizontalStep, outputLeft);

	for (int i = 0; i < 12; i++) {
		MAIN_BG_REG_BASE[i] = affineDmaBuffer[0][i];
	}
	DMA0_SOURCE_REG = (u32)&affineDmaBuffer[1][0];
	DMA0_DEST_REG = 0x04000010;
	DMA0_CONTROL_REG = 0x9660000C;

	scaleSprites(horizontalNumerator, horizontalDenominator, outputLeft);
	ggFullscreenWasActive = true;
}
