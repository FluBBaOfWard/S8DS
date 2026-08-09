/*
 * Game Gear fullscreen upscaler
 * =============================
 *
 * S8DS normally renders the Sega VDP's 256x192 field directly.  In Game Gear
 * mode the handheld LCD only shows the centred 160x144 window at (48, 24):
 *
 *                 emulated VDP field: 256x192
 *
 *              48       160 active pixels       48
 *           +------+-------------------------+------+
 *        24 |      |                         |      |
 *           +------+-------------------------+------+
 *       144 |      |    Game Gear picture    |      |
 *           +------+-------------------------+------+
 *        24 |      |                         |      |
 *           +------+-------------------------+------+
 *
 * Merely showing that window 1:1 leaves a small image.  Stretching 160x144 to
 * the DS's 256x192 screen is also the intended aspect correction: a Game Gear
 * LCD pixel is effectively 6:5 wider than it is tall, whereas DS pixels are
 * approximately square.  Therefore:
 *
 *             horizontal scale = 256 / 160 = 8 / 5
 *             vertical scale   = 192 / 144 = 4 / 3
 *
 * Or, expressed as the smallest repeating grid:
 *
 *                  5x3 Game Gear pixels -> 8x4 DS pixels
 *
 * The resulting 256:192 picture is 4:3.  This is why the GG upscaler is a
 * separate option from the ordinary SMS display geometry: Off leaves the
 * selected SMS display mode alone, while any of the three modes below replaces
 * it for Game Gear software.
 *
 * Fast - 60 fps: DS 2D affine nearest neighbour
 * ------------------------------------------------
 *
 *                  160x144 GG window
 *                          |
 *          expand 4-bpp tiles and convert tile maps
 *                          |
 *              DS affine BGs + affine sprites
 *                          |
 *                 256x192 DS display
 *
 * No complete RGB framebuffer is constructed.  The DS 2D engine scales the
 * tile layers directly, using an inverse horizontal affine step of 5/8 and a
 * per-scanline affine table for the 3/4 vertical step.  The table is rebuilt
 * from the normal renderer's HBlank scroll data so raster and line-scroll
 * effects remain visible.  Sprites are repositioned and enlarged separately.
 *
 * Mode 4 tiles are 4-bpp but extended affine backgrounds require 8-bpp tiles,
 * hence the cached nibble expansion in this file.  Two background layers are
 * used to preserve VDP priority around sprites: the foreground copy leaves
 * colour 0 transparent, while the behind-sprites copy maps it to palette index
 * 16 so that the GG backdrop colour remains opaque.
 *
 * Nearest-neighbour expansion necessarily repeats pixels unevenly.  Ignoring
 * scroll phase, one horizontal and vertical period looks like this:
 *
 *             horizontal: A B C D E -> A A B B C D D E
 *             vertical:   A B C     -> A A B C
 *
 * That uneven cadence is the visible scrolling "chonkiness" of Fast.  It is
 * also why Fast is cheap enough to present every emulated frame.
 *
 * Smooth - Adaptive: ARM9 separable bilinear filter
 * -------------------------------------------------
 *
 * Smooth first freezes one coherent source picture: tiles, both tile maps,
 * palette, line-scroll state and decoded sprite state.  For each source line it
 * then composes the same priority order as the VDP renderer:
 *
 *       background-behind -> sprites -> priority foreground
 *
 * The complete RGB15 line is filtered horizontally from 5 to 8 pixels.  The
 * unrolled filter samples source positions 0, 5/8, 10/8, ... rather than
 * choosing only the nearest source pixel:
 *
 *       source:       A---------B---------C---------D---------E
 *       destinations: 0    1    2    3    4    5    6    7
 *       source pos:   0   5/8 10/8 15/8 20/8 25/8 30/8 35/8
 *
 * Adjacent filtered lines are then blended vertically at source positions
 * 0, 3/4, 1 1/2, 2 1/4, 3, ... . Thus each output pixel is bilinearly filtered
 * with weighted contributions from a 2x2 source neighbourhood.
 *
 * Two line buffers are sufficient because the vertical pass only needs the
 * current and next horizontally filtered source lines.
 *
 * This high-quality software path costs too much to finish during every 60 Hz
 * host interval on existing DS hardware.  It therefore renders into the hidden
 * half of a C/D VRAM double buffer and publishes only complete pictures:
 *
 *       visible:  [ previous complete picture ......................... ]
 *       hidden:   [ part A ] -> [ part B ] -> [ part C ] -> complete
 *                                                              |
 *       VBlank:                                                swap
 *
 * Intermediate buffers are never displayed.  More importantly, every part is
 * rendered from the same frozen source snapshot; continuing on a later host
 * interval cannot mix state from two emulated frames.
 *
 * With GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING enabled, work is divided into
 * four-output-line slices.  Four lines are the natural unit because the 3:4
 * vertical filter phase repeats there.  At the beginning of each 60 Hz host
 * interval, timers 2 and 3 start a 33.5 MHz counter.  After each slice the
 * scheduler:
 *
 *   1. calculates elapsed time / slices completed in this interval;
 *   2. blends the noisy first few samples with the historical slice average;
 *   3. adds a 1/8 safety margin; and
 *   4. yields if one more slice is predicted to cross the 15 ms deadline.
 *
 * The deadline includes GUI and emulation work performed before this renderer,
 * so the split adapts to both the device and the current game's workload.  The
 * next interval resumes at smoothedRenderNextLine.
 *
 * At a 60 Hz host cadence, a picture completed in N intervals is presented at
 * approximately 60/N fps:
 *
 *        intervals used       1       2       3       4
 *        video rate          60      30      20      15 fps
 *
 * For example, a three-way split is:
 *
 *        host interval:     |   N   |  N+1  |  N+2  |  N+3  |
 *        Smooth work:       | part A| part B| part C| next A|
 *        presentation:      | old picture  | new picture    |
 *
 * Emulation, input and audio still run at the host cadence; only intermediate
 * video pictures are omitted.  Without the dynamic define, the legacy fallback
 * is a fixed two-part split at GG_SMOOTH_FIRST_SLICE_LINES.
 *
 * On a physical DSi at 134 MHz, 30 fps has been measured; DS-mode emulation
 * measured 15 fps.
 *
 * Smooth2 - 30 fps: captured 2D image plus DS 3D two-tap filter
 * ----------------------------------------------------------------
 *
 * Smooth2 trades filtering quality and about one host frame of pipeline latency
 * for very low ARM9 cost.  It alternates two stages:
 *
 *       stage 1: Fast affine result -> composited capture in VRAM C
 *       stage 2: VRAM C texture     -> 3D filter -> capture/display VRAM D
 *
 * The DS texture sampler itself is nearest-neighbour, not bilinear.  The 3D
 * filter draws the captured image once at full strength and again shifted left
 * by one output pixel at half alpha.  Equal-depth blending averages adjacent
 * horizontal output samples:
 *
 *                 output[x] ~= (Fast[x] + Fast[x + 1]) / 2
 *
 * These two fullscreen texture taps exactly consume the 3D engine's 512 pixels
 * per scanline fill allowance.  Vertical scaling remains Fast's nearest-
 * neighbour 3-to-4 mapping, so Smooth2 is an inexpensive horizontal smoothing
 * approximation, not source-space bilinear filtering.
 *
 * Geometry submission and display capture are pipelined across VBlanks because
 * the geometry engine consumes the submitted list after its buffer swap.  The
 * raw capture cannot safely become a texture and be rendered/captured in the
 * same VBlank.  This alternating capture/filter pipeline is the reason Smooth2
 * is fixed at 30 fps and has more input-to-display latency than Smooth.
 *
 * When changing these paths, retain two important invariants:
 *
 *   - Never expose a partially updated capture or bitmap during VDP-disabled
 *     transitions; games commonly replace tiles and maps in that state.
 *   - Reset the 3D layer scroll and replace (do not OR together) the display-
 *     source bits when selecting direct VRAM display.  Violating either rule
 *     produces black bars, doubled scrolling or repeated scanlines.
 */

