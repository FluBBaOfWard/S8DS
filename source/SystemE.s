#ifdef __arm__

#include "Equates.h"
#include "PPI8255/PPI8255.i"

	.global sysEReset
	.global sysELatch
	.global IO_Params_SysE_R
	.global IO_Params_SysE_W

//	.extern coinCounter0W
//	.extern coinCounter1W
//	.extern dipSwitch0iR
//	.extern dipSwitch1iR

	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
;@ System E
IO_Params_SysE_R:
	.long 0x005800DD, VDP1VCounterR		;@ 0x7A
	.long 0x005900DD, VDP1HCounterR		;@ 0x7B
	.long 0x005C00DD, VDP0VCounterR		;@ 0x7E
	.long 0x005D00DD, VDP0HCounterR		;@ 0x7F
	.long 0x009800DD, VDP1DataR			;@ 0xBA
	.long 0x009900DD, VDP1StatR			;@ 0xBB
	.long 0x009C00DD, VDP0DataR			;@ 0xBE
	.long 0x009D00DD, VDP0StatR			;@ 0xBF
	.long 0x0000001F, SysE_IN0_R		;@ 0xE0
	.long 0x0001001F, IOPortA_R			;@ 0xE1
	.long 0x0002001F, SysE_Player2_R	;@ 0xE2
	.long 0x0010001D, dipSwitch0iR		;@ 0xF2
	.long 0x0011001D, dipSwitch1iR		;@ 0xF3
	.long 0x00D800DF, PPI1PortAR		;@ 0xF8
	.long 0x00D900DF, PPI1PortBR		;@ 0xF9
	.long 0x00DA00DF, PPI1PortCR		;@ 0xFA
	.long 0x00DB00DF, PPI1PortDR		;@ 0xFB
	.long 0x0014001C, SysEBanking_R		;@ 0xF7
	.long 0x00000000, empty_R			;@ everything else
IO_Params_SysE_W:
	.long 0x005800DC, SN76496_0_W		;@ 0x7A, 0x7B
	.long 0x005C00DC, SN76496_1_W		;@ 0x7E, 0x7F
	.long 0x009800DD, VDP1DataSMSW		;@ 0xBA
	.long 0x009900DD, VDP1CtrlW			;@ 0xBB
	.long 0x009C00DD, VDP0DataSMSW		;@ 0xBE
	.long 0x009D00DD, VDP0CtrlW			;@ 0xBF
	.long 0x00D800DF, PPI1PortAW		;@ 0xF8
	.long 0x00D900DF, PPI1PortBW		;@ 0xF9
	.long 0x00DA00DF, PPI1PortCW		;@ 0xFA
	.long 0x00DB00DF, PPI1PortDW		;@ 0xFB
	.long 0x0014001C, SysEBanking_W		;@ 0xF7
//	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00000000, empty_W			;@ everything else

