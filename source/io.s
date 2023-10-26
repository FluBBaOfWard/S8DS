#ifdef __arm__

#include "ARMZ80/ARMZ80.i"
#include "Equates.h"
#include "SegaVDP/SegaVDP.i"
#include "PPI8255/PPI8255.i"
#include "Shared/EmuMenu.i"

	.global ioReset
	.global refreshEMUjoypads
	.global convertInput
	.global Z80In
	.global Z80Out

	.global joyCfg
	.global inputHW
	.global ym2413Enabled
	.global joy0State
	.global joy1State
	.global acExtra
	.global colecoKey
	.global sc3Keyboard
	.global keyboardRows
	.global emptyReadPtr

	.global AGBinput
	.global EMUinput
	.global coinCounter0
	.global coinCounter1
	.global dipSwitch0
	.global dipSwitch1
	.global coinCounter0W
	.global coinCounter1W
	.global dipSwitch0iR
	.global dipSwitch1iR

	.global PPI_1
	.global PPI1Reset
	.global PPI1PortAR
	.global PPI1PortBR
	.global PPI1PortCR
	.global PPI1PortDR
	.global PPI1PortAW
	.global PPI1PortBW
	.global PPI1PortCW
	.global PPI1PortDW
	.global IOPortA_R

	.extern sdscHandler


	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
;@ SG-1000				Fix real io map (mixed access for some devices).
IO_Params_SG1000_R:
	.long 0x00200061, VDP0DataR			;@ 0x20-0x3E, 0xA0-0xBE
	.long 0x00210061, VDP0StatR			;@ 0x21-0x3F, 0xA1-0xBF
	.long 0x00400063, PPI1PortAR		;@ 0x40-0x5C, 0xC0-0xDC
	.long 0x00410063, PPI1PortBR		;@ 0x41-0x5D, 0xC1-0xDD
	.long 0x00420063, PPI1PortCR		;@ 0x42-0x5E, 0xC2-0xDE
	.long 0x00430063, PPI1PortDR		;@ 0x43-0x5F, 0xC3-0xDF
	.long 0x00000000, empty_R_SMS1		;@ 0x00-0x1F, 0x60-0x9F, 0xE0-0xFF
IO_Params_SG1000_W:
	.long 0x006000E0, SN76496_W			;@ 0x60-0x7F
	.long 0x00A000E1, VDP0DataTMSW		;@ 0xA0-0xBE
	.long 0x00A100E1, VDP0CtrlW			;@ 0xA1-0xBF
	.long 0x00C000E3, PPI1PortAW		;@ 0xC0-0xDC
	.long 0x00C100E3, PPI1PortBW		;@ 0xC1-0xDD
	.long 0x00C200E3, PPI1PortCW		;@ 0xC2-0xDE
	.long 0x00C300E3, PPI1PortDW		;@ 0xC3-0xDF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00000000, empty_W			;@ 0x00-0x5F, 0x80-0x9F, 0xE0-0xFF

;@----------------------------------------------------------------------------
;@ Othello Multivision
IO_Params_OMV_R:
	.long 0x00200061, VDP0DataR			;@ 0x20-0x3E, 0xA0-0xBE
	.long 0x00210061, VDP0StatR			;@ 0x21-0x3F, 0xA1-0xBF
	.long 0x00C000E1, IOPortA_R			;@ 0xC0-0xDE
	.long 0x00C100E1, IOPortB_R			;@ 0xC1-0xDF
	.long 0x00E000E7, OMVPort1_R		;@ 0xE0-0xF8
	.long 0x00E100E7, OMVPort2_R		;@ 0xE1-0xF9
	.long 0x00E200E7, OMVPort3_R		;@ 0xE2-0xFA
	.long 0x00E300E7, OMVPort4_R		;@ 0xE3-0xFB
	.long 0x00000000, empty_R_SMS1		;@ 0x00-0x1F, 0x40-0x9F, 0xFC-0xFF
IO_Params_OMV_W:
	.long 0x006000E0, SN76496_W			;@ 0x60-0x7F
	.long 0x00A000E1, VDP0DataTMSW		;@ 0xA0-0xBE
	.long 0x00A100E1, VDP0CtrlW			;@ 0xA1-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00000000, empty_W			;@ 0x00-0x5F, 0x80-0x9F, 0xE0-0xFF

;@----------------------------------------------------------------------------
;@ Mark III
IO_Params_Mark3_R:
	.long 0x004000C1, VDP0VCounterR		;@ 0x40-0x7E
	.long 0x004100C1, VDP0HCounterR		;@ 0x41-0x7F
	.long 0x008000C1, VDP0DataR			;@ 0x80-0xBE
	.long 0x008100C1, VDP0StatR			;@ 0x81-0xBF
	.long 0x00F000FC, ExternalIO_R		;@ 0xF0-0xF2
	.long 0x00C000C1, ExtIO_0_SMS_R		;@ 0xC0-0xFE
	.long 0x00C100C1, ExtIO_1_SMS_R		;@ 0xC1-0xFF
	.long 0x00000000, empty_R_SMS1		;@ 0x00-0x3F
IO_Params_Mark3_W:
	.long 0x004000C0, SN76496_W			;@ 0x40-0x7F
	.long 0x008000C1, VDP0DataSMSW		;@ 0x80-0xBE
	.long 0x008100C1, VDP0CtrlW			;@ 0x81-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00C000C0, ExternalIO_W		;@ 0xC0-0xFF
	.long 0x00000000, empty_W			;@ 0x00-0x3F

