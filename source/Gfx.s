#ifdef __arm__

#include "Shared/nds_asm.h"
#include "Equates.h"
#include "ARMZ80/ARMZ80.i"
#include "SegaVDP/SegaVDP.i"

	.global antWars
	.global gfxInit
	.global gfxReset
	.global setupScaling
	.global VDP0ApplyScaling
	.global paletteInit
	.global mapSGPalette
	.global paletteTxAll
	.global refreshGfx
	.global transferVRAM
	.global makeBorder
	.global earlyFrame
	.global endFrame
	.global g_colorValue
	.global g_3DEnable
	.global g_twitch
	.global g_flicker
	.global g_gfxMask
	.global vblIrqHandler
	.global SysEVBlHandler
	.global yStart
	.global SPRS
	.global paletteMask
	.global bColor
	.global frameTotal
	.global SystemESetVRAM
	.global SystemEVDPRAMWrite

	.global VDP0SetMode
	.global VDP0ScanlineBPReset
	.global VDP0LatchHCounter

	.global VDP0VCounterR
	.global VDP0HCounterR
	.global VDP0StatR
	.global VDP0DataR
	.global VDP0DataTMSW
	.global VDP0DataSMSW
	.global VDP0DataGGW
	.global VDP0DataMDW
	.global VDP0CtrlW
	.global VDP0CtrlMDW

	.global VDP1VCounterR
	.global VDP1HCounterR
	.global VDP1StatR
	.global VDP1DataR
	.global VDP1DataSMSW
	.global VDP1CtrlW

	.global VDP0
	.global VDP1
	.global VDPRAM				;@ Used by cart.s, init ram.
	.global EMUPALBUFF			;@ Needs to be flushed before dma copied.

	.syntax unified
	.arm

	.section .text
	.align 2

antSeed:
	.long 0x8000