#include <nds.h>

#ifdef GG_SMOOTH_PROFILE
#include <stdio.h>
#endif

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
#define GG_SMOOTH_FIRST_SLICE_LINES       84
#define GG_SMOOTH_DYNAMIC_DEADLINE_MS     15
#define GG_SMOOTH_DYNAMIC_DEADLINE_TICKS \
	((BUS_CLOCK / 1000) * GG_SMOOTH_DYNAMIC_DEADLINE_MS)
#define GG_SMOOTH_DYNAMIC_EARLY_SLICES    4
#define GG_SMOOTH_DYNAMIC_MARGIN_SHIFT    3

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
static u32 smoothedPackedBackgroundTileCache[GG_BACKGROUND_TILE_BYTES / sizeof(u32)]
	__attribute__((aligned(4)));
static u8 smoothedExpandedBackgroundTiles[GG_BACKGROUND_TILE_BYTES * 2]
	__attribute__((aligned(32)));
static u8 smoothedSpriteTiles[GG_SPRITE_TILE_BYTES]
	__attribute__((aligned(32)));
static u16 smoothedBackgroundMaps[GG_BACKGROUND_MAP_BYTES / sizeof(u16)]
	__attribute__((aligned(32)));
static u8 smoothedScrollTMap[SCREEN_HEIGHT * 2];
static u16 smoothedPalette[0x200];