;@----------------------------------------------------------------------------
;@ SMS1
IO_Params_SMS1_R:
	.long 0x004000C1, VDP0VCounterR		;@ 0x40-0x7E
	.long 0x004100C1, VDP0HCounterR		;@ 0x41-0x7F
	.long 0x008000C1, VDP0DataR			;@ 0x80-0xBE
	.long 0x008100C1, VDP0StatR			;@ 0x81-0xBF
	.long 0x00F000FC, ExternalIO_R		;@ 0xF0-0xF2
	.long 0x00C000C1, ExtIO_0_SMS_R		;@ 0xC0-0xFE
	.long 0x00C100C1, ExtIO_1_SMS_R		;@ 0xC1-0xFF
	.long 0x00000000, empty_R_SMS1		;@ 0x00-0x3F
IO_Params_SMS1_W:
	.long 0x000000C1, MemCtrl_SMS_W		;@ 0x00-0x3E
	.long 0x000100C1, IOCtrl_SMS_W		;@ 0x01-0x3F
	.long 0x004000C0, SN76496_W			;@ 0x40-0x7F
	.long 0x008000C1, VDP0DataSMSW		;@ 0x80-0xBE
	.long 0x008100C1, VDP0CtrlW			;@ 0x81-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00C000C0, ExternalIO_W		;@ 0xC0-0xFF
	.long 0x00000000, empty_W			;@

;@----------------------------------------------------------------------------
;@ SMS2
IO_Params_SMS2_R:
	.long 0x004000C1, VDP0VCounterR		;@ 0x40-0x7E
	.long 0x004100C1, VDP0HCounterR		;@ 0x41-0x7F
	.long 0x008000C1, VDP0DataR			;@ 0x80-0xBE
	.long 0x008100C1, VDP0StatR			;@ 0x81-0xBF
	.long 0x00C000C1, ExtIO_0_SMS_R		;@ 0xC0-0xFE
	.long 0x00C100C1, ExtIO_1_SMS_R		;@ 0xC1-0xFF
	.long 0x00000000, empty_R			;@ 0x00-0x3F
IO_Params_SMS2_W:
	.long 0x000000C1, MemCtrl_SMS_W		;@ 0x00-0x3E
	.long 0x000100C1, IOCtrl_SMS_W		;@ 0x01-0x3F
	.long 0x004000C0, SN76496_W			;@ 0x40-0x7F
	.long 0x008000C1, VDP0DataSMSW		;@ 0x80-0xBE
	.long 0x008100C1, VDP0CtrlW			;@ 0x81-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00C000C0, ExternalIO_W		;@ 0xC0-0xFF
	.long 0x00000000, empty_W			;@

;@----------------------------------------------------------------------------
;@ Game Gear, SMS mode
IO_Params_GGSMS_R:
	.long 0x004000C1, VDP0VCounterR		;@ 0x40-0x7E
	.long 0x004100C1, VDP0HCounterR		;@ 0x41-0x7F
	.long 0x008000C1, VDP0DataR			;@ 0x80-0xBE
	.long 0x008100C1, VDP0StatR			;@ 0x81-0xBF
	.long 0x00C000FF, IOPortA_R			;@ 0xC0
	.long 0x00C100FF, IOPortB_R			;@ 0xC1
	.long 0x00DC00FF, IOPortA_R			;@ 0xDC
	.long 0x00DD00FF, IOPortB_R			;@ 0xDD
	.long 0x00000000, empty_R			;@ 0x00-0x3F, 0xC0-0xFF
IO_Params_GGSMS_W:
//	.long 0x000600FF, StereoCtrl_GG_W	;@ 0x06
	.long 0x000700FF, IOCtrl_SMS_W		;@ 0x07
	.long 0x000000F8, IOCtrl_GG_W		;@ 0x00-0x05
	.long 0x000000C1, BankSwitchB_GG_W	;@ 0x08-0x3E
	.long 0x000100C1, IOCtrl_SMS_W		;@ 0x09-0x3F
	.long 0x004000C0, SN76496_W			;@ 0x40-0x7F
	.long 0x008000C1, VDP0DataSMSW		;@ 0x80-0xBE
	.long 0x008100C1, VDP0CtrlW			;@ 0x81-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00C000C0, ExternalIO_W		;@ 0xC0-0xFF
	.long 0x00000000, empty_W			;@

;@----------------------------------------------------------------------------
;@ Game Gear
IO_Params_GG_R:
	.long 0x000000FF, Low0_IO_R			;@ 0x00
	.long 0x000000F8, Low1_IO_R			;@ 0x01-0x07
	.long 0x004000C1, VDP0VCounterR		;@ 0x40-0x7E
	.long 0x004100C1, VDP0HCounterR		;@ 0x41-0x7F
	.long 0x008000C1, VDP0DataR			;@ 0x80-0xBE
	.long 0x008100C1, VDP0StatR			;@ 0x81-0xBF
	.long 0x00C000FF, IOPortA_R			;@ 0xC0
	.long 0x00C100FF, IOPortB_R			;@ 0xC1
	.long 0x00DC00FF, IOPortA_R			;@ 0xDC
	.long 0x00DD00FF, IOPortB_R			;@ 0xDD
	.long 0x00000000, empty_R			;@ 0x08-0x3F, 0xC0-0xFF
IO_Params_GG_W:
	.long 0x000600FF, StereoCtrl_GG_W	;@ 0x06
	.long 0x000700FF, IOCtrl_SMS_W		;@ 0x07
	.long 0x000000F8, IOCtrl_GG_W		;@ 0x00-0x05
	.long 0x000000C1, BankSwitchB_GG_W	;@ 0x08-0x3E
	.long 0x000100C1, IOCtrl_SMS_W		;@ 0x09-0x3F
	.long 0x004000C0, SN76496_W			;@ 0x40-0x7F
	.long 0x008000C1, VDP0DataGGW		;@ 0x80-0xBE
	.long 0x008100C1, VDP0CtrlW			;@ 0x81-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00C000C0, ExternalIO_W		;@ 0xC0-0xFF
	.long 0x00000000, empty_W			;@
