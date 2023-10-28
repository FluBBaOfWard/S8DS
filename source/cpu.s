#ifdef __arm__

#include "Shared/nds_asm.h"
#include "Equates.h"
#include "ARMZ80/ARMZ80.i"
#include "SegaVDP/SegaVDP.i"

#define CYCLE_PSL (228)

	.global waitMaskIn
	.global waitMaskOut

	.global run
	.global runFrame
	.global cpuReset

	.syntax unified
	.arm

#ifdef GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
run:		;@ Return after X frame(s)
	.type   run STT_FUNC
;@----------------------------------------------------------------------------

	ldrh r0,waitCountIn
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountIn
	bxne lr
	stmfd sp!,{r4-r11,lr}

;@----------------------------------------------------------------------------
runStart:
;@----------------------------------------------------------------------------
	ldr r0,=EMUinput
	ldr r0,[r0]

	ldr r2,=yStart
	ldrb r1,[r2]
	tst r0,#0x200				;@ L?
	subsne r1,#1
	movmi r1,#0
	tst r0,#0x100				;@ R?
	addne r1,#1
	cmp r1,#224-SCREEN_HEIGHT
	movpl r1,#224-SCREEN_HEIGHT
@	strb r1,[r2]

	ldr z80ptr,=Z80OpTable
	bl refreshEMUjoypads		;@ Z=1 if communication ok

	add r0,z80ptr,#z80Regs
	ldmia r0,{z80f-z80pc,z80sp}	;@ Restore Z80 state

	mov lr,pc
	ldr pc,selectedFrameRun
;@----------------------------------------------------------------------------
FrameLoopEnd:
	add r0,z80ptr,#z80Regs
	stmia r0,{z80f-z80pc,z80sp}	;@ Save Z80 state

	ldr r1,=fpsValue
	ldr r0,[r1]
	add r0,r0,#1
	str r0,[r1]

	ldr r1,=gConfigSet
	ldrb r1,[r1]
	ldr r2,=EMUinput
	ldr r2,[r2]
	and r1,r1,r2,lsr#4			;@ R button and config FF
	ands r1,r1,#0x10

	ldrh r0,waitCountOut
	orrne r0,r0,#0x0300
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountOut
	ldmfdeq sp!,{r4-r11,lr}		;@ Exit here if doing single frame:
	bxeq lr						;@ Return to rommenu()
	b runStart

;@----------------------------------------------------------------------------
selectedFrameRun:	.long SMSFrameRun
scanlineCycles:		.long CYCLE_PSL
waitCountIn:		.byte 0
waitMaskIn:			.byte 0
waitCountOut:		.byte 0
waitMaskOut:		.byte 0

;@----------------------------------------
SM5FrameRun:
	stmfd sp!,{lr}
SM5FrameLoop:
	ldr r0,scanlineCycles
	bl CTC0Update
	ldr vdpptr,=VDP0
	bl VDPDoScanline
	cmp r0,#0
	ldmfdne sp!,{pc}
	ldr r0,scanlineCycles
	bl Z80RunXCycles
	b SM5FrameLoop
;@----------------------------------------
SYSEFrameRun:
	stmfd sp!,{lr}
SYSEFrameLoop:
	ldr vdpptr,=VDP1
	bl VDPDoScanline
	ldr vdpptr,=VDP0
	bl VDPDoScanline
	cmp r0,#0
	ldmfdne sp!,{pc}
	ldr r0,scanlineCycles
	bl Z80RunXCycles
	b SYSEFrameLoop
;@----------------------------------------
SMSFrameRun:
	stmfd sp!,{lr}
SMSFrameLoop:
	ldr vdpptr,=VDP0
	bl VDPDoScanline
	cmp r0,#0
	ldmfdne sp!,{pc}
	ldr r0,scanlineCycles
	bl Z80RunXCycles
	b SMSFrameLoop

;@----------------------------------------------------------------------------
runFrame:					;@ Return after 1 frame
	.type runFrame STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr z80ptr,=Z80OpTable
	add r0,z80ptr,#z80Regs
	ldmia r0,{z80f-z80pc,z80sp}	;@ Restore Z80 state
;@----------------------------------------------------------------------------
s8StepLoop:
;@----------------------------------------------------------------------------
	mov lr,pc
	ldr pc,selectedFrameRun
;@----------------------------------------------------------------------------
	add r0,z80ptr,#z80Regs
	stmia r0,{z80f-z80pc,z80sp}	;@ Save Z80 state
	ldmfd sp!,{r4-r11,lr}
	bx lr
;@----------------------------------------------------------------------------
;@ntsc_pal_reset:
;@--- PAL Speed - 3546893Hz / 49.7Hz / 313 = 228
;@---NTSC Speed - 3579545Hz / 59.9Hz / 262 = 228
;@----------------------------------------------------------------------------
cpuReset:		;@ Called by loadCart/resetGame
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}

	ldr r4,=gMachine
	ldrb r4,[r4]
	cmp r4,#HW_SYSE
	mov r0,#CYCLE_PSL
	addeq r0,r0,r0,lsr#1
	str r0,scanlineCycles


	adr r0,SMSFrameRun
	cmpne r4,#HW_MEGATECH
	adreq r0,SYSEFrameRun
	cmp r4,#HW_SORDM5
	adreq r0,SM5FrameRun
	str r0,selectedFrameRun

	ldr r0,=Z80OpTable
	mov r1,#0					;@ SMS
	cmp r4,#HW_MEGADRIVE
//	cmpne r4,#HW_MEGATECH
	moveq r1,#1					;@ MD
	cmp r4,#HW_GG
	moveq r1,#2					;@ GG
	bl Z80Reset

	ldr r0,=emptyReadPtr
	ldr r0,[r0]
	cmp r4,#HW_SORDM5
	ldreq r0,=CTC0GetIrqVector
	str r0,[z80ptr,#z80IrqVectorFunc]
	ldreq r0,=CTC0IrqAcknowledge
	streq r0,[z80ptr,#z80IrqAckFunc]

	ldmfd sp!,{r4,lr}
	bx lr

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