;@----------------------------------------------------------------------------
antWars:
	.type antWars STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}
	bl getRandomNumber

	mov r0,#0x40
	ldr r1,=VDP0
	strb r0,[r1,#vdpMode2Bak2]

	ldr r0,=EMUPALBUFF			;@ Setup palette for antWars.
	mov r4,#0
	strh r4,[r0]
	strh r4,[r0,#0x40]
	ldr r4,=0x7FFF
	strh r4,[r0,#0x1E]

	ldr r4,[r1,#vdpBgrMapOfs1]
	mov r0,#BG_GFX
	add r4,r0,r4,lsl#3
	mov r0,#0
tmLoop:
	add r1,r0,#8
	strh r1,[r4],#2
	add r0,r0,#1
	cmp r0,#1024
	bne tmLoop

	mov r0,r4
	ldr r1,=0x03000300
	mov r2,#0x800/4
	bl memset_					;@ BG2 clear

	ldr r0,=BG_GFX+0x04100
	ldr r3,antSeed
	ldr r1,=0x1E31
antLoop0:
	mov r2,#8
antLoop1:
	movs r3,r3,lsr#1
	eorcs r3,r3,#0x90000
	mov r4,r4,lsl#4
	orrcs r4,r4,#0xF
	subs r2,r2,#1
	bne antLoop1
	str r4,[r0],#4
	subs r1,r1,#1
	bne antLoop0

	str r3,antSeed
	ldmfd sp!,{r4,lr}
	bx lr
;@----------------------------------------------------------------------------
gfxInit:					;@ (called from main.c) only need to call once
	.type gfxInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldr r0,=BG_GFX+0x4000
	mov r1,#0x4000
	bl memclr_					;@ Clear NDS VRAM
	add r0,r0,#0x80
	ldr r1,=0x20202020
	mov r2,#0x10
	bl memset_					;@ BGR color 0
	add r0,r0,#0x40
	ldr r1,=0x30303030
	mov r2,#0x10
	bl memset_					;@ BGR color 1

	mov r1,#0xffffff00			;@ Build chr decode tbl
	ldr r2,=CHRDecode			;@ 0x400
ppi:
	mov r0,#0
	tst r1,#0x01
	orrne r0,r0,#0x10000000
	tst r1,#0x02
	orrne r0,r0,#0x01000000
	tst r1,#0x04
	orrne r0,r0,#0x00100000
	tst r1,#0x08
	orrne r0,r0,#0x00010000
	tst r1,#0x10
	orrne r0,r0,#0x00001000
	tst r1,#0x20
	orrne r0,r0,#0x00000100
	tst r1,#0x40
	orrne r0,r0,#0x00000010
	tst r1,#0x80
	orrne r0,r0,#0x00000001
	str r0,[r2],#4
	adds r1,r1,#1
	bne ppi

	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
gfxReset:					;@ Called with cpuReset
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

//	ldr r0,=gfxstate
	mov r1,#0
//	mov r2,#1					;@ 2*4
//	bl memclr_					;@ Clear GFX regs

	ldr r0,=wTop
	str r1,[r0]
	ldr r0,=yStart
	strb r1,[r0]

	bl VDP0Reset
	bl VDP1Reset

	bl clearTileMaps

	ldr r0,=OAMBuffer1			;@ No stray sprites please
	mov r1,#0x200+SCREEN_HEIGHT
	mov r2,#0x400
	bl memset_
	mov r0,#OAM
	mov r2,#0x100
	bl memset_

	ldr r0,=0x7FFF
	ldr r1,=paletteMask
	str r0,[r1]

	ldr r0,=g_gammaValue
	ldrb r0,[r0]
	bl mapSGPalette
	ldr r0,=g_gammaValue
	ldrb r0,[r0]
	bl paletteInit				;@ Do palette mapping
	bl paletteTxAll				;@ Transfer it

	bl makeBorder
	bl setupScaling

	mov r1,#0
	ldr r0,=g_machine
	ldrb r0,[r0]
	cmp r0,#HW_SYSE
//	cmpne r0,#HW_MEGATECH
	moveq r1,#1
	strb r1,doubleVDP

	ldr r0,=g_emuFlags
	ldr r0,[r0]
	tst r0,#PALTIMING
	moveq r0,#59				;@ NTSC
	movne r0,#49				;@ PAL
	ldr r1,=fpsNominal
	strb r0,[r1]

	ldmfd sp!,{pc}

;@----------------------------------------------------------------------------
HWToVDP:
	;@    Auto            SG-1000         SC-3000         OMV
	.byte VDPSega3155246, VDPTMS9918,     VDPTMS9918,     VDPTMS9918
	;@    SG1000II        Mark 3          SMS1            SMS2
	.byte VDPSega3155066, VDPSega3155124, VDPSega3155124, VDPSega3155246
	;@    GG              MD              Coleco          MSX
	.byte VDPSega3155378, VDPSega3155313, VDPTMS9918,     VDPTMS9918
	;@    SORDM5          Sys-E           SG1k AC         Mega Tech
	.byte VDPTMS9918,     VDPSega3155124, VDPTMS9918,     VDPSega3155246
	.pool
;@----------------------------------------------------------------------------
VDP0Reset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	ldr vdpptr,=VDP0
	ldr r0,=VDPRAM
	str r0,[vdpptr,#VRAMPtr]	;@ This needs to be set before reset.
	ldr r3,=g_machine
	ldrb r3,[r3]
	adr r1,HWToVDP
	ldrb r0,[r1,r3]
	ldr r1,=Z80SetIRQPin
	ldr r2,=Z80SetNMIPin
	cmp r3,#HW_COLECO
	ldreq r1,=Z80SetNMIPin
	cmp r3,#HW_SORDM5
	ldreq r1,=CTC0SetTrg3
	ldr r3,=g_emuFlags
	ldr r3,[r3]
	tst r3,#PALTIMING
	orrne r0,r0,#TVTYPEPAL
	tst r3,#GG_MODE
	orrne r0,r0,#GGMODE
	bl VDPReset				;@ r0=vdp/tv type, r1 = IRQ function ptr, r2 = debounce routine, r12 = vdpptr.
	ldr r0,=VDPRAMCopy0
	str r0,[vdpptr,#VRAMCopyPtr]
	ldr r0,=DirtyTilesBackup0
	str r0,[vdpptr,#vdpDirtyTilesPtr]
	ldr r0,=OAMBuffer1
	str r0,[vdpptr,#vdpTmpOAMBuffer]
	ldr r0,=OAMBuffer2
	str r0,[vdpptr,#vdpDMAOAMBuffer]

	mov r0,#0x0000				;@ BGR map
	str r0,[vdpptr,#vdpBgrMapOfs0]
	mov r0,#0x0300				;@ BGR map
	str r0,[vdpptr,#vdpBgrMapOfs1]
	mov r0,#0x08000				;@ BGR tiles
	str r0,[vdpptr,#vdpBgrTileOfs]
	mov r0,#0x400000			;@ SPR tiles
	str r0,[vdpptr,#vdpSprTileOfs]
	ldmfd sp!,{pc}
;@----------------------------------------------------------------------------
VDP1Reset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	ldr vdpptr,=VDP1
	ldr r0,=VDPRAM+0x8000
	str r0,[vdpptr,#VRAMPtr]	;@ This needs to be set before reset.
	ldr r3,=g_machine
	ldrb r3,[r3]
	adr r1,HWToVDP
	ldrb r0,[r1,r3]
	ldr r1,=Z80SetNMIPin
	mov r2,#0
	bl VDPReset				;@ r0=vdp type, r1 = IRQ function ptr, r2 = debounce routine, r12 = vdpptr.
	ldr r1,=VDP0
	ldr r0,[r1,#vdpScanline]
	add r0,r0,#1
	bl VDPSetScanline
	ldr r0,=VDPRAMCopy1
	str r0,[vdpptr,#VRAMCopyPtr]
	ldr r0,=DirtyTilesBackup1
	str r0,[vdpptr,#vdpDirtyTilesPtr]
	ldr r0,=OAMBuffer3
	str r0,[vdpptr,#vdpTmpOAMBuffer]
	ldr r0,=OAMBuffer4
	str r0,[vdpptr,#vdpDMAOAMBuffer]

	mov r0,#0x0900				;@ BGR map
	str r0,[vdpptr,#vdpBgrMapOfs0]
	mov r0,#0x0C00				;@ BGR map
	str r0,[vdpptr,#vdpBgrMapOfs1]
	mov r0,#0x0C000				;@ BGR tiles
	str r0,[vdpptr,#vdpBgrTileOfs]
	ldr r0,=0x404000			;@ SPR tiles
	str r0,[vdpptr,#vdpSprTileOfs]
	ldmfd sp!,{pc}
;@----------------------------------------------------------------------------
clearTileMaps:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	add r3,vdpptr,#vdpBgrMapOfs0
	mov r6,#2
clrtmLoop2:
	ldr r4,[r3],#4
	mov r0,#BG_GFX
	add r4,r0,r4,lsl#3
	mov r5,#2
clrtmLoop1:
	add r0,r4,#0x700
	add r4,r4,#0x800
	mov r1,#0x100/4
	bl memclr_
	subs r5,r5,#1
	bne clrtmLoop1
	subs r6,r6,#1
	bne clrtmLoop2
	ldmfd sp!,{r4-r6,pc}
;@----------------------------------------------------------------------------
makeBorder:					;@ Also called from UI.c, r0 = border type.
	.type   makeBorder STT_FUNC
;@----------------------------------------------------------------------------

	mov r1,#0x00C0				;@ H start-end
	mov r2,#0x08C0				;@ H start-end
	mov r12,#0xC000				;@ H start-end Win1

	ldrb r0,bColor
	cmp r0,#2
	beq setBorderValues

	ldr r0,=g_machine
	ldrb r0,[r0]
	cmp r0,#HW_GG

	moveq r1,#0x08C0			;@ H start-end
	ldreq r12,=0xC0F8			;@ H start-end Win1

	ldr r0,=g_emuFlags
	ldrb r0,[r0]
	tst r0,#GG_MODE

	ldrne r1,=0x30C0			;@ H start-end
	ldrne r2,=0x30C0			;@ H start-end
	ldrne r12,=0xC0D0			;@ H start-end Win1
setBorderValues:
	str r1,Window0HValue_normal
	str r2,Window0HValue_col0
	str r12,Window1HValue
	
	mov r1,#REG_BASE
	mov r0,#0x0000
	strh r0,[r1,#REG_WINOUT]

	bx lr

;@----------------------------------------------------------------------------
setupScaling:		;@ r0-r3, r12 modified.
	.type   setupScaling STT_FUNC
;@----------------------------------------------------------------------------
	ldrb r0,bColor
	cmp r0,#2
	ldr r3,=g_emuFlags
	ldrb r3,[r3]
	biceq r3,r3,#GG_MODE
	ldr r2,=g_machine
	ldrb r2,[r2]
	ldr r1,=g_scalingSet
	ldrb r1,[r1]
	adr r0,BG_SCALING_1_1

	tst r3,#GG_MODE
	adrne r0,BG_SCALING_1_1_GG
	cmp r1,#SCALED_1_1
	beq loadScaleValues

	cmp r1,#SCALED_FIT
	bne noFit
	adr r0,BG_SCALING_TO_FIT
	tst r3,#GG_MODE
	adrne r0,BG_SCALING_1_1_GG
	b loadScaleValues
noFit:
	cmp r1,#SCALED_ASPECT
	bne loadScaleValues

	adr r0,BG_SCALING_ASPECT_NTSC
	tst r3,#PALTIMING
	adrne r0,BG_SCALING_ASPECT_PAL
	cmp r2,#HW_GG
	adreq r0,BG_SCALING_ASPECT_GG
	tst r3,#GG_MODE
	adrne r0,BG_SCALING_ASPECT_GGMODE

loadScaleValues:
	ldmia r0!,{r1-r3}
	adr r12,BG_SCALING_TBL
	stmia r12,{r1-r3}

	ldmia r0!,{r1-r3}
	adr r12,BG_SCALING_WIN
	stmia r12,{r1-r3}

	ldmia r0!,{r1-r3}
	adr r12,BG_SCALING_OFS
	stmia r12,{r1-r3}

	ldmia r0!,{r1-r3}
	adr r12,scaleSprParam
	stmia r12,{r1-r3}
	
	b buildSpriteScaling

BG_SCALING_TO_FIT:	;@ 1:1, 7:6, 5:4
	.long 0xFFFF,0xDB6D,0xCCCD
	.long 0x00C0,0x00C0,0x00C0
	.long 0x0000,0x0000,0x0000
	.long 0x0100,0x0100,0x0080
BG_SCALING_1_1:
	.long 0xFFFF,0xFFFF,0xFFFF
	.long 0x00C0,0x00C0,0x00C0
	.long 0x0000,0x0010,0x0018
	.long 0x0100,0x0100,0x0080
BG_SCALING_1_1_GG:
	.long 0xFFFF,0xFFFF,0xFFFF
	.long 0x18A8,0x18A8,0x18A8
	.long 0x0000,0x0010,0x0018
	.long 0x0100,0x0100,0x0080
BG_SCALING_ASPECT_PAL:			;@ 192->142, 224->165, 240->177
	.long 0xBD56,0xBD56,0xBD56
	.long 0x19A7,0x0DB2,0x00C0
	.long    -34,   -17,    -8
	.long 0x0150,0x0150,0x00AD
BG_SCALING_ASPECT_NTSC:			;@ 192->170, 224->199, 240->213, 216->192, 9->8
	.long 0xE38F,0xE2AB,0xE2AB
	.long 0x0BB5,0x00C0,0x00C0
	.long    -12,0x0004,0x000C
	.long 0x0100,0x0120,0x0090
BG_SCALING_ASPECT_GG:			;@ 192->160, 216->180, 6->5
	.long 0xD555,0xD555,0xD555
	.long 0x10B0,0x07BA,0x06BA
	.long    -19,    -3,0x0008
	.long 0x0133,0x0133,0x0092
BG_SCALING_ASPECT_GGMODE:		;@ 160x144 -> 160x120, 6->5
	.long 0xD555,0xD555,0xD555
	.long 0x249C,0x249C,0x249C
	.long    -19,    -3,0x0008
	.long 0x0133,0x0133,0x0092

BG_SCALING_TBL:
	.long 0,0,0
BG_SCALING_WIN:
	.long 0x00C0,0x00C0,0x00C0
BG_SCALING_OFS:
	.long 0,0,0

scaleParms:
	.long 0x0000				;@ Rotate value
	.long 0x0100				;@ Normal
	.long 0x0080				;@ Double
scaleSprParam:
	.long 0x0100				;@ Scaled Normal Vertical
	.long 0x0120				;@ Scaled 8x16 Vertical
	.long 0x0090				;@ Scaled Double Vertical
	.long OAMBuffer1+6
	.long OAM+768+6
;@----------------------------------------------------------------------------
buildSpriteScaling:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r8}
	adr r7,scaleParms			;@ Set sprite scaling params
	ldmia r7,{r0-r7}

	mov r8,#4
scaleLoop:
	strh r1,[r6],#8				;@ buffer1, buffer2. normal sprites
	strh r0,[r6],#8
	strh r0,[r6],#8
	strh r3,[r6],#8
		strh r1,[r6],#8			;@ 8x16 sprites
		strh r0,[r6],#8
		strh r0,[r6],#8
		strh r4,[r6],#8
			strh r2,[r6],#8		;@ Double sprites
			strh r0,[r6],#8
			strh r0,[r6],#8
			strh r5,[r6],#8
				strh r2,[r6],#8	;@ 8x16 double sprites
				strh r0,[r6],#8
				strh r0,[r6],#8
				strh r5,[r6],#8
	add r6,r6,#0x380
	subs r8,r8,#1
	bne scaleLoop

	strh r3,[r7],#8				;@ 0x07000306
	strh r0,[r7],#8
	strh r0,[r7],#8
	strh r4,[r7]

	ldmfd sp!,{r4-r8}
	bx lr
;@----------------------------------------------------------------------------
VDP0ApplyScaling:		;@ r0-r3, r12 modified.
	.type VDP0ApplyScaling STT_FUNC
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
;@----------------------------------------------------------------------------
applyScaling:		;@ r0-r3, r12 modified.
;@----------------------------------------------------------------------------
	ldrb r0,[vdpptr,#vdpHeightMode]
	and r0,r0,#VDPMODE_HEIGHTMASK	;@ 224 and/or 240 height
	ldr r2,=BG_SCALING_TBL
	ldr r1,[r2,r0,lsr#2]
	str r1,bg_scaleValue
	ldr r2,=BG_SCALING_WIN
	ldr r1,[r2,r0,lsr#2]
	str r1,WindowVValue
	ldr r2,=BG_SCALING_OFS
	ldr r1,[r2,r0,lsr#2]
	ldr r0,=yStart
	strb r1,[r0]
	bx lr
;@----------------------------------------------------------------------------
paletteInit:		;@ r0-r3 modified.
	.type   paletteInit STT_FUNC
;@ Called by ui.c:  void paletteInit(u8 gammaVal);
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r9,lr}
	mov r7,r0					;@ Gamma value = 0 -> 4
	ldr vdpptr,=VDP0
	ldr r6,=mappedRGB
	ldrb r8,g_colorValue		;@ Color value = 0 -> 4
	mov r4,#4096*2
	sub r4,r4,#2
noMap:							;@ Map BGR12  ->  BGR15
	mov r0,r4,lsr#1
	bl VDPGetRGBFromIndex
	mov r1,r8
	bl yConvert
	mov r9,r0

	mov r1,r7
	mov r0,r9,lsr#16
	bl gammaConvert
	mov r5,r0

	mov r0,r9,lsr#8
	and r0,r0,#0xFF
	bl gammaConvert
	orr r5,r0,r5,lsl#5

	and r0,r9,#0xFF
	bl gammaConvert
	orr r5,r0,r5,lsl#5

	strh r5,[r6,r4]
	subs r4,r4,#2
	bpl noMap
	ldmfd sp!,{r4-r9,lr}
	bx lr

;@----------------------------------------------------------------------------
mapSGPalette:
	.type   mapSGPalette STT_FUNC
;@ Called by ui.c:  void mapSGPalette(u8 gammaVal);
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	mov r10,r0					;@ Gamma value = 0 -> 4
	ldr vdpptr,=VDP0
	ldr r7,=EMUPALBUFF+0x80
	ldr r8,=EMUPALBUFF+0x202
	ldr r9,=BG_PALETTE+0x202
	mov r4,#16
mapSGLoop:						;@ Map RGB24  ->  BGR15
	rsb r0,r4,#16
	bl VDPGetRGBFromIndexSG
	ldrb r1,g_colorValue
	bl yConvert
	mov r11,r0

	mov r1,r10
	mov r0,r11,lsr#16
	bl gammaConvert
	mov r5,r0

	mov r0,r11,lsr#8
	and r0,r0,#0xff
	bl gammaConvert
	orr r5,r0,r5,lsl#5

	and r0,r11,#0xff
	bl gammaConvert
	orr r5,r0,r5,lsl#5

	strh r5,[r7],#2
	strh r5,[r8],#0x20
	strh r5,[r9],#0x20
	subs r4,r4,#1
	bne mapSGLoop
	ldmfd sp!,{r4-r11,lr}
	bx lr

;@----------------------------------------------------------------------------
yPrefix:					;@ Takes r0=BGR12, outputs r0=BGR24
;@----------------------------------------------------------------------------
	mov r2,r0,lsr#8				;@ Blue
	and r1,r0,#0xF0				;@ Green
	and r0,r0,#0x0F				;@ Red
	orr r0,r0,r1,lsl#8
	orr r0,r0,r2,lsl#16
	orr r0,r0,r0,lsl#4
;@----------------------------------------------------------------------------
yConvert:					;@ r0=BGR24, r1=color 0-4, outputs r0=BGR24
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r5}
	mov r3,r0,lsr#16
	mov r2,r0,lsr#8
	and r2,r2,#0xFF
	and r0,r0,#0xFF

	mov r5,#77
	mul r4,r5,r0				;@ Red
	mov r5,#151
	mla r4,r5,r2,r4				;@ Green
	mov r5,#29
	mla r4,r5,r3,r4				;@ Blue

	rsb r5,r1,#4
	mul r4,r5,r4				;@ B&W
	orr r0,r0,r0,lsl#8
	mla r0,r1,r0,r4
	mov r0,r0,lsr#10
	
	orr r3,r3,r3,lsl#8
	mla r3,r1,r3,r4
	mov r3,r3,lsr#10

	orr r2,r2,r2,lsl#8
	mla r2,r1,r2,r4
	mov r2,r2,lsr#10

	orr r0,r0,r2,lsl#8
	orr r0,r0,r3,lsl#16

	ldmfd sp!,{r4-r5}
	bx lr
;@----------------------------------------------------------------------------
gPrefix:
	orr r0,r0,r0,lsl#4
;@----------------------------------------------------------------------------
gammaConvert:	;@ Takes value in r0(0-0xFF), gamma in r1(0-4),returns new value in r0=0x1F
;@----------------------------------------------------------------------------
	rsb r2,r0,#0x100
	mul r3,r2,r2
	rsbs r2,r3,#0x10000
	rsb r3,r1,#4
	orr r0,r0,r0,lsl#8
	mul r2,r1,r2
	mla r0,r3,r0,r2
	mov r0,r0,lsr#13

	bx lr
;@----------------------------------------------------------------------------
paletteTxAll:				;@ Called from ui.c
	.type   paletteTxAll STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r8,lr}
	ldr r12,=VDP0
	add r12,r12,#vdpPaletteRAM
	ldr r3,=mappedRGB
	ldr r4,=EMUPALBUFF
	ldr r8,=paletteMask
	ldr r5,[r8]
	mov r2,#0x40
	bl copyPalette

	ldr r2,=g_machine
	ldrb r2,[r2]
	cmp r2,#HW_SYSE
//	cmpne r2,#HW_MEGATECH
	;@ Do this only for System-E
	ldreq r12,=VDP1
	addeq r12,r12,#vdpPaletteRAM
	addeq r4,r4,#0x40
	mov r2,#0
	bleq copyPalette

	ldr r0,=0x7FFF
	str r0,[r8]					;@ Restore palettemask
	ldmfd sp!,{r4-r8,lr}
	bx lr

copyPalette:
	mov r6,#0
txLoop:
	ldrh r0,[r12],#2

	mov r0,r0,lsl#1
	ldrh r0,[r3,r0]
	and r0,r0,r5				;@ Palette mask for anaglyph 3D.
	add r7,r4,r6,lsl#1
	movs r1,r6,lsl#28
	strhne r0,[r7]				;@ Bgtile palette
	strheq r0,[r7,r2]			;@ Background palette
	movcs r1,#480
	strhcs r0,[r7,r1]			;@ Sprite palette

	add r6,r6,#1
	cmp r6,#0x20
	bmi txLoop
	bx lr

;@----------------------------------------------------------------------------
paletteTxGGSG:				;@ For SG modes on GG
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r5}
	ldr r2,=EMUPALBUFF+2
	add r3,r2,#0x80
	ldr r4,=EMUPALBUFF+0x202
	add r5,r4,#0x20
	mov r1,#0xf
ggSGLoop:
	ldrh r0,[r2],#2
	strh r0,[r3],#2
	ldrh r0,[r4],#2
	strh r0,[r5],#0x20
	subs r1,r1,#1
	bne ggSGLoop

	ldmfd sp!,{r4-r5}
	bx lr

.pool


;@----------------------------------------------------------------------------
delayVRAM:					;@ This is used to get rid of graphics corruption
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r9,lr}

	ldr r0,[vdpptr,#vdpDirtyTilesPtr]
	add r1,vdpptr,#dirtyTiles
	ldr r4,[vdpptr,#VRAMPtr]
	ldr r5,[vdpptr,#VRAMCopyPtr]

	mov r2,#0x200
	mov r9,#-1
	sub r2,r2,#1
dVRamLoop0:
	ldrb r3,[r1,r2]
	cmp r3,#0
	bleq dVRamCopy
	subs r2,r2,#1
	bpl dVRamLoop0

	ldmfd sp!,{r4-r9,lr}
	bx lr

dVRamCopy:
	strb r9,[r1,r2]
	strb r3,[r0,r2]

	add r6,r4,r2,lsl#5
	add r7,r5,r2,lsl#5
	mov r8,#0x8
dVRamLoop1:
	ldr r3,[r6],#4
	str r3,[r7],#4
	subs r8,r8,#1
	bhi dVRamLoop1
	bx lr

;@----------------------------------------------------------------------------
tData:
//	.long dirtyTiles
	.long DirtyTilesBackup0
VDPRAMPtr:
	.long VDPRAMCopy0
	.long CHRDecode
//	.long BG_GFX+0x08000		;@ BGR tiles
//	.long BG_GFX+0x400000		;@ SPR tiles
;@----------------------------------------------------------------------------
transferVRAM:
;@----------------------------------------------------------------------------
	stmfd sp!,{r12,lr}
	adr r0,tData
	ldmia r0,{r4-r6}
	ldr r4,[vdpptr,#vdpDirtyTilesPtr]
	ldr r5,[vdpptr,#VRAMCopyPtr]
	ldr r7,[vdpptr,#vdpBgrTileOfs]
	ldr r8,[vdpptr,#vdpSprTileOfs]
	add r7,r7,#BG_GFX
	add r8,r8,#BG_GFX
	mov r1,#0

	ldrb r0,[vdpptr,#vdpRealMode]
	cmp r0,#VDPMODE_4
	beq transferVRAM_m4
	cmp r0,#VDPMODE_5
	beq transferVRAM_m5
	ldrb r2,[vdpptr,#vdpMode2]
	tst r2,#0x40				;@ Screen on?
	ldmfdeq sp!,{r12,pc}
	ldr r8,=0x11111111
	ldrb r2,[vdpptr,#vdpPGOffsetBak1]
	ldrb r3,[vdpptr,#vdpCTOffset]
	cmp r0,#VDPMODE_2
	beq	transferVRAM_m2
	cmp r0,#VDPMODE_0
	beq	transferVRAM_m0
	cmp r0,#VDPMODE_1
	beq	transferVRAM_m1
	cmp r0,#VDPMODE_3
	beq	transferVRAM_m3
	ldmfd sp!,{r12,pc}

;@----------------------------------------------------------------------------
transferVRAM_m0:
;@----------------------------------------------------------------------------
	and r2,r2,#0x07
	add r11,r5,r3,lsl#6
	add r5,r5,r2,lsl#11
	ldrb r9,[r4,r3,lsl#1]
	orr r0,r9,#0x01
	strb r0,[r4,r3,lsl#1]
	orr r9,r9,r9,lsl#8
	orr r9,r9,r9,lsl#16
	add r4,r4,r2,lsl#6
tileLoop0_0:
	ldr r0,=0x01010101			;@ Dirtytiles mode 0 bgr.
	ldr r12,[r4]
	orr r2,r12,r0
	str r2,[r4],#4
	and r12,r12,r9
	tst r12,#0x00000001
	addne r1,r1,#0x20
	bleq tileLoop0_1
	tst r12,#0x00000100
	addne r1,r1,#0x20
	bleq tileLoop0_1
	tst r12,#0x00010000
	addne r1,r1,#0x20
	bleq tileLoop0_1
	tst r12,#0x01000000
	addne r1,r1,#0x20
	bleq tileLoop0_1
	cmp r1,#0x2000
	bne tileLoop0_0

	b tileLoopSpr
;@----------------------------------------------------------------------------
transferVRAM_m1:
;@----------------------------------------------------------------------------
	and r2,r2,#0x07
	add r5,r5,r2,lsl#11
	add r4,r4,r2,lsl#6
	ldr r10,=0x02020202			;@ Dirtytiles mode 1 bgr.
tileLoop1_0:
	ldr r12,[r4]
	orr r2,r12,r10
	str r2,[r4],#4
	tst r12,#0x00000002
	addne r1,r1,#0x20
	bleq tileLoop1_1
	tst r12,#0x00000200
	addne r1,r1,#0x20
	bleq tileLoop1_1
	tst r12,#0x00020000
	addne r1,r1,#0x20
	bleq tileLoop1_1
	tst r12,#0x02000000
	addne r1,r1,#0x20
	bleq tileLoop1_1
	cmp r1,#0x0800
	bne tileLoop1_0

	ldmfd sp!,{r12,pc}

;@----------------------------------------------------------------------------
transferVRAM_m2:
;@----------------------------------------------------------------------------
	and r2,r2,#0x04
	and r3,r3,#0x80
	add r11,r5,r3,lsl#6
	add r9,r4,r3,lsl#1
	add r5,r5,r2,lsl#11
	add r4,r4,r2,lsl#6
	ldr r10,=0x04040404			;@ Dirtytiles mode 2 bgr.

tileLoop2_0:
	ldr r12,[r4]
	orr r2,r12,r10
	str r2,[r4],#4
	ldr r2,[r9]
	and r12,r12,r2
	orr r2,r2,r10
	str r2,[r9],#4
	tst r12,#0x00000004
	addne r1,r1,#0x20
	bleq tileLoop2_2
	tst r12,#0x00000400
	addne r1,r1,#0x20
	bleq tileLoop2_2
	tst r12,#0x00040000
	addne r1,r1,#0x20
	bleq tileLoop2_2
	tst r12,#0x04000000
	addne r1,r1,#0x20
	bleq tileLoop2_2
	cmp r1,#0x2000
	bne tileLoop2_0

	b tileLoopSpr
;@----------------------------------------------------------------------------
transferVRAM_m3:
;@----------------------------------------------------------------------------
	and r2,r2,#0x07
	add r5,r5,r2,lsl#11
	add r4,r4,r2,lsl#6
	ldr r10,=0x08080808			;@ Dirtytiles mode 3 bgr.
tileLoop3_0:
	ldr r12,[r4]
	orr r2,r12,r10
	str r2,[r4],#4
	tst r12,#0x00000008
	addne r1,r1,#0x20
	bleq tileLoop3_1
	tst r12,#0x00000800
	addne r1,r1,#0x20
	bleq tileLoop3_1
	tst r12,#0x00080000
	addne r1,r1,#0x20
	bleq tileLoop3_1
	tst r12,#0x08000000
	addne r1,r1,#0x20
	bleq tileLoop3_1
	cmp r1,#0x800
	bne tileLoop3_0

;@----------------------------------------------------------------------------
tileLoopSpr:				;@ Mode0, 2 & 3 sprites.
;@----------------------------------------------------------------------------
	ldmfd sp!,{r12}
	ldr r9,=0x10101010			;@ Dirtytiles mode 0, 2 & 3 spr.
	ldrb r1,[vdpptr,#vdpSPROffset]
	and r1,r1,#0x07
	mov r1,r1,lsl#11
	ldr r4,tData
	ldr r5,VDPRAMPtr
	add r4,r4,r1,lsr#5
	add r7,r7,#0x400000			;@ Sprites @ 0x06406000
	sub r7,r7,#0x2000			;@ Sprites @ 0x06406000
	sub r7,r7,r1,lsl#2			;@ r1 is later added in the render loop.
	add r8,r1,#0x800
tileLoop2_1:
	ldr r10,[r4]
	orr r2,r10,r9
	str r2,[r4],#4
	tst r10,#0x00000010
	addne r1,r1,#0x20
	bleq tileLoop1_1
	tst r10,#0x00001000
	addne r1,r1,#0x20
	bleq tileLoop1_1
	tst r10,#0x00100000
	addne r1,r1,#0x20
	bleq tileLoop1_1
	tst r10,#0x10000000
	addne r1,r1,#0x20
	bleq tileLoop1_1
	cmp r1,r8
	bne tileLoop2_1

	ldmfd sp!,{pc}

;@----------------------------------------------------------------------------
;@----------------------------------------------------------------------------
tileLoop0_1:
	bic r2,r1,#0x1800
	ldrb r2,[r11,r2,lsr#6]
	mov r3,r2,lsr#4
	and r2,r2,#0x0F
tileLoop0_2:
	ldrb r0,[r5,r1]
	ldr r0,[r6,r0,lsl#2]
	mul r10,r0,r3
	eor r0,r0,r8
	mla r10,r0,r2,r10
	str r10,[r7,r1,lsl#2]
	add r1,r1,#1
	tst r1,#0x1F
	bne tileLoop0_2
	bx lr

tileLoop1_1:
	ldrb r0,[r5,r1]
	ldr r0,[r6,r0,lsl#2]
	str r0,[r7,r1,lsl#2]
	add r1,r1,#1
	tst r1,#0x1F
	bne tileLoop1_1
	bx lr

tileLoop2_2:
	ldrb r2,[r11,r1]
	ldrb r0,[r5,r1]
	ldr r0,[r6,r0,lsl#2]
	movs r3,r2,lsr#4
	mulne r3,r0,r3
	eor r0,r0,r8
	ands r2,r2,#0x0F
	mlane r3,r0,r2,r3
	str r3,[r7,r1,lsl#2]
	add r1,r1,#1
	tst r1,#0x1F
	bne tileLoop2_2
	bx lr

tileLoop3_1:
	ldrb r0,[r5,r1]
	mov r0,r0,ror#4
	orr r0,r0,r0,lsr#12
	orr r0,r0,r0,lsl#4
	orr r0,r0,r0,lsl#8
	add r11,r7,r1,lsl#4
	str r0,[r11],#4
	str r0,[r11],#4
	str r0,[r11],#4
	str r0,[r11],#4
	add r1,r1,#1
	tst r1,#0x1F
	bne tileLoop3_1
	bx lr

;@----------------------------------------------------------------------------
transferVRAM_m4:
;@----------------------------------------------------------------------------
	ldr r9,=0x20202020			;@ Dirtytiles mode4 bgr & spr

tileLoop4_0:
	ldr r10,[r4]
	str r9,[r4],#4
	tst r10,#0x00000020
	addne r1,r1,#0x20
	bleq tileLoop4_1
	tst r10,#0x00002000
	addne r1,r1,#0x20
	bleq tileLoop4_1
	tst r10,#0x00200000
	addne r1,r1,#0x20
	bleq tileLoop4_1
	tst r10,#0x20000000
	addne r1,r1,#0x20
	bleq tileLoop4_1
	cmp r1,#0x4000
	bne tileLoop4_0

	ldmfd sp!,{r12,pc}

tileLoop4_1:
	ldr r0,[r5,r1]

	ands r3,r0,#0x000000FF
	ldrne r3,[r6,r3,lsl#2]
	ands r2,r0,#0x0000FF00
	ldrne r2,[r6,r2,lsr#6]
	orrne r3,r3,r2,lsl#1
	ands r2,r0,#0x00FF0000
	ldrne r2,[r6,r2,lsr#14]
	orrne r3,r3,r2,lsl#2
	ands r2,r0,#0xFF000000
	ldrne r2,[r6,r2,lsr#22]
	orrne r3,r3,r2,lsl#3

	str r3,[r7,r1]
	str r3,[r8,r1]
	add r1,r1,#4
	tst r1,#0x1C
	bne tileLoop4_1

	bx lr
;@----------------------------------------------------------------------------
transferVRAM_m5:
;@----------------------------------------------------------------------------
	ldr r9,=0x40404040			;@ Dirtytiles mode5 bgr & spr

tileLoop5_0:
	ldr r10,[r4]
	str r9,[r4],#4
	tst r10,#0x00000040
	addne r1,r1,#0x20
	bleq tileLoop5_1
	tst r10,#0x00004000
	addne r1,r1,#0x20
	bleq tileLoop5_1
	tst r10,#0x00400000
	addne r1,r1,#0x20
	bleq tileLoop5_1
	tst r10,#0x40000000
	addne r1,r1,#0x20
	bleq tileLoop5_1
	cmp r1,#0x4000
	bne tileLoop5_0

	ldmfd sp!,{r12,pc}

tileLoop5_1:
	ldr r0,[r5,r1]

	str r0,[r7,r1]
	str r0,[r8,r1]
	add r1,r1,#4
	tst r1,#0x1C
	bne tileLoop5_1

	bx lr

;@----------------------------------------------------------------------------
vblIrqHandler:
	.type vblIrqHandler STT_FUNC
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	ldrb r0,doubleVDP
	cmp r0,#0
	bne SysEVBlHandler
	stmfd sp!,{r4-r11,lr}
	bl calculateFPS

	ldrb r3,[vdpptr,#vdpYScrollBak1]
	ldr r7,[vdpptr,#vdpScrollMask]
	add r5,vdpptr,#scrollBuff
	ldr r4,=yStart
	ldrsb r4,[r4]
	add r5,r5,r4
	adds r3,r3,r4
	addmi r3,r3,r7

	ldr r6,bg_scaleValue

	mul r1,r6,r3
	beq noLoop
	sub r6,r6,r1,lsl#16
	cmn r6,r6,lsl#16
	mov r1,#0
	movcc r1,#1
@	rsbs r1,r3,#0
@	bpl noLoop
@setLoop:
@	adds r6,r6,r6,lsl#16
@	adcs r1,r1,#1
@	bmi setLoop					;@ r1 will come out as 0 or 1.
noLoop:

	ldrb r0,g_flicker
	ldrb r2,g_twitch
	eors r2,r2,r0
	strb r2,g_twitch
	beq noJump
	subs r6,r6,r6,lsl#16
	movcs r1,#1
noJump:
	add r5,r5,r1
	add r3,r3,r1
	cmp r1,#0
	subne r6,r6,r6,lsl#16

	ldr r2,=DMA0Buff
	mov r4,r2
	mov r8,r3
	subs r3,r3,r7
	subpl r3,r3,r7
	subpl r8,r8,r7
	mov r10,#SCREEN_HEIGHT
scrolLoop2:
	ldrb r11,[r5],#1
	add r0,r11,r8,lsl#16
	mov r1,r0
	mov r9,r0
	stmia r4!,{r0-r1,r9,r11}
	subs r6,r6,r6,lsl#16
	subcs r6,r6,r6,lsl#16
	adc r8,r8,#0
	adc r5,r5,#0
	adcs r3,r3,#1
	subcs r8,r8,r7
	subs r10,r10,#1
	bne scrolLoop2



	mov r6,#REG_BASE
	strh r6,[r6,#REG_DMA0CNT_H]	;@ DMA0 stop

	add r1,r6,#REG_DMA0SAD
;@	mov r2,r2					;@ Setup DMA buffer for scrolling:
	ldmia r2!,{r4,r5,r7,r8}		;@ Read
	add r3,r6,#REG_BG0HOFS		;@ DMA0 always goes here
	stmia r3,{r4-r5,r7,r8}		;@ Set 1st value manually, HBL is AFTER 1st line
	ldr r4,=0x96600004			;@ noIRQ hblank 32bit repeat incsrc inc_reloaddst, 4 word
	stmia r1,{r2-r4}			;@ DMA0 go

	add r1,r6,#REG_DMA3SAD

	ldr r2,[vdpptr,#vdpDMAOAMBuffer]	;@ DMA3 src, OAM transfer:
	mov r3,#OAM					;@ DMA3 dst
	mov r4,#0x84000000			;@ noIRQ 32bit incsrc incdst
	orr r4,r4,#0x100			;@ 128 sprites (1024 bytes)
	stmia r1,{r2-r4}			;@ DMA3 go

	ldr r2,=EMUPALBUFF			;@ DMA3 src, Palette transfer:
	mov r3,#BG_PALETTE			;@ DMA3 dst
	mov r4,#0x84000000			;@ noIRQ 32bit incsrc incdst
	orr r4,r4,#0x100			;@ 256 words (1024 bytes)
	stmia r1,{r2-r4}			;@ DMA3 go

	ldr r2,[vdpptr,#vdpBgrMapOfs1]
	add r0,r2,#0x0005
	strh r0,[r6,#REG_BG0CNT]
	ldr r3,=0x02870102
	add r0,r2,r3
	ldr r1,[vdpptr,#vdpBgrTileOfs]
	add r0,r0,r1,lsr#12
	strh r0,[r6,#REG_BG1CNT]
	strh r0,[r6,#REG_BG3CNT]
	add r0,r2,r3,lsr#16
	strh r0,[r6,#REG_BG2CNT]

	mov r0,#0x001F
	ldrb r1,g_gfxMask
	bic r0,r0,r1
	ldrb r1,[vdpptr,#vdpMode2Bak2]
	tst r1,#0x40
	biceq r0,r0,#0x001F			;@ Turn off sprites and bg
	orr r0,r0,r0,lsl#8
	bic r0,r0,#0x0008

	ldrb r1,[vdpptr,#vdpMode1]
	tst r1,#0x80				;@ Columns 24-31 locked?
	biceq r0,r0,#0x0800
	bicne r0,r0,#0x0300

	strh r0,[r6,#REG_WININ]

	tst r1,#0x20				;@ Column 0 blanked?
	ldreq r0,Window0HValue_normal
	ldrne r0,Window0HValue_col0
	strh r0,[r6,#REG_WIN0H]
	ldr r0,Window1HValue
	strh r0,[r6,#REG_WIN1H]
	ldr r0,WindowVValue
	strh r0,[r6,#REG_WIN0V]
	strh r0,[r6,#REG_WIN1V]

@	ldrh r0,DisplayControl		;@ 1d sprites, Win0, OBJ, BG0/1/2/3 enable. mode0.
@	orrne r0,r0,#0x6000			;@ Enable Win0 & 1
@	ldrb r2,[vdpptr,#vdpMode2Bak2]
@	tst r2,#0x40
@	biceq r0,r0,#0x1700			;@ Turn off sprites and bg
@	ldrb r2,gfxLayerMask
@	bic r0,r0,r2,lsl#8

	ldrb r4,[vdpptr,#vdpTVType]
	blx scanKeys
	tst r4,#TVTYPEPAL
	beq exitVbl
	ldr r0,=pauseEmulation
	ldr r0,[r0]
	cmp r0,#0
	bne exitVbl
hz50Start:
	mov r0,#5
hz50Loop0:
	ldrh r1,[r6,#REG_VCOUNT]
	cmp r1,#212
	beq hz50Loop0
hz50Loop1:
	ldrh r1,[r6,#REG_VCOUNT]
	cmp r1,#212
	bne hz50Loop1
	mov r1,#202
	strh r1,[r6,#REG_VCOUNT]
	subs r0,r0,#1
	bne hz50Loop0
exitVbl:
	ldmfd sp!,{r4-r11,pc}

;@----------------------------------------------------------------------------
DisplayControl:
	.short 0x3F40,0
Window0HValue_normal:
	.long 0x00E0
Window0HValue_col0:
	.long 0x08E0
Window1HValue:
	.long 0xE000
WindowVValue:
	.long 0x00C0
;@----------------------------------------------------------------------------
bg_scaleValue:	.long 0x00002B10			;@ was 0x2AAB
g_twitch:		.byte 0
g_flicker:		.byte 1
g_colorValue:	.byte 4
g_gfxMask:		.byte 0
bColor:			.byte 0
g_3DEnable:		.byte 1
doubleVDP:		.byte 0
				.byte 0
;@----------------------------------------------------------------------------
SysEVBlHandler:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	bl calculateFPS
	bl SysESprFix

	ldr vdpptr,=VDP0
	ldrb r3,[vdpptr,#vdpYScrollBak1]
	ldr r7,[vdpptr,#vdpScrollMask]
	add r5,vdpptr,#scrollBuff
	ldr r4,=yStart
	ldrsb r4,[r4]
	add r5,r5,r4
	adds r3,r3,r4
	addmi r3,r3,r7

	ldr r6,bg_scaleValue
	mul r1,r6,r3
	beq noELoop
	sub r6,r6,r1,lsl#16
	cmn r6,r6,lsl#16
	mov r1,#0
	movcc r1,#1
noELoop:

	ldrb r0,g_flicker
	ldrb r2,g_twitch
	eors r2,r2,r0
	strb r2,g_twitch
	beq noEJump
	subs r6,r6,r6,lsl#16
	movcs r1,#1
noEJump:
	add r5,r5,r1
	add r3,r3,r1
	cmp r1,#0
	subne r6,r6,r6,lsl#16

	mov r8,r3
	ldr r2,=VDP1
	add r2,r2,#scrollBuff
	ldr r4,=DMA0Buff
	subs r3,r3,r7
	subpl r3,r3,r7
	subpl r8,r8,r7
	mov r10,#SCREEN_HEIGHT
scrolELoop2:
	ldrb r0,[r5],#1
	add r0,r0,r8,lsl#16
	mov r1,r0
	mov r9,r0
	ldrb r11,[r2],#1
//	add r11,r11,r8,lsl#16
	stmia r4!,{r0-r1,r9,r11}
	subs r6,r6,r6,lsl#16
	subcs r6,r6,r6,lsl#16
	adc r8,r8,#0
	adc r2,r2,#0
	adc r5,r5,#0
	adcs r3,r3,#1
	subcs r8,r8,r7
	subs r10,r10,#1
	bne scrolELoop2



	ldr r2,=DMA0Buff
	mov r6,#REG_BASE
	strh r6,[r6,#REG_DMA0CNT_H]	;@ DMA0 stop

	add r1,r6,#REG_DMA0SAD
@	mov r2,r2					;@ Setup DMA buffer for scrolling:
	ldmia r2!,{r4,r5,r7,r8}		;@ Read
	add r3,r6,#REG_BG0HOFS		;@ DMA0 always goes here
	stmia r3,{r4-r5,r7,r8}		;@ Set 1st value manually, HBL is AFTER 1st line
	ldr r4,=0x96600004			;@ noIRQ hblank 32bit repeat incsrc inc_reloaddst, 4 word
	stmia r1,{r2-r4}			;@ DMA0 go

	add r1,r6,#REG_DMA3SAD

	ldr r2,[vdpptr,#vdpDMAOAMBuffer]	;@ DMA3 src, OAM transfer:
	mov r3,#OAM					;@ DMA3 dst
	mov r4,#0x84000000			;@ noIRQ 32bit incsrc incdst
	orr r4,r4,#0x100			;@ 128 sprites (1024 bytes)
	stmia r1,{r2-r4}			;@ DMA3 go

	ldr r2,=EMUPALBUFF			;@ DMA3 src, Palette transfer:
	mov r3,#BG_PALETTE			;@ DMA3 dst
	mov r4,#0x84000000			;@ noIRQ 32bit incsrc incdst
	orr r4,r4,#0x100			;@ 256 words (1024 bytes)
	stmia r1,{r2-r4}			;@ DMA3 go

	ldr r2,[vdpptr,#vdpBgrMapOfs1]
	add r0,r2,#0x0005
	strh r0,[r6,#REG_BG0CNT]
	ldr r3,=0x02870102
	add r0,r2,r3
	ldr r1,[vdpptr,#vdpBgrTileOfs]
	add r0,r0,r1,lsr#12
	strh r0,[r6,#REG_BG1CNT]

	ldr r11,=VDP1
	ldr r2,[r11,#vdpBgrMapOfs1]
	ldr r1,[r11,#vdpBgrTileOfs]
	add r0,r2,r3
	add r0,r0,r1,lsr#12
	strh r0,[r6,#REG_BG3CNT]
	add r0,r2,r3,lsr#16
	strh r0,[r6,#REG_BG2CNT]

	mov r0,#0x001F
	ldrb r1,g_gfxMask
	bic r0,r0,r1
	ldrb r1,[vdpptr,#vdpMode2Bak2]
	tst r1,#0x40
	biceq r0,r0,#0x0013			;@ Turn off VDP0 sprites and bg
	ldrb r1,[r11,#vdpMode2Bak2]
	tst r1,#0x40
	biceq r0,r0,#0x000C			;@ Turn off VDP1 bg
	orr r0,r0,r0,lsl#8
//	bic r0,r0,#0x0008

	ldrb r1,[vdpptr,#vdpMode1]
//	tst r1,#0x80				;@ Columns 24-31 locked?
//	biceq r0,r0,#0x0800
//	bicne r0,r0,#0x0700

	strh r0,[r6,#REG_WININ]

	tst r1,#0x20				;@ Column 0 blanked?
	ldreq r0,Window0HValue_normal
	ldrne r0,Window0HValue_col0
	strh r0,[r6,#REG_WIN0H]
	ldr r0,Window1HValue
	strh r0,[r6,#REG_WIN1H]
	ldr r0,WindowVValue
	strh r0,[r6,#REG_WIN0V]
	strh r0,[r6,#REG_WIN1V]

	blx scanKeys
	ldmfd sp!,{r4-r11,pc}

;@------------------------------------------------------------------------------
earlyFrame:					;@ Called at line 0,16 or 32	(r0,r2 safe to use)
;@------------------------------------------------------------------------------
	stmfd sp!,{r1,r3-r12,lr}

	ldr r0,=SPRS
	ldrb r0,[r0]
	cmp r0,#0
	ldreq r0,=defaultScanlineHook
	ldrne r0,=spriteScanner
	str r0,[vdpptr,#vdpScanlineHook]
	adrne lr,earlyFrameEnd
	bne spriteScannerStart


	ldrb r0,[vdpptr,#vdpRealMode]
	cmp r0,#VDPMODE_4
	bleq sprDMADo0
earlyFrameEnd:
	ldmfd sp!,{r1,r3-r12,pc}

;@----------------------------------------------------------------------------
endFrame:					;@ Called just before screen end (~line 192)	(r0 & r2 safe to use)
;@----------------------------------------------------------------------------
	stmfd sp!,{r1,r3-r11,lr}

//	mov r11,vdpptr
	ldrb r0,[vdpptr,#vdpNameTable]
	mov r2,#224
	bl ntFinnish
	ldrb r1,[vdpptr,#vdpXScroll]
	bl VDPReg08W
	bl bgFinish
;@--------------------------
	bl applyScaling
	adr lr,sDMARet
	ldrb r0,[vdpptr,#vdpRealMode]
	cmp r0,#VDPMODE_4
	bmi sprDMADoM2
	beq sprDMADo1
sDMARet:
;@--------------------------
	ldr r0,=g_machine
	ldrb r0,[r0]
	cmp r0,#HW_GG
	bleq paletteTxGGSG
;@--------------------------

	ldr r2,=EMUPALBUFF
	ldr r1,=bColor
	ldrb r0,[r1]
	cmp r0,#0
	ldreq r1,=g_machine
	ldrbeq r1,[r1]
	cmpeq r1,#HW_GG
	beq setBc
	ldrb r0,[vdpptr,#vdpRealMode]
	add r3,r2,#0x80				;@ SG palette
	cmp r0,#0x04				;@ Mode4?
	addpl r3,r3,#0x180			;@ Use normal palette
	ldrb r4,[vdpptr,#vdpBDColor]
	and r1,r4,#0xF				;@ Border color
	add r1,r3,r1,lsl#1
	ldrh r0,[r1]

	mov r1,r4,lsr#4				;@ Text color for TMS9918 modes.
	add r1,r3,r1,lsl#1
	ldrh r1,[r1]
	strhmi r1,[r2,#64+2]
setBc:
	strh r0,[r2]				;@ Color 0


	ldrb r0,[vdpptr,#vdpMode2Bak1]
	strb r0,[vdpptr,#vdpMode2Bak2]

	ldr r0,[vdpptr,#vdpDMAOAMBuffer]
	ldr r1,[vdpptr,#vdpTmpOAMBuffer]
	str r0,[vdpptr,#vdpTmpOAMBuffer]
	str r1,[vdpptr,#vdpDMAOAMBuffer]

	ldr r0,[vdpptr,#vdpBgrMapOfs0]
	ldr r1,[vdpptr,#vdpBgrMapOfs1]
	str r0,[vdpptr,#vdpBgrMapOfs1]
	str r1,[vdpptr,#vdpBgrMapOfs0]

	ldr r0,=windowTop			;@ Load wtop, store in wtop+4.......load wtop+8, store in wtop+12
	ldmia r0,{r1-r3}			;@ Load with post increment
	stmib r0,{r1-r3}			;@ Store with pre increment


//	bl transferVRAM
	bl delayVRAM
	ldr r0,frameTotal
	add r0,r0,#1
	str r0,frameTotal
	ldmfd sp!,{r1,r3-r11,lr}
	bx lr

	.pool
;@----------------------------------------------------------------------------
VDP0SetMode:
	.type VDP0SetMode STT_FUNC
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPSetMode
;@----------------------------------------------------------------------------
SysESprFix:
;@----------------------------------------------------------------------------
	stmfd sp!,{r0-r3,r12}
	ldr vdpptr,=VDP0
	ldr r2,[vdpptr,#vdpDMAOAMBuffer]	;@ DMA3 src, OAM transfer:
	add r2,r2,#0x200
	ldr vdpptr,=VDP1
	ldr r3,[vdpptr,#vdpDMAOAMBuffer]	;@ DMA3 src, OAM transfer:
	mov r1,#64
eSprLoop:
	ldr r0,[r3],#4
	str r0,[r2],#4
	ldrh r0,[r3],#4
	orr r0,r0,#0x2200					;@ Palette 2 & tiles +0x200.
	strh r0,[r2],#4
	subs r1,r1,#1
	bne eSprLoop

	ldmfd sp!,{r0-r3,r12}
	bx lr
;@----------------------------------------------------------------------------
spriteScannerStart:
	stmfd sp!,{r3-r10}
;@	mov r11,r11

	ldr r9,[vdpptr,#VRAMPtr]
	ldrb r0,[vdpptr,#vdpSATOffset]
	and r0,r0,#0x7E
	add r9,r9,r0,lsl#7
	add r8,r9,#0x80

	ldr r2,[vdpptr,#vdpSprStop]
	cmp r2,#0xD0
	moveq r2,#0xFFFFFFD0

	ldr r4,=SMSOAMBuff
	add r3,r4,#0x80
	ldrb r5,[vdpptr,#vdpScrStartLine]
	sub r1,r5,#0xF

	ldrb r10,[vdpptr,#vdpSPROffset]	;@ First or second half of VRAM for sprites?
	and r10,r10,#4

	mov r6,#0
	mov r7,#0
ss1Loop:
	ldrsb r0,[r9,r7]			;@ MasterSystem OBJ, r0=Ypos.
	cmp r0,r2
	beq ss1End
	cmp r0,r5
	bgt ss1Chk
	cmp r0,r1
	bpl ss1Add
ss1Chk:
	add r7,r7,#1
	cmp r7,#0x40
	bne ss1Loop
ss1End:
	str r6,smsOamPtr
	strb r2,[r4,r6]

	ldmfd sp!,{r3-r10}
	bx lr

ss1Add:
	strb r0,[r4,r6]
	mov r0,r7,lsl#1
	ldrh r0,[r8,r0]				;@ MasterSystem OBJ, r4=Tile,Xpos.
	orr r0,r0,r10,lsl#14
	str r0,[r3,r6,lsl#2]
	cmp r6,#0x7F
	addmi r6,r6,#1
	bmi ss1Chk
	b ss1End

;@----------------------------------------------------------------------------
spriteScanner:
	stmfd sp!,{r4-r10}

	ldr r9,[vdpptr,#VRAMPtr]
	ldrb r0,[vdpptr,#vdpSATOffset]
	and r0,r0,#0x7E
	add r9,r9,r0,lsl#7
	add r8,r9,#0x100

	ldrb r10,[vdpptr,#vdpSPROffset]	;@ First or second half of VRAM for sprites?
	and r10,r10,#4

	ldr r2,[vdpptr,#vdpSprStop]
	ldr r4,=SMSOAMBuff
	add r5,r4,#0x80
;@	ldr r1,[vdpptr,#vdpScanline]	;@ r1 is allready scanline.
	ldr r6,smsOamPtr
	mov r7,#-0x80
ss0Loop:
	ldrb r0,[r9],#1					;@ MasterSystem OBJ, r0=Ypos.
	cmp r0,r2
	cmpne r0,r1
	beq ss0Add
ss0Chk:
	adds r7,r7,#2
	bne ss0Loop
ss0End:
	str r6,smsOamPtr
	strb r2,[r4,r6]
	ldmfd sp!,{r4-r10}
	b defaultScanlineHook

ss0Add:
	cmp r0,r2
	beq ss0End

	strb r0,[r4,r6]
	ldrh r0,[r8,r7]					;@ MasterSystem OBJ, r4=Tile,Xpos.
	orr r0,r0,r10,lsl#14
	str r0,[r5,r6,lsl#2]
	cmp r6,#0x7F
	addmi r6,r6,#1
	bmi ss0Chk
	b ss0End

;@----------------------------------------------------------------------------
;@sprDMADo:					;@ Called from endFrame. YATX
;@----------------------------------------------------------------------------
#define PRIORITY	(0x800)		// 0x800=AGB OBJ priority 2

sprDMADo0:						;@ Called from earlyFrame if no spr scanning.
	ldr r2,[vdpptr,#vdpTmpOAMBuffer]	;@ Destination
	b sprPassDoM4

sprDMADo1:						;@ Called from endFrame.
	ldr r2,[vdpptr,#vdpTmpOAMBuffer]	;@ Destination
	ldrb r0,SPRS
	cmp r0,#0
	bne pSprDo
	add r2,r2,#0x200


sprPassDoM4:
	mov r8,#64					;@ Number of sprites
	ldr r10,[vdpptr,#VRAMPtr]
	ldrb r0,[vdpptr,#vdpSATOffset]
	and r0,r0,#0x7E
	add r10,r10,r0,lsl#7
	mov r6,#0x100				;@ r6= scale obj

	ldr r11,[vdpptr,#vdpSprStop]
	ldrb r0,[vdpptr,#vdpMode2]
	movs r0,r0,lsl#31			;@ Double pixels/8x16 size
	orrmi r6,r6,#0x00000200		;@ Doublesize
	orrmi r6,r6,#0x04000000		;@ Scaling param
	orrcs r6,r6,#0x00008000		;@ 8x16 shape
	orrcs r6,r6,#0x02000000		;@ Scaling param

	ldr r5,bg_scaleValue
	add r5,r5,#1
dm4_1:
	add r9,r10,#0x80
	mov r7,#PRIORITY
	ldrb r0,[vdpptr,#vdpSPROffset]	;@ First or second half of VRAM for sprites?
	tst r0,#4
	orrne r7,r7,#0x100
	ldrb r1,[vdpptr,#vdpMode1]
	and r1,r1,#8
	mov r1,r1,lsl#23
dm4_2:
	ldrb r0,[r10],#1			;@ MasterSystem OBJ, r0=Ypos.
	ldrh r4,[r9],#2				;@ MasterSystem OBJ, r4=Tile,Xpos.
	cmp r0,r11
	beq dm4_3					;@ Skip the rest if sprite Y=208

	cmp r0,#0xEF
	subpl r0,r0,#0x100
	add r0,r0,#1
	ldr r3,=yStart
	ldrsb r3,[r3]
	sub r0,r0,r3

	mov r3,#4
	tst r6,#0x02000000
	movne r3,r3,lsl#1
	tst r6,#0x04000000
	movne r3,r3,lsl#1
	add r0,r0,r3

	mul r0,r5,r0
	sub r0,r0,r3,lsl#16
	and r0,r0,#0xFF0000
	orr r0,r6,r0,lsr#16			;@ Size plus scaling?
	and r3,r4,#0xFF
	rsb r3,r1,r3,lsl#23
	orr r0,r0,r3,lsr#7
	str r0,[r2],#4				;@ Store OBJ Atr 0,1. Xpos, ypos, flip, scale/rot, size, shape.

	bic r4,r4,r6,lsr#7			;@ Only even tiles in 8x16 mode
	orr r0,r7,r4,lsr#8			;@ Priority & tile offset
	strh r0,[r2],#4				;@ Store OBJ Atr 2. Pattern, palette.
	subs r8,r8,#1
	bne dm4_2
	bx lr

dm4_3:
	mov r0,#0x2C0				;@ Double, y=192
dm4_4:
	str r0,[r2],#8
	subs r8,r8,#1
	bne dm4_4
	bx lr



;@----------------------------------------------------------------------------
pSprDo:
	ldr r8,smsOamPtr
	cmp r8,#0
	beq pSpr3
	ldr r10,=SMSOAMBuff

	mov r6,#0x100				;@ r6= scale obj

	ldr r11,[vdpptr,#vdpSprStop]
	ldrb r0,[vdpptr,#vdpMode2]
	movs r0,r0,lsl#31			;@ Double pixels/8x16 size
	orrmi r6,r6,#0x00000200		;@ Doublesize
	orrmi r6,r6,#0x04000000		;@ Scaling param
	orrcs r6,r6,#0x00008000		;@ 8x16 shape
	orrcs r6,r6,#0x02000000		;@ Scaling param

	ldr r5,bg_scaleValue
	add r5,r5,#1
pSpr1:
	add r9,r10,#0x80
	mov r7,#PRIORITY
	ldrb r1,[vdpptr,#vdpMode1]
	and r1,r1,#8
	mov r1,r1,lsl#23
pSpr2:
	ldrb r0,[r10],#1			;@ MasterSystem OBJ, r0=Ypos.
	ldr r4,[r9],#4				;@ MasterSystem OBJ, r4=Tile,Xpos.
	cmp r0,r11
	beq pSpr3					;@ Skip the rest if sprite Y=208

	cmp r0,#0xEF
	subpl r0,r0,#0x100
	add r0,r0,#1
	ldr r3,=yStart
	ldrsb r3,[r3]
	sub r0,r0,r3

	mov r3,#4
	tst r6,#0x02000000
	movne r3,r3,lsl#1
	tst r6,#0x04000000
	movne r3,r3,lsl#1
	add r0,r0,r3

	mul r0,r5,r0
	sub r0,r0,r3,lsl#16
	and r0,r0,#0xFF0000
	orr r0,r6,r0,lsr#16			;@ Size plus scaling?
	and r3,r4,#0xFF
	rsb r3,r1,r3,lsl#23
	orr r0,r0,r3,lsr#7
	str r0,[r2],#4				;@ Store OBJ Atr 0,1. Xpos, ypos, flip, scale/rot, size, shape.

	bic r4,r4,r6,lsr#7			;@ Only even tiles in 8x16 mode
	orr r0,r7,r4,lsr#8			;@ Priority & tile offset
	strh r0,[r2],#4				;@ Store OBJ Atr 2. Pattern, palette.
	subs r8,r8,#1
	bne pSpr2
//	bx lr

pSpr3:
	ldr r0,smsOamPtr
	sub r8,r0,r8
	rsb r8,r8,#0x80
	mov r0,#0x2C0				;@ Double, y=192
pSpr4:
	subs r8,r8,#1
	strpl r0,[r2],#8
	bhi pSpr4
	bx lr



;@----------------------------------------------------------------------------
sprDMADoM2:					;@ Called from endFrame.
;@----------------------------------------------------------------------------
	ldr r2,[vdpptr,#vdpTmpOAMBuffer]	;@ Destination

	ldrb r1,[vdpptr,#vdpSATOffset]
	ldr r10,[vdpptr,#VRAMPtr]
	and r1,r1,#0x7F
	add r10,r10,r1,lsl#7

	mov r6,#0x100				;@ r6= scale obj

	ldrb r1,[vdpptr,#vdpMode2]
	movs r1,r1,lsl#31			;@ Double pixels/16x16 size
	orrmi r6,r6,#0x00000200		;@ Doublesize
	orrmi r6,r6,#0x04000000		;@ Scaling param
	orrcs r6,r6,#0x00008000		;@ 8x16 shape
	orrcs r6,r6,#0x02000000		;@ Scaling param

	ldr r5,bg_scaleValue
	add r5,r5,#1

	tst r6,r6,lsl#11			;@ 16x16 size + scaling?
	mov r8,#32					;@ Number of sprites
	cmp r0,#VDPMODE_1
	beq dm2_3					;@ No sprites in Mode1
	mov r7,#PRIORITY+0x300
	mov r1,#0x10000000
dm2_2:
	ldr r4,[r10],#4				;@ MasterSystem OBJ, r0=Ypos.
	mov r0,r4,lsl#24
	cmp r0,#0xD0000000
	beq dm2_3					;@ Skip the rest if sprite Y=208
	and r9,r4,#0xFF00
	and r3,r1,r4,lsr#3			;@ EC early clock, x -=32.
	rsb r9,r3,r9,lsl#15

	mov r0,r0,lsr#24
	cmp r0,#0xEF
	subpl r0,r0,#0x100
	add r0,r0,#1
	ldr r3,=yStart
	ldrsb r3,[r3]
	sub r0,r0,r3

	mov r3,#4					;@ Sprites are scaled around the center
	tst r6,#0x02000000			;@ That's why this is needed
	movne r3,r3,lsl#1
	tst r6,#0x04000000
	movne r3,r3,lsl#1
	add r0,r0,r3

	mul r0,r5,r0
	sub r0,r0,r3,lsl#16
	and r0,r0,#0xFF0000
	orr r0,r6,r0,lsr#16			;@ Size plus scaling?
	tst r4,#0xF000000			;@ Color 0 sprite = invisible.
	moveq r0,#0x2C0				;@ Double, y=192
	orr r3,r0,r9,lsr#7
	str r3,[r2],#4				;@ Store OBJ Atr 0,1. Xpos, ypos, flip, scale/rot, size, shape.

	mov r4,r4,ror#24
	orr r3,r7,r4,lsr#24			;@ Tiles + tileoffset + priority
	orr r3,r3,r4,lsl#12			;@ Palette
	tst r6,#0x00008000			;@ 16x16 size?
	bicne r3,r3,#3				;@ Only even tiles in 16x16 mode
	strh r3,[r2],#4				;@ Store OBJ Atr 2. Pattern, palette.

	moveq r0,#0x2C0				;@ Double, y=192
	addne r3,r3,#2				;@ Tile+2
	addne r9,r9,#0x04000000
	tstne r6,#0x00000200		;@ Zoom?
	addne r9,r9,#0x04000000
	orr r0,r0,r9,lsr#7
	str r0,[r2],#4				;@ Store OBJ Atr 0,1. Xpos, ypos, flip, scale/rot, size, shape.
	strh r3,[r2],#4				;@ Store OBJ Atr 2. Pattern, palette.

	subs r8,r8,#1
	bne dm2_2
;@	bx lr

dm2_3:
	add r8,r8,#32
	mov r0,#0x2C0				;@ Double, y=192
dm2_4:
	str r0,[r2],#8
	str r0,[r2],#8
	subs r8,r8,#1
	bne dm2_4
	bx lr

;@----------------------------------------------------------------------------
bgFinish:					;@ End of frame...
;@----------------------------------------------------------------------------
	stmfd sp!,{r3-r11,lr}


;@	b bgCont

;@	ldrb r0,frame
;@	tst r0,#0x7
;@	ldmnefd sp!,{r3-r11,pc}
bgCont:
;@	mov r0,#0
;@	strb r0,[r4]

	mov r10,#BG_GFX
	ldr r0,[vdpptr,#vdpBgrMapOfs0]
	add r10,r10,r0,lsl#3
	ldr r5,=0xF000F000
	ldr r6,=0x000003FF
	ldr r7,=0x00010001
	ldrb lr,[vdpptr,#vdpYScrollBak1]
	and r0,lr,#7
	mov lr,lr,lsr#3
	rsbs r1,r0,#4
	movmi r1,#0
	add r8,vdpptr,#TMapBuff
	ldrb r1,[r8,r1]!
	ldrb r2,[vdpptr,#vdpHeightMode]
	cmp r2,#VDPMODE_4_224
	cmpne r2,#VDPMODE_4_240
	ldr r11,[vdpptr,#VRAMPtr]
	addeq r11,r11,#0x700
	mov r9,#28
	moveq r9,#32

	and r2,r2,#0x0F
	cmp r2,#VDPMODE_4
	beq bgMode4
	cmp r2,#VDPMODE_5
	beq bgMode5

	ldr r7,=0x40004000			;@ Palette 4 & tileoffset
	ldrb r9,[vdpptr,#vdpPGOffsetBak1]
	and r9,r9,#3
	eor r9,r9,#3
	orr r9,r9,r9,lsl#16
	mov r9,r9,lsl#8

	mov r8,#0
	cmp r2,#VDPMODE_2
	ldreq r8,=0x01000100
	and r1,r1,#0xF
	add r3,r11,r1,lsl#10


	mov r4,r10
	mov r0,#0

	mov r6,#3
	cmp r2,#VDPMODE_0
	cmpne r2,#VDPMODE_2
	beq bgMode02
	mov r6,#24
	cmp r2,#VDPMODE_3
	beq bgMode3
	cmp r2,#VDPMODE_1
	bne bgModeB
;@----------------------------------------------------------------------------
bgMode1:
	ldr r7,=0x20002000			;@ Palette 2
bgM1Loop:
	mov r5,#16
bgM1Row:
	ldrh r1,[r3],#2				;@ Read from MasterSystem Tilemap RAM
	orr r1,r1,r1,lsl#8
	bic r1,r1,#0xFF00
	orr r1,r1,r7				;@ Palette & tileoffset

	str r0,[r4,r7,lsr#17]		;@ Write to NDS Tilemap RAM, BGR color
	str r1,[r4,#0x800]			;@ Write to NDS Tilemap RAM, behind sprites
	str r0,[r4],#4				;@ Write to NDS Tilemap RAM, in front of sprites
	subs r5,r5,#1
	bne bgM1Row
	add r3,r3,#8
	subs r6,r6,#1
	bne bgM1Loop
bgModeB:
	ldmfd sp!,{r3-r11,pc}

;@----------------------------------------------------------------------------
;@bgChrFinish				;@ End of frame...
;@----------------------------------------------------------------------------
;@	mov r0,#0
;@	ldr r7,=0x40004000
;@	ldr r9,=0x00020002
bgMode02:						;@ Fake

bgM2Loop2:
	bic r11,r7,r9
	mov r5,#16*8
bgM2Loop:
	ldrh r1,[r3],#2				;@ Read from MasterSystem Tilemap RAM
	orr r1,r1,r1,lsl#8
	bic r1,r1,#0xFF00
	orr r1,r1,r11				;@ Palette & tile offset.

	str r0,[r4,r8,lsr#12]		;@ Write to NDS Tilemap RAM, BGR color
	str r1,[r4,#0x800]			;@ Write to NDS Tilemap RAM, behind sprites
	str r0,[r4],#4				;@ Write to NDS Tilemap RAM, in front of sprites
	subs r5,r5,#1
	bne bgM2Loop
	add r7,r7,r8				;@ Add tileoffset for group.
	subs r6,r6,#1
	bne bgM2Loop2

	ldmfd sp!,{r3-r11,pc}

;@----------------------------------------------------------------------------
;@bgChrFinish				;@ End of frame...
;@----------------------------------------------------------------------------
;@	mov r0,#0
;@	ldr r7,=0x40004000
;@	ldr r9,=0x00040004
bgMode3:
	ldr r7,=0x40004000
	ldr r8,=0x00010001
	ldr r9,=0x00040004
bgM3Loop2:
	bic r7,r7,r9
	mov r5,#16
bgM3Loop:
	ldrh r1,[r3],#2				;@ Read from MasterSystem Tilemap RAM
	orr r1,r1,r1,lsl#8
	bic r1,r1,#0xFF00
	orr r1,r7,r1,lsl#2			;@ Palette & tile offset.

	str r0,[r4,r8,lsr#12]		;@ Write to NDS Tilemap RAM, BGR color
	str r1,[r4,#0x800]			;@ Write to NDS Tilemap RAM, behind sprites
	str r0,[r4],#4				;@ Write to NDS Tilemap RAM, in front of sprites
	subs r5,r5,#1
	bne bgM3Loop
	add r7,r7,r8				;@ Add tileoffset for group.
	subs r6,r6,#1
	bne bgM3Loop2

	ldmfd sp!,{r3-r11,pc}


;@----------------------------------------------------------------------------
;@bgChrFinish				;@ End of frame...
;@----------------------------------------------------------------------------
;@	ldr r5,=0xF000F000
;@	ldr r6,=0x000003FF
;@	ldr r7,=0x00010001
;@ MSB          LSB
;@ ---pcvhnnnnnnnnn
bgMode4:
	ldr r2,[vdpptr,#vdpBgrTileOfs]
	and r2,r2,#0x4000
	orr r2,r2,r2,lsl#16
bgM4Frame:
	ldrb r1,[r8],#8
	ldrb r0,[vdpptr,#vdpNTMask]
	movs r0,r0,lsr#1
	orrcs r1,r1,#0x01
	ands r0,r0,r1,lsr#1
	orr r0,lr,r0,lsl#5
	biccc r0,r0,#0x10
	add r3,r11,r0,lsl#6
	add r4,r10,lr,lsl#6

	add lr,lr,#1
	ldr r0,[vdpptr,#vdpScrollMask]
	cmp lr,r0,lsr#3
	subpl lr,lr,r0,lsr#3

bgM4Row:
	ldr r0,[r3],#4				;@ Read from MasterSystem Tilemap RAM

	and r1,r7,r0,lsr#11
	orr r1,r1,r7,lsl#1			;@ Bgr color 0x30 & 0x40
	str r1,[r4,r7,lsr#4]		;@ Write to GBA Tilemap RAM, BGR color

	tst r7,r0,lsl#4				;@ Shift out top P bit, test low P bit.
	bic r0,r0,r5
	and r1,r0,r5,lsr#3
	add r0,r0,r1				;@ XY flip + color.

	orr r0,r0,r2,lsr#1
	str r0,[r4,#0x800]			;@ Write to GBA Tilemap RAM, behind sprites
	add r0,r0,r7,lsl#9			;@ New tile offset
	biccc r0,r0,r6,lsl#16
	biceq r0,r0,r6
	str r0,[r4],#4				;@ Write to GBA Tilemap RAM, in front of sprites
	tst r4,#0x3C				;@ 32 tiles wide
	bne bgM4Row
	subs r9,r9,#1
	bne bgM4Frame

	ldmfd sp!,{r3-r11,pc}

;@----------------------------------------------------------------------------
bgMode5:
	ldr r2,=0x01C001C0
	ldr r5,=0xFC00FC00
;@	ldr r6,=0x000003FF
;@	ldr r7,=0x00010001
;@	r10 = NDS destination address
;@	lr = y scroll row
;@	r11 = VDPRAM
;@ MSB          LSB
;@ pccvhnnnnnnnnnnn
bgM5Loop:
	ldrb r0,[r8],#8
	and r0,r0,#0x08				;@ 0x2000, should be 0x1C
	orr r0,lr,r0,lsl#4
	add r3,r11,r0,lsl#6
	add r4,r10,lr,lsl#6

	add lr,lr,#1
;@	ldr r0,[vdpptr,#vdpScrollMask]
	mov r0,#256
	cmp lr,r0,lsr#3
	subpl lr,lr,r0,lsr#3

bgM5Row:
	ldr r0,[r3],#4				;@ Read from MegaDrive Tilemap RAM

	and r1,r7,r0,lsr#11
	str r1,[r4,r7,lsr#4]		;@ Write to NDS Tilemap RAM, BGR color

	bic r1,r0,r5
	and r0,r5,r0,lsr#1
	add r0,r0,r2				;@ Tile offset.
	orr r0,r0,r7,lsl#15			;@ MD palette
	orr r1,r1,r0				;@ XY flip + color.

	str r1,[r4,#0x800]			;@ Write to NDS Tilemap RAM, behind sprites
	mov r1,#0
	str r1,[r4],#4				;@ Write to NDS Tilemap RAM, in front of sprites
	tst r4,#0x3C				;@ 32 tiles wide
	bne bgM5Row
	subs r9,r9,#1
	bne bgM5Loop

	ldmfd sp!,{r3-r11,pc}
;@----------------------------------------------------------------------------
SystemESetVRAM:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP1
	ldr r2,=VDPRAM+0x8000
	tst r0,#0x80
	addne r2,r2,#0x4000
	str r2,[vdpptr,#VRAMPtr]
	ldr vdpptr,=VDP0
	ldr r2,=VDPRAM
	tst r0,#0x40
	addne r2,r2,#0x4000
	str r2,[vdpptr,#VRAMPtr]
	stmfd sp!,{r1,lr}
	tst r1,#0x40
	blne VDPClearDirtyTiles
	ldmfd sp!,{r1,lr}
	ldr vdpptr,=VDP1
	tst r1,#0x80
	bne VDPClearDirtyTiles
	bx lr
;@----------------------------------------------------------------------------
SystemEVDPRAMWrite:
;@----------------------------------------------------------------------------
	mov r1,r12,lsr#14
	cmp r1,#0x02
	beq debug000
	mov r11,r11
	bx lr
debug000:
	ldr r1,=sysELatch
	ldrb r1,[r1]
	movs r1,r1,lsr#6
	andcc r1,r1,#1
	movcs r1,r1,lsr#1
	orrcc r1,r1,#2
	ldr r2,=VDPRAM-0x8000+0xC000
	sub r2,r2,r1,lsl#14
	strb r0,[r2,r12]
	bx lr
	
;@----------------------------------------------------------------------------
VDP0ScanlineBPReset:
	.type VDP0ScanlineBPReset STT_FUNC
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPScanlineBPReset
;@----------------------------------------------------------------------------
VDP0LatchHCounter:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPLatchHCounter

;@----------------------------------------------------------------------------
VDP0VCounterR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPVCounterR
;@----------------------------------------------------------------------------
VDP0HCounterR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPHCounterR
;@----------------------------------------------------------------------------
VDP0StatR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPStatR
;@----------------------------------------------------------------------------
VDP0DataR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPDataR
;@----------------------------------------------------------------------------
VDP0DataTMSW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPDataTMSW
;@----------------------------------------------------------------------------
VDP0DataSMSW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPDataSMSW
;@----------------------------------------------------------------------------
VDP0DataGGW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPDataGGW
;@----------------------------------------------------------------------------
VDP0DataMDW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPDataMDW
;@----------------------------------------------------------------------------
VDP0CtrlW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPCtrlW
;@----------------------------------------------------------------------------
VDP0CtrlMDW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP0
	b VDPCtrlMDW

;@----------------------------------------------------------------------------
VDP1VCounterR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP1
	b VDPVCounterR
;@----------------------------------------------------------------------------
VDP1HCounterR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP1
	b VDPHCounterR
;@----------------------------------------------------------------------------
VDP1StatR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP1
	b VDPStatR
;@----------------------------------------------------------------------------
VDP1DataR:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP1
	b VDPDataR
;@----------------------------------------------------------------------------
VDP1DataSMSW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP1
	b VDPDataSMSW
;@----------------------------------------------------------------------------
VDP1CtrlW:
;@----------------------------------------------------------------------------
	ldr vdpptr,=VDP1
	b VDPCtrlW

;@----------------------------------------------------------------------------

;@----------------------------------------------------------------------------

smsOamPtr:			.long 0
windowTop:			.long 0
wTop:
	.long 0,0,0		;@ windowtop  (this label too)   L/R scrolling in unscaled mode
frameTotal:			;@ let ui.c see frame count for savestates
	.long 0
paletteMask:		.long 0x7FFF

gfxState:
yStart:				.byte 0
SPRS:				.byte 1		;@ SpriteScanning On/Off
				.byte 0
				.byte 0
	.pool


	.section .bss
	.align 2
VDP0:
	.space vdpSize
VDP1:
	.space vdpSize
VDPRAM:
	.space 0x4000
	.space 0x4000
	.space 0x4000
	.space 0x4000
	.size VDPRAM, 0x4000
VDPRAMCopy0:
	.space 0x4000
VDPRAMCopy1:
	.space 0x4000

CHRDecode:
	.space 0x400
DirtyTilesBackup0:
	.space 0x200
DirtyTilesBackup1:
	.space 0x200
SMSOAMBuff:
	.space 0x280
OAMBuffer1:
	.space 0x400
OAMBuffer2:
	.space 0x400
OAMBuffer3:
	.space 0x400
OAMBuffer4:
	.space 0x400
DMA0Buff:
	.space 0x1100				;@ Actually 0x1000 plus extra
mappedRGB:
	.space 0x2000
EMUPALBUFF:
	.space 0x400
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