;@----------------------------------------------------------------------------
;@ Mega Drive
IO_Params_MD_R:
	.long 0x004000C1, VDP0VCounterR		;@ 0x40-0x7E
	.long 0x004100C1, VDP0HCounterR		;@ 0x41-0x7F
	.long 0x008000C1, VDP0DataR			;@ 0x80-0xBE
	.long 0x008100C1, VDP0StatR			;@ 0x81-0xBF
	.long 0x00C000FF, IOPortA_R			;@ 0xC0
	.long 0x00C100FF, IOPortB_R			;@ 0xC1
	.long 0x00DC00FF, IOPortA_R			;@ 0xDC
	.long 0x00DD00FF, IOPortB_R			;@ 0xDD
	.long 0x00000000, empty_R			;@ 0x00-0x3F, 0xC0-0xFF
IO_Params_MD_W:
	.long 0x000000C1, MemCtrl_MD_W		;@ 0x00-0x3F
	.long 0x000100C1, IOCtrl_SMS_W		;@ 0x00-0x3F
	.long 0x004000C0, SN76496_W			;@ 0x40-0x7F
	.long 0x008000C1, VDP0DataMDW		;@ 0x80-0xBE
	.long 0x008100C1, VDP0CtrlMDW		;@ 0x81-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00C000C0, ExternalIO_W		;@ 0xC0-0xFF
	.long 0x00000000, empty_W			;@

;@----------------------------------------------------------------------------
;@ Coleco
IO_Params_Coleco_R:
	.long 0x00A000E1, VDP0DataR			;@ 0xA0-0xBE
	.long 0x00A100E1, VDP0StatR			;@ 0xA1-0xBF
	.long 0x00E000E2, ColecoPortA_R		;@ 0xE0-0xFD
	.long 0x00E200E2, ColecoPortB_R		;@ 0xE2-0xFF
	.long 0x00000000, empty_R			;@ 0x00-0x9F, 0xC0-0xDF
IO_Params_Coleco_W:
	.long 0x008000E0, setKeypadWrite	;@ 0x80-0x9F
	.long 0x00A000E1, VDP0DataTMSW		;@ 0xA0-0xBE
	.long 0x00A100E1, VDP0CtrlW			;@ 0xA1-0xBF
	.long 0x00C000E0, setJoystickWrite	;@ 0xC0-0xDF
	.long 0x00E000E0, SN76496_W			;@ 0xE0-0xFF
	.long 0x00000000, empty_W			;@ 0x00-0x7F

;@----------------------------------------------------------------------------
;@ MegaTech
IO_Params_MT_R:
	.long 0x004000C1, MTVDPVCounterR	;@ 0x40-0x7E
	.long 0x004100C1, MTVDPHCounterR	;@ 0x41-0x7F
	.long 0x008000C1, MTVDPDataR		;@ 0x80-0xBE
	.long 0x008100C1, MTVDPStatR		;@ 0x81-0xBF
	.long 0x00C000C1, ExtIO_0_SMS_R		;@ 0xC0-0xFE
	.long 0x00C100C1, ExtIO_1_SMS_R		;@ 0xC1-0xFF
	.long 0x00000000, empty_R			;@ 0x00-0x3F
IO_Params_MT_W:
	.long 0x000000C1, MemCtrl_SMS_W		;@ 0x00-0x3E
	.long 0x000100C1, IOCtrl_SMS_W		;@ 0x01-0x3F
	.long 0x004000C0, SN76496_W			;@ 0x40-0x7F
	.long 0x008000C1, MTVDPDataW		;@ 0x80-0xBE
	.long 0x008100C1, MTVDPCtrlW		;@ 0x81-0xBF
	.long 0x00FD00FF, SDSC_Debug_W		;@ 0xFD
	.long 0x00C000C0, ExternalIO_W		;@ 0xC0-0xFF
	.long 0x00000000, empty_W			;@

emptyReadPtr:
	.long empty_R
joypadA_R_ptr:
	.long joypadA2ButtonR
joypadB_R_ptr:
	.long joypadB2ButtonR
;@----------------------------------------------------------------------------
io_params:
;@----------------------------------------------------------------------------
	.long IO_Params_SMS2_R,   IO_Params_SMS2_W			;@ HW_AUTO
	.long IO_Params_SG1000_R, IO_Params_SG1000_W		;@ HW_SG1000
	.long IO_Params_SG1000_R, IO_Params_SG1000_W		;@ HW_SC3000
	.long IO_Params_OMV_R,    IO_Params_OMV_W			;@ HW_OMV
	.long IO_Params_SG1000_R, IO_Params_SG1000_W		;@ HW_SG1000II
	.long IO_Params_Mark3_R,  IO_Params_Mark3_W			;@ HW_Mark3
	.long IO_Params_SMS1_R,   IO_Params_SMS1_W			;@ HW_SMS1
	.long IO_Params_SMS2_R,   IO_Params_SMS2_W			;@ HW_SMS2
	.long IO_Params_GGSMS_R,  IO_Params_GGSMS_W			;@ HW_GG (SMS mode)
	.long IO_Params_MD_R,     IO_Params_MD_W			;@ HW_MEGADRIVE
	.long IO_Params_Coleco_R, IO_Params_Coleco_W		;@ HW_COLECO
	.long IO_Params_MSX_R,    IO_Params_MSX_W			;@ HW_MSX
	.long IO_Params_SordM5_R, IO_Params_SordM5_W		;@ HW_SORDM5
	.long IO_Params_SysE_R,   IO_Params_SysE_W			;@ HW_SYSE
	.long IO_Params_SG1000_R, IO_Params_SG1000_W		;@ HW_SGAC