typedef struct {
	s16 x;
	s16 y;
	u16 baseTile;
	u8 width;
	u8 height;
	u8 zoomShift;
	u8 palette;
	u8 tilesPerRow;
} SmoothedSprite;

static SmoothedSprite smoothedSprites[128];
static int smoothedSpriteCount;
static int smoothedRenderNextLine;
static int smoothedRenderDestinationBase = -1;
static int smoothedRenderYScroll;
static int smoothedRenderScrollMask;
#ifdef GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING
static u32 smoothedSliceEstimateTicks;
#endif
static bool ggFullscreenWasActive;
static bool expansionTablesReady;
static bool tileCacheValid;
static bool paletteCacheValid;
static bool affineBufferReady;
static volatile int smoothedReadyBitmapBase = -1;
static volatile int smoothedDisplayedBitmapBase = -1;
static u8 smoothedPreviewMode = GG_UPSCALER_OFF;
static bool smoothedPreviewPending;
static bool smoothed2VideoInitialised;
static bool smoothedCacheInitialised;
static bool smoothedExpandedTilesValid;
static bool smoothed2BanksActive;
static bool smoothed2FilterPass;
static bool smoothed2FilteredReady;
static volatile bool smoothed2DrawPending;

#ifdef GG_SMOOTH_PROFILE
#define GG_SMOOTH_PROFILE_WARMUP_FRAMES 60
#define GG_SMOOTH_PROFILE_CAPTURE_FRAMES 180

typedef struct {
	u32 total;
	u32 copy;
	u32 backgroundBehind;
	u32 sprites;
	u32 backgroundForeground;
	u32 horizontal;
	u32 vertical;
	u16 firstLine;
	u16 endLine;
	bool dropped;
	u32 sliceAverage;
	u32 sliceEstimate;
} GGSmoothProfileFrame;

static GGSmoothProfileFrame smoothProfileCurrent;
static GGSmoothProfileFrame smoothProfileFrames[GG_SMOOTH_PROFILE_CAPTURE_FRAMES];
static int smoothProfileWarmup;
static int smoothProfileCount;
static bool smoothProfileWritten;
static u32 smoothProfileStarted;

static void writeSmoothProfile(void) {
	if (smoothProfileWritten) {
		return;
	}
	smoothProfileWritten = true;
	FILE *file = fopen("fat:/s8ds-smooth1-profile.csv", "w");
	if (file == NULL) {
		file = fopen("s8ds-smooth1-profile.csv", "w");
	}
	if (file == NULL) {
		return;
	}

	fputs("frame,total,copy,bg_behind,sprites,bg_foreground,horizontal,vertical,other,first_line,end_line,dropped,slice_average,slice_estimate\n",
		file);
	u64 sums[7] = { 0 };
	int droppedFrames = 0;
	for (int frame = 0; frame < smoothProfileCount; frame++) {
		const GGSmoothProfileFrame *sample = &smoothProfileFrames[frame];
		u32 measured = sample->copy + sample->backgroundBehind + sample->sprites
			+ sample->backgroundForeground + sample->horizontal + sample->vertical;
		u32 other = sample->total > measured ? sample->total - measured : 0;
		fprintf(file, "%d,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%lu,%lu\n", frame,
			(unsigned long)sample->total, (unsigned long)sample->copy,
			(unsigned long)sample->backgroundBehind,
			(unsigned long)sample->sprites,
			(unsigned long)sample->backgroundForeground,
			(unsigned long)sample->horizontal, (unsigned long)sample->vertical,
			(unsigned long)other, sample->firstLine, sample->endLine,
			sample->dropped, (unsigned long)sample->sliceAverage,
			(unsigned long)sample->sliceEstimate);
		sums[0] += sample->total;
		sums[1] += sample->copy;
		sums[2] += sample->backgroundBehind;
		sums[3] += sample->sprites;
		sums[4] += sample->backgroundForeground;
		sums[5] += sample->horizontal;
		sums[6] += sample->vertical;
		droppedFrames += sample->dropped;
	}
	if (smoothProfileCount > 0) {
		fputs("# average_ticks", file);
		for (int stage = 0; stage < 7; stage++) {
			fprintf(file, ",%lu", (unsigned long)(sums[stage] / smoothProfileCount));
		}
		fputc('\n', file);
		fputs("# average_us", file);
		for (int stage = 0; stage < 7; stage++) {
			fprintf(file, ",%lu", (unsigned long)timerTicks2usec(
				(u32)(sums[stage] / smoothProfileCount)));
		}
		fputc('\n', file);
		fprintf(file, "# dropped_frames,%d\n", droppedFrames);
	}
	fclose(file);
}

