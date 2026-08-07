#ifndef GFX_HEADER
#define GFX_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "SegaVDP/SegaVDP.h"

extern u8 gFlicker;
extern u8 gTwitch;
extern u8 gGfxMask;
extern u8 gColorValue;
extern u8 g3DEnable;
extern u8 bColor;
extern u8 SPRS;

extern SegaVDP VDP0;
extern u16 EMUPALBUFF[];
extern u32 DMA0Buff[];

void gfxInit(void);
void vblIrqHandler(void);
void antWarsInit(void);
void antWars(void);
void paletteInit(u8 gammaVal);
void mapSGPalette(u8 gammaVal);
void paletteTxAll(void);
void refreshGfx(void);
void makeBorder(void);
void setupScaling(void);
void updateLCDRefresh(void);
void hz50Refresh(void);
void ggFullscreenVBlank(void);
void ggHardwareSmoothedRender(void);
void ggSmoothedRender(void);
void ggFrameTimingStart(void);
u8 *getVDP0BgrTileAddress(void);
u32 getVDP0BgrMapOffset(void);
bool getVDP0ScreenEnabled(void);
u8 getVDP0DisplayMode(void);
u8 *getVDP0ScrollTMapBuffer(void);
u8 getVDP0YScroll(void);
u32 getVDP0ScrollMask(void);
u16 *getVDP0OAMBuffer(void);
u8 *getVDP0SpriteTileAddress(void);

void VDP0ApplyScaling(void);
void VDP0SetMode(void);
void VDP0ScanlineBPReset(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !GFX_HEADER