//	.long IO_Params_MT_R,     IO_Params_MT_W			;@ HW_MEGATECH
	.long IO_Params_MD_R,     IO_Params_MD_W			;@ HW_MEGATECH
	.long IO_Params_GG_R,     IO_Params_GG_W			;@ HW_GG (GG mode)

;@----------------------------------------------------------------------------
ioReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}

	ldr r2,=GGIO_Default
	ldmia r2!,{r0-r1}
	stmia r2,{r0-r1}
	mov r1,#0xFF
	strb r1,sc3Keyboard

	ldr r4,=gMachine
	ldrb r4,[r4]

	ldr r2,=joypadA2ButtonR
	ldr r3,=joypadB2ButtonR
	mov r1,#0xC0
	cmp r4,#HW_MEGADRIVE
	cmpne r4,#HW_MEGATECH
	moveq r1,#0xE0				;@ Bit 5 is different on MD.
	strb r1,joy1Extra
	ldrb r0,inputHW
	cmpeq r0,#0
	cmpne r0,#2
	ldreq r2,=joypadA3ButtonR
	ldreq r3,=joypadB3ButtonR
	str r2,joypadA_R_ptr
	str r3,joypadB_R_ptr


	cmp r4,#HW_SMS1
	cmpne r4,#HW_SG1000
	cmpne r4,#HW_SGAC
	cmpne r4,#HW_SC3000
	cmpne r4,#HW_OMV
	cmpne r4,#HW_SG1000II
	ldr r1,=empty_R
	ldreq r1,=empty_R_SMS1
	str r1,emptyReadPtr

	ldr r3,=gEmuFlags
	ldrb r3,[r3]
	tst r3,#GG_MODE
	addne r4,r4,#8

	adr lr,io_params
	ldr r0,[lr,r4,lsl#3]!
	ldr r1,[lr,#4]

	bl IO_Install2Tables

	cmp r4,#HW_SMS1
	cmpne r4,#HW_SMS2
	ldreq r2,=gMachineSet
	ldrbeq r2,[r2]
	cmpeq r2,#HW_AUTO

	ldreq r0,=empty_W
	ldreq r1,=outTable
	streq r0,[r1,#0x06*4]		;@ Fake fix for Frogger proto conversion.
	streq r0,[r1,#0x18*4]		;@ Fake fix for Super Tetris (KR).

	cmp r4,#HW_SG1000
	cmpne r4,#HW_SC3000
	cmpne r4,#HW_SG1000II
	bleq sgReset
	cmp r4,#HW_SYSE
	bleq sysEReset
	cmp r4,#HW_MSX
	bleq msxReset
	cmp r4,#HW_SORDM5
	bleq sordM5Reset
	cmp r4,#HW_SGAC
	bleq sgacReset
	cmp r4,#HW_MEGATECH
	bleq megaTechReset

	ldmfd sp!,{r4,lr}
	bx lr
;@----------------------------------------------------------------------------
IO_Install2Tables:			;@ r0 = tbl0, r1 = tbl1.
;@----------------------------------------------------------------------------
	stmfd sp!,{r1,lr}
	ldr r1,=inTable
	bl IO_InstallTable
	ldmfd sp!,{r0,lr}
	ldr r1,=outTable
;@----------------------------------------------------------------------------
IO_InstallTable:			;@ r0 = mask+routine table ptr, r1 = destination
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,r5}
	mov r2,#0xFF
ioTLoop0:
	mov r3,r0
ioTLoop1:
	ldmia r3!,{r4,r5}
	and r12,r2,r4
	cmp r12,r4,lsr#16
	bne ioTLoop1
	str r5,[r1,r2,lsl#2]
	subs r2,r2,#1
	bpl ioTLoop0
	ldmfd sp!,{r4,r5}
	bx lr

;@----------------------------------------------------------------------------
sgReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	bl PPI1Reset
	ldr r0,=IOPortA_SG_R
	str r0,[ppiptr,#ppiPortAInFptr]
	ldr r0,=IOPortB_SG_R
	str r0,[ppiptr,#ppiPortBInFptr]
	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
sgacReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	bl PPI1Reset
	ldr r0,=SGAC_P1_R
	str r0,[ppiptr,#ppiPortAInFptr]
	ldr r0,=SGAC_P2_R
	str r0,[ppiptr,#ppiPortBInFptr]
	ldr r0,=dipSwitch0iR
	str r0,[ppiptr,#ppiPortCInFptr]
	ldr r0,=SGAC_Coin_W
	str r0,[ppiptr,#ppiPortCOutFptr]
	ldmfd sp!,{lr}
	bx lr

	.pool
;@----------------------------------------------------------------------------
convertInput:			;@ Convert from device keys to target r0=input/output
	.type convertInput STT_FUNC
;@----------------------------------------------------------------------------
	mvn r1,r0
	tst r1,#KEY_L|KEY_R				;@ Keys to open menu
	orreq r0,r0,#KEY_OPEN_MENU
	bx lr
;@----------------------------------------------------------------------------
refreshEMUjoypads:			;@ Call every frame
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldr r0,=frameTotal
	ldr r0,[r0]
	tst r0,r0,lsr#2		;@ C=frame&2 (autofire alternates every other frame)
	ldr r4,EMUinput
	mov r1,r4
		ldr r2,joyCfg
		andcs r1,r1,r2
		tstcs r1,r1,lsr#9		;@ R?
		andcs r1,r1,r2,lsr#16
	ands r0,r1,#3
	cmpne r0,#3
	tstne r2,#0x400				;@ Swap A/B?
	eorne r0,r0,#3
	bic r4,r4,#3
	orr r4,r4,r0
	and r3,r4,#0xf0

	ldr r1,=gMachine
	ldrb r1,[r1]
	cmp r1,#HW_COLECO
	beq refreshColJoypads
	cmp r1,#HW_MSX
	beq refreshMSXJoypads
	cmp r1,#HW_SORDM5
	beq refreshSM5Joypads

;@----------------------------------------------------------------------------
refreshSMSJoypads:
;@----------------------------------------------------------------------------
	cmp r1,#HW_SYSE
	cmpne r1,#HW_SGAC
	beq noPause
	ldr r0,=gConfig
	ldrb r0,[r0]
	tst r0,#0x20
	biceq r4,r4,#0x04			;@ Remove Reset if disabled
	ldr r0,=gEmuFlags
	ldrb r0,[r0]
	tst r0,#GG_MODE
	bne noPause
	orr r0,r4,r4,lsr#7			;@ NDS X
	and r0,r0,#0x08				;@ NDS START
	cmp r1,#HW_SG1000
	cmpne r1,#HW_OMV
	cmpne r1,#HW_SC3000
	cmpne r1,#HW_SG1000II
	ldrne vdpptr,=VDP0
	strbne r0,[vdpptr,#vdpDebouncePin]
	bleq Z80SetNMIPin
noPause:
	bl refreshArcadeInput
	adr addy,rlud2durl
	ldrb r0,[addy,r3,lsr#4]		;@ downupleftright

	orr r0,r0,r4,lsl#4
	mov r3,r4,lsr#8

	mov r1,#0

	tst r2,#0x40000000			;@ Player2?
	strbeq r0,joy0State
	strbeq r3,joy0Extra
	strbne r0,joy1State

	strbne r1,joy0State
	strbne r1,joy0Extra
	strbeq r1,joy1State

	ldmfd sp!,{lr}
	bx lr

	;@ paddle stuff
	;@--------------------------------
;@	ldrb r3,paddle0x
;@	tst r0,#0x8
;@	addne r3,r3,#2
;@	tst r0,#0x4
;@	subnes r3,r3,#2
;@	movmi r3,#0
;@	cmp r3,#0xFF
;@	movpl r3,#0xFF
;@	strb r3,paddle0x
	;@--------------------------------

;@----------------------------------------------------------------------------
refreshColJoypads:
;@----------------------------------------------------------------------------
	adr addy,joy2Coleco
	ldrb r0,[addy,r3,lsr#4]		;@ downupleftright

	orr r0,r0,r4,lsl#5			;@ Button 1
	mov r1,r4,lsl#6				;@ Button 2
	tst r4,#0x4					;@ Select
//	ldrb r3,gKeymapSelect
	mov r3,#0x03
	orrne r1,r1,r3				;@ (3)
	tst r4,#0x8					;@ Start
//	ldrb r3,gKeymapStart
	mov r3,#0x02
	orrne r1,r1,r3				;@ (1)

	tst r4,#0x200				;@ L
//	ldrb r3,gKeymapL
	mov r3,#0x09
	orrne r1,r1,r3				;@ (#)
	tst r4,#0x100				;@ R
//	ldrb r3,gKeymapR
	mov r3,#0x06
	orrne r1,r1,r3				;@ (*)
	ldrb r3,colecoKey
	orr r1,r1,r3

	tst r2,#0x40000000			;@ Player2?
	mov r2,#0
	str r2,joy0State
	strbeq r0,joy0State
	strbne r0,joy1State
	strbeq r1,joy0Extra
	strbne r1,joy1Extra
	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
refreshMSXJoypads:			;@ Call every frame
;@----------------------------------------------------------------------------
	adr addy,rlud2durl
	ldrb r0,[addy,r3,lsr#4]		;@ downupleftright

	orr r0,r0,r4,lsl#4
	and r0,r0,#0x3F
	eor r0,r0,#0xFF
	mov r1,#0xFF

	tst r2,#0x40000000			;@ Player 2?
	strbeq r0,joy0State
	strbne r0,joy1State
	strbne r1,joy0State
	strbeq r1,joy1State
	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
refreshSM5Joypads:			;@ Call every frame
;@----------------------------------------------------------------------------
	adr addy,rlud2ruld
	ldrb r0,[addy,r3,lsr#4]		;@ downupleftright
	and r1,r4,#0x03

	tst r2,#0x40000000			;@ Player 2?
	movne r0,r0,lsl#4
	movne r1,r1,lsl#4

	strb r0,joy0State
	strb r1,joy1State
	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
refreshArcadeInput:
;@----------------------------------------------------------------------------
	mov r1,#0
	and r2,r4,#0x0F0
	tst r4,#0x008				;@ NDS Start
	orrne r1,r1,r2,lsr#4
	orrne r1,r1,#0x010
	tst r4,#0x004				;@ NDS Select
	orrne r1,r1,r2,lsl#1
	orrne r1,r1,#0x200
	tst r4,#0x200				;@ NDS L
	orrne r1,r1,r2,lsl#6
	orrne r1,r1,#0x4000
	tst r4,#0x100				;@ NDS R
	orrne r1,r1,#0x8000

	str r1,acExtra

	bx lr

;@----------------------------------------------------------------------------
AGBinput:		.long 0	;@ This label here for main.c to use
EMUinput:		.long 0	;@ EMUjoypad (this is what Emu sees)
joyCfg:			.long 0x00ff05ff	;@ byte0=auto mask, byte1=(saves R), byte2=R auto mask
						;@ bit 31=single/multi, 30=1P/2P, 27=(multi) link active, 24=reset signal received
nrPlayers:		.long 0	;@ Number of players in multilink.
joy0State:		.byte 0
joy1State:		.byte 0
joy0Extra:		.byte 0
joy1Extra:		.byte 0
joyMode:		.byte 0
colecoKey:		.byte 0
inputHW:		.byte 0
ym2413Enabled:	.byte 1

sc3Keyboard:
keyboardRows:
keyboardRow0:	.byte 0xFF
keyboardRow1:	.byte 0xFF
keyboardRow2:	.byte 0xFF
keyboardRow3:	.byte 0xFF
keyboardRow4:	.byte 0xFF
keyboardRow5:	.byte 0xFF
keyboardRow6:	.byte 0xFF
keyboardRow7:	.byte 0xFF
acExtra:		.long 0

rlud2durl:		.byte 0x00,0x08,0x04,0x0C, 0x01,0x09,0x05,0x0D, 0x02,0x0A,0x06,0x0E, 0x03,0x0B,0x07,0x0F
rlud2ruld:		.byte 0x00,0x01,0x04,0x05, 0x02,0x03,0x06,0x07, 0x08,0x09,0x0C,0x0D, 0x0A,0x0B,0x0E,0x0F
joy2Coleco:		.byte 0x00,0x02,0x08,0x0A, 0x01,0x03,0x09,0x0B, 0x04,0x06,0x0C,0x0E, 0x05,0x07,0x0D,0x0F

;@ Arcade Stuff
coinCounter0:	.long 0
coinCounter1:	.long 0
coinCounter2:	.long 0
coinCounter3:	.long 0
dipSwitch0:		.byte 0
dipSwitch1:		.byte 0
dipSwitch2:		.byte 0
dipSwitch3:		.byte 0
dipSwitch4:		.byte 0
dipSwitch5:		.byte 0
dipSwitch6:		.byte 0
dipSwitch7:		.byte 0
coinState0:		.byte 0
coinState1:		.byte 0
coinState2:		.byte 0
coinState3:		.byte 0
;@----------------------------------------------------------------------------
coinCounter0W:					;@ r0 = new state
;@----------------------------------------------------------------------------
	cmp r0,#0
	movne r0,#0xFF
	ldrb r1,coinState0
	strb r0,coinState0
	bics r1,r0,r1
	ldrne r1,coinCounter0
	addne r1,r1,#1
	strne r1,coinCounter0
	bx lr
;@----------------------------------------------------------------------------
coinCounter1W:					;@ r0 = new state
;@----------------------------------------------------------------------------
	cmp r0,#0
	movne r0,#0xFF
	ldrb r1,coinState1
	strb r0,coinState1
	bics r1,r0,r1
	ldrne r1,coinCounter1
	addne r1,r1,#1
	strne r1,coinCounter1
	bx lr
;@----------------------------------------------------------------------------
dipSwitch0iR:
;@----------------------------------------------------------------------------
	ldrb r0,dipSwitch0
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
dipSwitch1iR:
;@----------------------------------------------------------------------------
	ldrb r0,dipSwitch1
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------

;@----------------------------------------------------------------------------
SGAC_P1_R:					;@ SG-1000 Arcade input P1, PPI_PortA
;@----------------------------------------------------------------------------
	ldrb r1,joy1State
	and r1,r1,#0x80				;@ P2 start
	ldrb r0,joy0State
	tst r0,#0x80
	and r0,r0,#0x3F
	orrne r0,r0,#0x40			;@ P1 start
	orr r0,r0,r1
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
SGAC_P2_R:					;@ SG-1000 Arcade input P2, PPI_PortB
;@----------------------------------------------------------------------------
	ldrb r0,joy1State
	and r0,r0,#0x3F
	ldr r1,acExtra
	tst r1,#0x0200				;@ NDS Select
	orrne r0,r0,#0x80			;@ Coin
	tst r1,#0x0020				;@ NDS Select+Right
	orrne r0,r0,#0x40			;@ Service coin
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
SGAC_Coin_W:				;@ PPI_PortC
;@----------------------------------------------------------------------------
	and r0,r0,#0x01
	b coinCounter0W
;@----------------------------------------------------------------------------
IOPortA_SG_R:				;@ Player1/KB, PPI_PortA
;@----------------------------------------------------------------------------
	ldrb r0,[ppiptr,#ppiPortCOut]
	and r0,r0,#7
	cmp r0,#7
	beq IOPortA_R

	ldrb r1,sc3Keyboard
	cmp r0,r1,lsr#4				;@ Check row
	mov r0,#0xFF
	bxne lr
	and r1,r1,#0x0F
	mov r2,#1
	bic r0,r0,r2,lsl r1
	bx lr
;@----------------------------------------------------------------------------
IOPortB_SG_R:				;@ Player2/KB/Printer/Cassette input, PPI_PortB
;@----------------------------------------------------------------------------
	mov r0,#0x7F				;@ Default output
	ldrb r1,[ppiptr,#ppiPortCOut]
	and r1,r1,#7
	cmp r1,#7
	beq sgp2_r
	ldrb r2,sc3Keyboard
	cmp r1,r2,lsr#4				;@ Check row
	bxne lr
	and r2,r2,#0x0F
	subs r2,r2,#8
	mov r1,#1
	bicpl r0,r0,r1,lsl r2
	bx lr
sgp2_r:
	ldrb r1,joy1State
	and r1,r1,#0x3C
	bic r0,r0,r1,lsr#2
	bx lr

;@----------------------------------------------------------------------------
OMVPort1_R:		;@ 0xE0-0xF8
;@----------------------------------------------------------------------------
	mov r1,#0
	b setOMVKeys
;@----------------------------------------------------------------------------
OMVPort2_R:		;@ 0xE1-0xF9
;@----------------------------------------------------------------------------
	mov r1,#1
	b setOMVKeys
;@----------------------------------------------------------------------------
OMVPort3_R:		;@ 0xE2-0xFA
;@----------------------------------------------------------------------------
	mov r1,#2
	b setOMVKeys
;@----------------------------------------------------------------------------
OMVPort4_R:		;@ 0xE3-0xFB
;@----------------------------------------------------------------------------
	mov r1,#3
setOMVKeys:
	ldrb r2,sc3Keyboard
	cmp r1,r2,lsr#4				;@ Check row
	and r2,r2,#0x07
	mov r0,#0xFF				;@ Default output
	mov r1,#1
	biceq r0,r0,r1,lsl r2
	bx lr
;@----------------------------------------------------------------------------
ExtIO_0_SMS_R:
;@----------------------------------------------------------------------------
;@	mov r11,r11					;@ No$GBA breakpoint
	ldr r0,=BankMap4
	ldrb r0,[r0]
	tst r0,#0x04				;@ IO ports 0xC0 -> 0xFF enable/disable.
	ldrne pc,emptyReadPtr
;@----------------------------------------------------------------------------
IOPortA_R:					;@ Player 1...
;@----------------------------------------------------------------------------
	ldr pc,joypadA_R_ptr

;@----------------------------------------------------------------------------
joypadA2ButtonR:					;@ 2 button SMS pad
;@----------------------------------------------------------------------------
	ldrb r0,joy0State
	and r0,r0,#0x3F
	ldrb r1,joy1State
	and r1,r1,#0x03
	orr r0,r0,r1,lsl#6
	eor r0,r0,#0xFF
	bx lr

;@----------------------------------------------------------------------------
joypadA3ButtonR:					;@ 3 button MD pad
;@----------------------------------------------------------------------------
	ldrb r0,joy0State
	and r0,r0,#0x3F
	ldrb r1,joy1State
	and r1,r1,#0x03
	orr r0,r0,r1,lsl#6
	eor r0,r0,#0xFF

	ldrb r1,joy1Extra
	tst r1,#0x40				;@ Port A TH output.
	bxne lr

	bic r0,r0,#0x3C
	ldrb r1,joy0Extra
	tst r1,#0x08				;@ NDS Y
	orreq r0,r0,#0x10
	tst r1,#0x04				;@ NDS X
	orreq r0,r0,#0x20

	bx lr

;@----------------------------------------------------------------------------
ExtIO_1_SMS_R:
;@----------------------------------------------------------------------------
;@	mov r11,r11					;@ No$GBA breakpoint
	ldr r0,=BankMap4
	ldrb r0,[r0]
	tst r0,#0x04				;@ IO ports 0xC0 -> 0xFF enable/disable.
	ldrne pc,emptyReadPtr
;@----------------------------------------------------------------------------
IOPortB_R:					;@ Player 2...
;@----------------------------------------------------------------------------
	ldr pc,joypadB_R_ptr

;@----------------------------------------------------------------------------
joypadB2ButtonR:					;@ 2 button SMS pad
joypadB3ButtonR:					;@ 3 button MD pad
;@----------------------------------------------------------------------------
	ldrb r0,joy0State
	and r0,r0,#0x40				;@ Reset is taken from both players.
	ldrb r1,joy1State
	and r1,r1,#0x7C
	orr r0,r0,r1

	ldr r1,=gEmuFlags
	ldrb r1,[r1]
	tst r1,#COUNTRY				;@ Jap bit
	ldrb r1,joy1Extra
	bicne r1,#0xC0
	orr r0,r1,r0,lsr#2
	eor r0,r0,#0x3F
	bx lr

;@----------------------------------------------------------------------------
setKeypadWrite:				;@ Coleco, 0x80 written?
;@----------------------------------------------------------------------------
	mov r0,#1
	strb r0,joyMode
	bx lr
;@----------------------------------------------------------------------------
setJoystickWrite:			;@ Coleco, 0x80 written?
;@----------------------------------------------------------------------------
	mov r0,#0
	strb r0,joyMode
	bx lr
;@----------------------------------------------------------------------------
ColecoPortA_R:				;@ Player 1...
;@----------------------------------------------------------------------------
;@	mov r11,r11					;@ No$GBA breakpoint
	ldrb r0,joyMode
	cmp r0,#0
	ldrbeq r0,joy0State
	ldrbne r0,joy0Extra
	and r0,r0,#0x4F				;@ bit 4 & 5 for extra controller?
	eor r0,r0,#0x7F
	bx lr
;@----------------------------------------------------------------------------
ColecoPortB_R:				;@ Player 2...
;@----------------------------------------------------------------------------
;@	mov r11,r11					;@ No$GBA breakpoint
	ldrb r0,joyMode
	cmp r0,#0
	ldrbeq r0,joy1State
	ldrbne r0,joy1Extra
	and r0,r0,#0x4F
	eor r0,r0,#0x7F
	bx lr

;@----------------------------------------------------------------------------
Low0_IO_R:					;@ GG start button & country, 0x00
;@----------------------------------------------------------------------------
	ldr r1,=gEmuFlags
	ldrb r1,[r1]
	and r1,r1,#3				;@ Jap / PAL bits
	ldrb r0,joy0Extra			;@ Port 0x00
	and r0,r0,#0x04				;@ NDS X
	mov r0,r0,lsl#5
	orr r0,r0,r1,lsl#5
	ldrb r1,joy0State
	tst r1,#0x80
	orrne r0,r0,#0x80
;@	orr r0,r0,#0x40				;@ JAP/EXPORT
;@	orr r0,r0,#0x20				;@ NTSC/PAL
	eor r0,r0,#0xC0
	bx lr
;@----------------------------------------------------------------------------
Low1_IO_R:					;@ GG com port stuff, 0x01-0x07
;@----------------------------------------------------------------------------
	adr r1,GGIO
	ldrb r0,[r1,addy]
	bx lr

;@------------------------------------------------------------------------------
StereoCtrl_GG_W:			;@ GG stereo control, 0x06
;@------------------------------------------------------------------------------
	strb r0,GGIO+6
	b GG_Stereo_W
;@------------------------------------------------------------------------------
IOCtrl_GG_W:				;@ GG com port stuff, 0x00-0x07
;@------------------------------------------------------------------------------
	adr r1,GGIO
	strb r0,[r1,addy]
	bx lr
GGIO_Default:
	.byte 0xC0,0x7F,0xFF,0x00,0xFF,0x00,0xFF,0xFF
GGIO:
	.byte 0xC0,0x7F,0xFF,0x00,0xFF,0x00,0xFF,0xFF
;@------------------------------------------------------------------------------
IOCtrl_SMS_W:
;@------------------------------------------------------------------------------
	orr r0,r0,r0,lsl#4
	and r0,r0,#0xA0				;@ Only care about TH A/B right now.
	orr r0,r0,r0,lsl#1
	and r0,r0,#0xC0
	ldrb r1,joy1Extra
	stmfd sp!,{r0,r2,lr}
	eor r2,r1,r0
	and r2,r2,r0
	ands r2,r2,#0xC0
	blne VDP0LatchHCounter
	ldmfd sp!,{r0,r2,lr}

	bic r1,r1,#0xC0
	orr r0,r0,r1
	strb r0,joy1Extra
	bx lr

	;@ paddle stuff
	;@--------------------------------
;@	ands r0,r0,#0x40
;@	ldrb r0,joy0State
;@	and r0,r0,#0xD0
;@	orrne r0,r0,#0x20
;@	ldrb r1,paddle0x
;@	andne r1,r1,#0x0f
;@	moveq r1,r1,lsr#4
;@	eor r1,r1,#0x0F
;@	orr r0,r0,r1
;@	strb r0,joy0State
	;@--------------------------------
	
;@	bx lr

;@----------------------------------------------------------------------------
ExternalIO_R:
;@----------------------------------------------------------------------------
	ldrb r1,ym2413Enabled
	cmp r1,#0
	ldreq pc,emptyReadPtr
	cmp addy,#0xF1				;@ FM Unit
	beq YM2413_0_StatusR
	cmp addy,#0xF2				;@ FM/PSG enable/disable?
	beq SMSJSoundControlR
	ldr pc,emptyReadPtr
;@----------------------------------------------------------------------------
ExternalIO_W:
;@----------------------------------------------------------------------------
	ldrb r1,ym2413Enabled
	cmp r1,#0
	beq empty_W
	cmp addy,#0xF0				;@ FM Unit
	beq YM2413_0_AddressW
	cmp addy,#0xF1				;@ FM Unit
	beq YM2413_0_DataW
	cmp addy,#0xF2				;@ FM/PSG enable/disable?
	beq SMSJSoundControlW
	b empty_W
;@----------------------------------------------------------------------------
SDSC_Debug_W:				;@ 0xFD
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}
	blx sdscHandler
	ldmfd sp!,{r3,lr}
	bx lr

;@----------------------------------------------------------------------------
Z80In:
;@----------------------------------------------------------------------------
	and addy,addy,#0xFF
	ldr pc,[pc,addy,lsl#2]
	.long 0
inTable:
	.space 0x400
;@----------------------------------------------------------------------------
Z80Out:
;@----------------------------------------------------------------------------
	and addy,addy,#0xFF
	ldr pc,[pc,addy,lsl#2]
	.long 0
outTable:
	.space 0x400
;@----------------------------------------------------------------------------
PPI1Reset:
	adr ppiptr,PPI_1
	b PPI8255Reset
;@----------------------------------------------------------------------------
PPI1R:
	mov r1,addy
	adr ppiptr,PPI_1
	b PPI8255R
;@----------------------------------------------------------------------------
PPI1PortAR:
	adr ppiptr,PPI_1
	b PPI8255PortAR
;@----------------------------------------------------------------------------
PPI1PortBR:
	adr ppiptr,PPI_1
	b PPI8255PortBR
;@----------------------------------------------------------------------------
PPI1PortCR:
	adr ppiptr,PPI_1
	b PPI8255PortCR
;@----------------------------------------------------------------------------
PPI1PortDR:
	adr ppiptr,PPI_1
	b PPI8255PortDR
;@----------------------------------------------------------------------------
PPI1W:
	mov r1,addy
	adr ppiptr,PPI_1
	b PPI8255W
;@----------------------------------------------------------------------------
PPI1PortAW:
	adr ppiptr,PPI_1
	b PPI8255PortAW
;@----------------------------------------------------------------------------
PPI1PortBW:
	adr ppiptr,PPI_1
	b PPI8255PortBW
;@----------------------------------------------------------------------------
PPI1PortCW:
	adr ppiptr,PPI_1
	b PPI8255PortCW
;@----------------------------------------------------------------------------
PPI1PortDW:
	adr ppiptr,PPI_1
	b PPI8255PortDW
;@----------------------------------------------------------------------------
PPI_1:
	.space ppiSize
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