static inline void beginSmoothProfile(void) {
	smoothProfileCurrent = (GGSmoothProfileFrame){ 0 };
	smoothProfileStarted = cpuGetTiming();
}

static inline void finishSmoothProfile(void) {
	smoothProfileCurrent.total = cpuGetTiming() - smoothProfileStarted;
	if (smoothProfileWarmup < GG_SMOOTH_PROFILE_WARMUP_FRAMES) {
		smoothProfileWarmup++;
		return;
	}
	if (smoothProfileCount < GG_SMOOTH_PROFILE_CAPTURE_FRAMES) {
		smoothProfileFrames[smoothProfileCount++] = smoothProfileCurrent;
	}
	if (smoothProfileCount == GG_SMOOTH_PROFILE_CAPTURE_FRAMES) {
		writeSmoothProfile();
	}
}
#endif

#if defined(GG_SMOOTH_PROFILE) \
	|| defined(GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING)
void ggFrameTimingStart(void) {
	// Timers 0-1 are reserved by the audio stream. Timers 2-3 form a cascaded
	// 32-bit counter at the 33.5 MHz bus clock and are restarted once per host
	// frame so the dynamic renderer accounts for emulation and GUI work too.
	cpuStartTiming(2);
}
#endif

static bool ggFullscreenActive(void) {
	u32 tileAddress = (u32)getVDP0BgrTileAddress();
	return gGGScalingMethod != GG_UPSCALER_OFF
		&& (gEmuFlags & GG_MODE)
		&& tileAddress >= (u32)BG_GFX
		&& tileAddress < (u32)BG_GFX + 0x20000;
}

static bool ggCpuSmoothedMode(void) {
	return (gEmuFlags & GG_MODE)
		&& gGGScalingMethod == GG_UPSCALER_SMOOTH;
}