;@----------------------------------------------------------------------------
sysEReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}

	mov r1,#0xFF
	strb r1,sysELatch
	mov r1,#0
	ldr r0,=SPRS
	strb r1,[r0]

	bl PPI1Reset
	ldr r0,=SysEOutput_W
	str r0,[ppiptr,#ppiPortBOutFptr]
	ldr r0,=SysEAnalogW
	str r0,[ppiptr,#ppiPortCOutFptr]

	ldmfd sp!,{r4,lr}
	bx lr
;@----------------------------------------------------------------------------
SysE_IN0_R:					;@ Coin, service, start, 0xE0
;@----------------------------------------------------------------------------
	mov r0,#0xFF
	ldr r1,=joy0State
	ldrb r1,[r1]
	ldr r2,=joy1State
	ldrb r2,[r2]
	tst r1,#0x80				;@ Start1
	bicne r0,r0,#0x40			;@ Start1
	bicne r0,r0,#0x10			;@ Start1, HangOnJr
	tst r2,#0x80				;@ Start2
	bicne r0,r0,#0x80			;@ Start2
	tst r1,#0x40				;@ Select1
	bicne r0,r0,#0x01			;@ Coin1
	tst r2,#0x40				;@ Select2
	bicne r0,r0,#0x02			;@ Coin2
	ldr r1,=acExtra
	ldr r1,[r1]
	tst r1,#0x4000				;@ NDS L
	bicne r0,r0,#0x04			;@ Service menu
	tst r1,#0x0020				;@ NDS Select+Right
	bicne r0,r0,#0x08			;@ Service coin
	bx lr
;@----------------------------------------------------------------------------
SysE_Player2_R:				;@ Player2, 0xE2
;@----------------------------------------------------------------------------
	ldr r0,=joy1State
	ldrb r0,[r0]
	and r0,r0,#0x3F
	eor r0,r0,#0xFF
	bx lr

;@----------------------------------------------------------------------------
SysEOutput_W:				;@ 0xF9, PPI_PortB
;@----------------------------------------------------------------------------
	;@ Bit 0 = Coin Counter 1, bit 1 = Coin Counter 2, bit 2 = Start Lamp Hang On Jr.
	stmfd sp!,{r3,lr}

	mov r3,r0
	and r0,r3,#0x01
	bl coinCounter0W
	and r0,r3,#0x02
	bl coinCounter1W

	ldmfd sp!,{r3,lr}
	bx lr
;@----------------------------------------------------------------------------
SysEAnalogW:				;@ 0xFA
;@----------------------------------------------------------------------------
	ldrb r1,[ppiptr,#ppiPortCOut]
	and r1,r1,#0x0F
	ldr r2,=gArcadeGameSet
	ldrb r2,[r2]
	cmp r2,#AC_RIDDLE_OF_PYTHAGORAS
	cmpne r2,#AC_MEGUMI_RESCUE
	ldr r2,=joy0State
	ldrb r2,[r2]
	beq pythagoras
	cmp r1,#0x08
	bne SysEAccel
	mov r0,#0x80
	tst r2,#0x04
	movne r0,#0x30
	tst r2,#0x08
	movne r0,#0xD0
	strb r0,[ppiptr,#ppiPortAIn]
	bx lr
SysEAccel:
	mov r0,#0x30
	strb r0,[ppiptr,#ppiPortAIn]
	cmp r1,#0x09
	bxne lr
	tst r2,#0x10
	movne r0,#0xE8
	strb r0,[ppiptr,#ppiPortAIn]
	bx lr
;@----------------------------------------------------------------------------
pythagoras:
	mov r0,#0x00
	and r1,r1,#0x0C
	cmp r1,#0x04
	strb r0,[ppiptr,#ppiPortAIn]
	bxhi lr
	beq pythagorasButton
	tst r2,#0x01
	movne r0,#0x28
	tst r2,#0x02
	movne r0,#0xD8
	strb r0,[ppiptr,#ppiPortAIn]
	bx lr
pythagorasButton:
	tst r2,#0x01
	movne r0,#0x00
	tst r2,#0x02
	movne r0,#0x0F
	tst r2,#0x10
	orrne r0,r0,#0x40
	tst r2,#0x20
	orrne r0,r0,#0x10
	strb r0,[ppiptr,#ppiPortAIn]
	bx lr
;@----------------------------------------------------------------------------
SysEBanking_R:				;@ 0xF7
;@----------------------------------------------------------------------------
	mov r0,#0xFF
;@----------------------------------------------------------------------------
SysEBanking_W:				;@ 0xF7
;@----------------------------------------------------------------------------
	stmfd sp!,{r0,lr}
	ldrb r1,sysELatch
	strb r0,sysELatch
	eor r1,r1,r0
	ands r1,r1,#0xC0
	blne SystemESetVRAM

	ldrb r0,sysELatch
	and r0,r0,#0x0F
	add r0,r0,#2
	bl BankSwitch2_W

	ldrb r0,sysELatch
	and r0,r0,#0x10				;@ bit #4 controls sound mute.
	bl setMuteSoundGame

	ldmfd sp!,{r0,pc}
sysELatch:
	.byte 0
	.space 3
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