static bool ggHardwareSmoothedMode(void) {
	return (gEmuFlags & GG_MODE)
		&& gGGScalingMethod == GG_UPSCALER_SMOOTH2;
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

static void drawSmoothedBackgroundLine(u16 *restrict dst, int y,
									   const u16 *restrict map,
									   const u8 *restrict tiles,
									   const u8 *scrollTMap,
									   int yScroll, int scrollMask,
									   bool foreground) {
	int sourceLine = GG_ACTIVE_Y + y;
	int scrollX = scrollTMap[sourceLine * 2 + 1];
	int mapY = yScroll + sourceLine;
	if (mapY >= scrollMask) {
		mapY -= scrollMask;
	}

	// Work a tile segment at a time. The former pixel loop repeated the map
	// lookup, tile-address calculation and flip handling for all 160 pixels.
	// A line crosses only 20 or 21 tiles, including an unaligned first tile.
	for (int x = 0; x < GAME_WIDTH_GG;) {
		int mapX = (scrollX + GG_ACTIVE_X + x) & 0xFF;
		int tileX = mapX & 7;
		int run = 8 - tileX;
		if (run > GAME_WIDTH_GG - x) {
			run = GAME_WIDTH_GG - x;
		}
		u16 entry = map[(mapY >> 3) * 32 + (mapX >> 3)];
		if (foreground && (entry & 0x3FF) == 0x3FF) {
			x += run;
			continue;
		}

		int tileY = mapY & 7;
		if (entry & BIT(11)) {
			tileY = 7 - tileY;
		}
		int tileNumber = entry & 0x3FF;
		int palette = (entry >> 12) & 0x0F;
		const u16 *colors = smoothedPalette + palette * 16;
		const u8 *row = tiles + tileNumber * 64 + tileY * 8;
		int sourceX;
		int sourceStep;
		if (entry & BIT(10)) {
			sourceX = 7 - tileX;
			sourceStep = -1;
		}
		else {
			sourceX = tileX;
			sourceStep = 1;
		}

		if (foreground) {
			for (int pixel = 0; pixel < run; pixel++, sourceX += sourceStep) {
				u8 color = row[sourceX];
				if (color != 0) {
					dst[x + pixel] = colors[color];
				}
			}
		}
		else {
			u16 colorZero = smoothedPalette[(palette & 1) ? 0x40 : 0x30];
			for (int pixel = 0; pixel < run; pixel++, sourceX += sourceStep) {
				u8 color = row[sourceX];
				dst[x + pixel] = color != 0 ? colors[color] : colorZero;
			}
		}
		x += run;
	}
}

static void prepareSmoothedSprites(const u16 *oam) {
	smoothedSpriteCount = 0;
	// Store the active entries in reverse OAM order so drawing the compact list
	// preserves the DS rule that lower OAM indexes win overlapping pixels.
	for (int i = 127; i >= 0; i--) {
		u16 attr0 = oam[i * 4];
		u16 attr1 = oam[i * 4 + 1];
		u16 attr2 = oam[i * 4 + 2];
		if ((attr0 & 0xFF) == SCREEN_HEIGHT || !(attr0 & ATTR0_ROTSCALE)) {
			continue;
		}

		SmoothedSprite *sprite = &smoothedSprites[smoothedSpriteCount++];
		sprite->x = attr1 & 0x1FF;
		if (sprite->x >= 256) {
			sprite->x -= 512;
		}
		sprite->y = attr0 & 0xFF;
		if (sprite->y >= SCREEN_HEIGHT) {
			sprite->y -= 256;
		}
		int width;
		int height;
		spriteSize(attr0, attr1, &width, &height);
		sprite->width = width;
		sprite->height = height;
		sprite->zoomShift = (((attr1 >> 9) & 0x1F) & 2) ? 1 : 0;
		sprite->baseTile = attr2 & 0x3FF;
		sprite->palette = (attr2 >> 12) & 0x0F;
		sprite->tilesPerRow = width >> 3;
	}
}

static void drawSmoothedSpriteLine(u16 *dst, int y, const u8 *tiles) {
	int screenY = GG_ACTIVE_Y + y;

	for (int i = 0; i < smoothedSpriteCount; i++) {
		const SmoothedSprite *sprite = &smoothedSprites[i];
		int zoom = 1 << sprite->zoomShift;
		int outputY = screenY - sprite->y;
		if ((unsigned)outputY >= (unsigned)(sprite->height * zoom)) {
			continue;
		}
		int sourceY = outputY >> sprite->zoomShift;

		for (int outputX = 0; outputX < sprite->width * zoom; outputX++) {
			int dstX = sprite->x - GG_ACTIVE_X + outputX;
			if ((unsigned)dstX >= GAME_WIDTH_GG) {
				continue;
			}
			int sourceX = outputX >> sprite->zoomShift;
			int tileNumber = sprite->baseTile
				+ (sourceY >> 3) * sprite->tilesPerRow
				+ (sourceX >> 3);
			const u8 *tile = tiles + tileNumber * 32;
			u8 packed = tile[(sourceY & 7) * 4 + ((sourceX & 7) >> 1)];
			u8 color = (packed >> ((sourceX & 1) * 4)) & 0x0F;
			if (color != 0) {
				dst[dstX] = smoothedPalette[0x100 + sprite->palette * 16 + color];
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
								const u16 *foregroundMap,
				const u16 *backgroundMap,
				const u8 *backgroundTiles, const u8 *scrollTMap,
				int yScroll, int scrollMask, const u8 *spriteTiles) {
#ifdef GG_SMOOTH_PROFILE
	u32 started = cpuGetTiming();
#endif
	drawSmoothedBackgroundLine(source, sourceY, backgroundMap, backgroundTiles,
		scrollTMap, yScroll, scrollMask, false);
#ifdef GG_SMOOTH_PROFILE
	u32 finished = cpuGetTiming();
	smoothProfileCurrent.backgroundBehind += finished - started;
	started = finished;
#endif
	drawSmoothedSpriteLine(source, sourceY, spriteTiles);
#ifdef GG_SMOOTH_PROFILE
	finished = cpuGetTiming();
	smoothProfileCurrent.sprites += finished - started;
	started = finished;
#endif
	drawSmoothedBackgroundLine(source, sourceY, foregroundMap, backgroundTiles,
		scrollTMap, yScroll, scrollMask, true);
#ifdef GG_SMOOTH_PROFILE
	finished = cpuGetTiming();
	smoothProfileCurrent.backgroundForeground += finished - started;
#endif
}

static void ITCM_CODE prepareSmoothedLine(int sourceY, int slot,
								const u16 *foregroundMap,
				const u16 *backgroundMap,
				const u8 *backgroundTiles, const u8 *scrollTMap,
				int yScroll, int scrollMask, const u8 *spriteTiles) {
	u16 *source = ggSourceLines[slot];
	u16 *horizontal = ggHorizontalLines[slot];
	composeSmoothedLine(source, sourceY, foregroundMap, backgroundMap,
		backgroundTiles, scrollTMap, yScroll, scrollMask, spriteTiles);

	// These source coordinates match the nearest-neighbour 5:8 mapping. The
	// fractional remainder supplies the contribution from the adjacent pixel.
#ifdef GG_SMOOTH_PROFILE
	u32 horizontalStarted = cpuGetTiming();
#endif
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
#ifdef GG_SMOOTH_PROFILE
	smoothProfileCurrent.horizontal += cpuGetTiming() - horizontalStarted;
#endif
}

#ifdef GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING
static inline void updateSmoothedSliceEstimate(u32 currentAverage) {
	if (smoothedSliceEstimateTicks == 0) {
		smoothedSliceEstimateTicks = currentAverage;
	}
	else {
		// Retain enough history to seed the first few slices of the next interval,
		// but let the current picture move the estimate by one quarter.
		smoothedSliceEstimateTicks = (smoothedSliceEstimateTicks * 3
			+ currentAverage + 2) >> 2;
	}
}
#endif

static int ITCM_CODE __attribute__((noinline)) scaleSmoothedFrame(
				vu16 *dst, const u16 *foregroundMap,
				const u16 *backgroundMap,
				const u8 *backgroundTiles, const u8 *scrollTMap,
				int yScroll, int scrollMask, const u8 *spriteTiles,
				int firstOutputLine, int endOutputLine) {
	int bufferedLine[2] = { -1, -1 };
#ifdef GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING
	u32 sliceTimingStarted = cpuGetTiming();
	u32 priorSliceEstimate = smoothedSliceEstimateTicks;
	u32 currentSliceAverage = priorSliceEstimate;
	int slicesRendered = 0;
#endif

	for (int y = firstOutputLine; y < endOutputLine; y++) {
		int fixedY = y * 3;
		int sourceY = fixedY >> 2;
		int nextY = sourceY < GAME_HEIGHT_GG - 1 ? sourceY + 1 : sourceY;
		int firstSlot = sourceY & 1;
		int secondSlot = nextY & 1;
		if (bufferedLine[firstSlot] != sourceY) {
			prepareSmoothedLine(sourceY, firstSlot, foregroundMap, backgroundMap,
				backgroundTiles, scrollTMap, yScroll, scrollMask, spriteTiles);
			bufferedLine[firstSlot] = sourceY;
		}
		if (bufferedLine[secondSlot] != nextY) {
			prepareSmoothedLine(nextY, secondSlot, foregroundMap, backgroundMap,
				backgroundTiles, scrollTMap, yScroll, scrollMask, spriteTiles);
			bufferedLine[secondSlot] = nextY;
		}
		const u16 *first = ggHorizontalLines[firstSlot];
		const u16 *second = ggHorizontalLines[secondSlot];
		const u32 *firstPairs = (const u32 *)first;
		const u32 *secondPairs = (const u32 *)second;
		int weight = fixedY & 3;
		vu32 *output = (vu32 *)(dst + y * SCREEN_WIDTH);
#ifdef GG_SMOOTH_PROFILE
		u32 verticalStarted = cpuGetTiming();
#endif
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
#ifdef GG_SMOOTH_PROFILE
		smoothProfileCurrent.vertical += cpuGetTiming() - verticalStarted;
#endif
#ifdef GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING
		// The 3:4 vertical filter repeats every four output lines. At each complete
		// group, use all groups rendered during this call for a stable average and
		// decide whether another complete group is likely to fit.
		if (((y + 1) & (GG_VERTICAL_SCALE_NUMERATOR - 1)) == 0) {
			slicesRendered++;
			u32 now = cpuGetTiming();
			currentSliceAverage = (now - sliceTimingStarted) / slicesRendered;
			u32 nextSliceEstimate = currentSliceAverage;
			if (slicesRendered < GG_SMOOTH_DYNAMIC_EARLY_SLICES) {
				if (priorSliceEstimate != 0) {
					// Blend the initially noisy current average with history. Its
					// influence falls to zero after four groups.
					int historyWeight = GG_SMOOTH_DYNAMIC_EARLY_SLICES
						- slicesRendered;
					nextSliceEstimate = (currentSliceAverage * slicesRendered
						+ priorSliceEstimate * historyWeight)
						/ GG_SMOOTH_DYNAMIC_EARLY_SLICES;
				}
				else {
					// The first-ever group has no history and tends to be an
					// optimistic sample, so bias it conservatively.
					nextSliceEstimate += nextSliceEstimate >> 2;
				}
			}
			// Keep a small margin for group-to-group variation and the timer check.
			nextSliceEstimate += nextSliceEstimate
				>> GG_SMOOTH_DYNAMIC_MARGIN_SHIFT;
#ifdef GG_SMOOTH_PROFILE
			smoothProfileCurrent.sliceAverage = currentSliceAverage;
			smoothProfileCurrent.sliceEstimate = nextSliceEstimate;
#endif
			bool pictureComplete = y + 1 == endOutputLine;
			bool nextSliceMissesDeadline = now >= GG_SMOOTH_DYNAMIC_DEADLINE_TICKS
				|| nextSliceEstimate
					>= GG_SMOOTH_DYNAMIC_DEADLINE_TICKS - now;
			if (pictureComplete || nextSliceMissesDeadline) {
				updateSmoothedSliceEstimate(currentSliceAverage);
				if (!pictureComplete) {
					return y + 1;
				}
			}
		}
#endif
	}
	return endOutputLine;
}

static void renderCpuSmoothedFrame(void) {
	if (!ggCpuSmoothedMode() || !(gEmuFlags & GG_MODE)
			|| !getVDP0ScreenEnabled() || getVDP0DisplayMode() != 4) {
		smoothedRenderNextLine = 0;
		smoothedRenderDestinationBase = -1;
		return;
	}
	// Do not overwrite a completed frame which VBlank has not displayed yet.
	if (smoothedReadyBitmapBase >= 0) {
		return;
	}

#ifdef GG_SMOOTH_PROFILE
	beginSmoothProfile();
	smoothProfileCurrent.firstLine = smoothedRenderNextLine;
#endif

	if (smoothedRenderNextLine == 0) {
		smoothedRenderDestinationBase = smoothedDisplayedBitmapBase
			== GG_SMOOTH_BITMAP_C_BASE
			? GG_SMOOTH_BITMAP_D_BASE : GG_SMOOTH_BITMAP_C_BASE;
	}
	int destinationBitmapBase = smoothedRenderDestinationBase;
	vu16 *destination = (vu16 *)((u8 *)BG_GFX + destinationBitmapBase * 0x4000);

	if (smoothedRenderNextLine == 0) {
		u32 mapOffset = getVDP0BgrMapOffset() & 0x1F00;
		const vu16 *vramForegroundMap =
			(const vu16 *)((const u8 *)BG_GFX + mapOffset * 8);
		const vu8 *vramBackgroundTiles = (const vu8 *)getVDP0BgrTileAddress();
		const u16 *oam = getVDP0OAMBuffer();
		const vu8 *vramSpriteTiles = (const vu8 *)getVDP0SpriteTileAddress();

		if (!smoothedCacheInitialised) {
			DC_FlushRange(smoothedBackgroundTiles, sizeof(smoothedBackgroundTiles));
			DC_FlushRange(smoothedSpriteTiles, sizeof(smoothedSpriteTiles));
			DC_FlushRange(smoothedBackgroundMaps, sizeof(smoothedBackgroundMaps));
			smoothedCacheInitialised = true;
		}
#ifdef GG_SMOOTH_PROFILE
		u32 copyStarted = cpuGetTiming();
#endif
		dmaCopyWords(1, (const void *)vramBackgroundTiles, smoothedBackgroundTiles,
			sizeof(smoothedBackgroundTiles));
		dmaCopyWords(1, (const void *)vramSpriteTiles, smoothedSpriteTiles,
			sizeof(smoothedSpriteTiles));
		dmaCopyWords(1, (const void *)vramForegroundMap, smoothedBackgroundMaps,
			sizeof(smoothedBackgroundMaps));
		DC_InvalidateRange(smoothedBackgroundTiles, sizeof(smoothedBackgroundTiles));
		DC_InvalidateRange(smoothedSpriteTiles, sizeof(smoothedSpriteTiles));
		DC_InvalidateRange(smoothedBackgroundMaps, sizeof(smoothedBackgroundMaps));

		const u32 *packedTiles = (const u32 *)smoothedBackgroundTiles;
		for (int word = 0; word < GG_BACKGROUND_TILE_BYTES / 4; word++) {
			u32 packed = packedTiles[word];
			if (smoothedExpandedTilesValid
					&& smoothedPackedBackgroundTileCache[word] == packed) {
				continue;
			}
			smoothedPackedBackgroundTileCache[word] = packed;
			u8 *expanded = smoothedExpandedBackgroundTiles + word * 8;
			for (int pixel = 0; pixel < 8; pixel++) {
				expanded[pixel] = (packed >> (pixel * 4)) & 0x0F;
			}
		}
		smoothedExpandedTilesValid = true;
		const u8 *scrollTMap = getVDP0ScrollTMapBuffer();
		for (int index = 0; index < SCREEN_HEIGHT * 2; index++) {
			smoothedScrollTMap[index] = scrollTMap[index];
		}
		for (int index = 0; index < 0x200; index++) {
			smoothedPalette[index] = EMUPALBUFF[index];
		}
		smoothedRenderYScroll = getVDP0YScroll();
		smoothedRenderScrollMask = getVDP0ScrollMask();
#ifdef GG_SMOOTH_PROFILE
		smoothProfileCurrent.copy = cpuGetTiming() - copyStarted;
#endif
#ifdef GG_SMOOTH_PROFILE
		u32 spriteSetupStarted = cpuGetTiming();
#endif
		prepareSmoothedSprites(oam);
#ifdef GG_SMOOTH_PROFILE
		smoothProfileCurrent.sprites += cpuGetTiming() - spriteSetupStarted;
#endif
	}

	const u16 *foregroundMap = smoothedBackgroundMaps;
	const u16 *backgroundMap = foregroundMap + 0x400;
#ifdef GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING
	// Try the whole picture. A fast host interval can publish immediately at
	// 60 Hz; otherwise scaleSmoothedFrame yields at the deadline and this same
	// frozen source picture resumes during as many intervals as it needs.
	int endOutputLine = SCREEN_HEIGHT;
#else
	bool firstRenderInterval = smoothedRenderNextLine == 0;
	int endOutputLine = firstRenderInterval
		? GG_SMOOTH_FIRST_SLICE_LINES : SCREEN_HEIGHT;
#endif
	smoothedRenderNextLine = scaleSmoothedFrame(destination,
		foregroundMap, backgroundMap,
		smoothedExpandedBackgroundTiles,
		smoothedScrollTMap, smoothedRenderYScroll, smoothedRenderScrollMask,
		smoothedSpriteTiles, smoothedRenderNextLine, endOutputLine);
#ifdef GG_SMOOTH_PROFILE
	smoothProfileCurrent.endLine = smoothedRenderNextLine;
#endif
	if (smoothedRenderNextLine == SCREEN_HEIGHT) {
		smoothedReadyBitmapBase = destinationBitmapBase;
		smoothedRenderNextLine = 0;
		smoothedRenderDestinationBase = -1;
	}
#ifdef GG_SMOOTH_PROFILE
	finishSmoothProfile();
#endif
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

static void resetCpuSmoothedRender(void) {
	smoothedRenderNextLine = 0;
	smoothedRenderDestinationBase = -1;
#ifdef GG_SMOOTH_DYNAMIC_FRAME_RENDER_SPLITTING
	smoothedSliceEstimateTicks = 0;
#endif
}

void ggSmoothedRender(void) {
	smoothedPreviewMode = gGGScalingMethod;
	smoothedPreviewPending = false;
	if (ggCpuSmoothedMode()) {
		renderCpuSmoothedFrame();
	}
	else {
		resetCpuSmoothedRender();
	}
}

void ggSmoothedRenderPausedPreview(void) {
	// Autopause stops emulation frames, so render one frozen picture when the
	// menu changes mode, then stay idle after that picture reaches VBlank.
	if (gGGScalingMethod != smoothedPreviewMode) {
		smoothedPreviewMode = gGGScalingMethod;
		smoothedPreviewPending = ggCpuSmoothedMode();
		resetCpuSmoothedRender();
	}
	if (!smoothedPreviewPending) {
		return;
	}

	renderCpuSmoothedFrame();
	if (smoothedReadyBitmapBase >= 0) {
		smoothedPreviewPending = false;
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
		smoothedRenderNextLine = 0;
		smoothedRenderDestinationBase = -1;
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
		smoothedRenderNextLine = 0;
		smoothedRenderDestinationBase = -1;
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
		smoothedRenderNextLine = 0;
		smoothedRenderDestinationBase = -1;
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
