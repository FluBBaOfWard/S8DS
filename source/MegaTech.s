#ifdef __arm__

#include "ARMZ80/ARMZ80.i"
#include "PPI8255/PPI8255.i"
#include "CXD1095Q/CXD1095Q.i"

	.global megaTechReset
	.global MTIORead
	.global MTIOWrite
	.global MTVDPVCounterR
	.global MTVDPHCounterR
	.global MTVDPStatR
	.global MTVDPDataR
	.global MTVDPDataW
	.global MTVDPCtrlW

	.global mtIOSelectReg

	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
megaTechReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}

	bl CXD1Reset
	ldr r0,=dipSwitch0iR
	str r0,[cxdptr,#cxdPortAInFptr]
	ldr r0,=dipSwitch1iR
	str r0,[cxdptr,#cxdPortBInFptr]
	ldr r0,=MTCartSelectW
	str r0,[cxdptr,#cxdPortEOutFptr]

	bl CXD2Reset
	ldr r0,=MTUserInput0R
	str r0,[cxdptr,#cxdPortAInFptr]
	ldr r0,=MTUserInput1R
	str r0,[cxdptr,#cxdPortBInFptr]
	ldr r0,=MTCartSenseR
	str r0,[cxdptr,#cxdPortCInFptr]
	ldr r0,=MTInputCtrlW
	str r0,[cxdptr,#cxdPortDOutFptr]
	ldr r0,=MTFlashW
	str r0,[cxdptr,#cxdPortEOutFptr]

	ldr r1,=dipSwitch0
	mov r0,#0x80
	strb r0,[r1]
	mov r0,#0x0E
	strb r0,[r1,#1]

	ldmfd sp!,{r4,lr}
	bx lr

;@----------------------------------------------------------------------------
MTIORead:
;@----------------------------------------------------------------------------
	and r1,addy,#0xFC00
	cmp r1,#0x6000
	beq CXD0R
	cmp r1,#0x6400
	beq CXD1R
	cmp r1,#0x6800
	beq CXD2R
	cmp r1,#0x6C00
	beq CXD3R
	cmp r1,#0x7800
	beq MDIOR
	and r1,r1,#0xF000
	cmp r1,#0x3000
	beq BankedRAMR
	and r1,r1,#0xE000
	cmp r1,#0x8000
	beq MDGameR
	ldr r1,=cartBase
	ldr r1,[r1]
	ldrb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
MTIOWrite:
;@----------------------------------------------------------------------------
	and r1,addy,#0xFC00
	cmp r1,#0x6000
	beq CXD0W
	cmp r1,#0x6400
	beq CXD1W
	cmp r1,#0x6800
	beq CXD2W
	cmp r1,#0x6C00
	beq CXD3W
	cmp r1,#0x7800
	beq MDIOW
	and r1,r1,#0xF000
	cmp r1,#0x3000
	beq BankedRAMW
	ldr r1,=cartBase
	ldr r1,[r1]
	strb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
CXD0R:						;@ 0x6000
;@----------------------------------------------------------------------------
	mov r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
CXD0W:						;@ 0x6000, MD Z80 banking
;@----------------------------------------------------------------------------
//	mov r11,r11
								;@ Map 32kB of MD memory to Z80 0x8000.
								;@ 9 writes with bit #0 sets bit 15-23 of offset.
	bx lr
;@----------------------------------------------------------------------------
CXD3R:						;@ 0x6C00, reset CXD1?
;@----------------------------------------------------------------------------
//	mov r11,r11
	mov r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
CXD3W:						;@ 0x6C00, Coins inhibit/accepted.
;@----------------------------------------------------------------------------
//	mov r11,r11
								;@ Values 8-B written at startup & also 0-3 if Free Play.
								;@ Values 0, 1, 3 & 2 when those coin slots are inhibitet.
	bx lr
;@----------------------------------------------------------------------------
MDIOW:						;@ 0x7800 & 0x7801?
;@----------------------------------------------------------------------------
	mov r11,r11					;@ MegaDrive communication ports?
	bx lr
;@----------------------------------------------------------------------------
MDIOR:						;@ 0x7800 & 0x7801?
;@----------------------------------------------------------------------------
	mov r11,r11					;@ MegaDrive communication ports?
								;@ bit 0 & 1 tested on 7801, 7800 compared to 0x6
	mov r0,#0xFF
	tst addy,#1
	moveq r0,#0x06
	bx lr
;@----------------------------------------------------------------------------
MDGameR:
	bic addy,addy,r1
	mov addy,addy,lsr#1
	orr addy,addy,r1
	ldr r1,=cartBase
	ldr r1,[r1]
	ldrb r0,cartSelectReg
	and r0,r0,#0x7
	add r1,r1,r0,lsl#15		//32kB rom
	ldrb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
BankedRAMR:
	bic addy,addy,r1
	ldr r1,=EMU_SRAM
	ldrb r0,cartSelectReg
	and r0,r0,#0x7
	add r1,r1,r0,lsl#12		//4kB page
	ldrb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
BankedRAMW:
	bic addy,addy,r1
	ldr r1,=EMU_SRAM
	ldrb r2,cartSelectReg
	and r2,r2,#0x7
	add r1,r1,r2,lsl#12		//4kB page
	strb r0,[r1,addy]
	bx lr

;@----------------------------------------------------------------------------
MTCartSelectW:				;@ Select which cart (& ram bank?).
;@----------------------------------------------------------------------------
	strb r0,cartSelectReg
//	mov r11,r11
	bx lr
;@----------------------------------------------------------------------------
MTUserInput0R:				;@ Unknown inputs.
;@----------------------------------------------------------------------------
	ldr r1,=acExtra
	ldr r1,[r1]
	mov r0,#0xFF				;@ Default output
	tst r1,#0x4000				;@ NDS L
	bicne r0,r0,#0x01			;@ Game Select button
//	bicne r0,r0,#0x02			;@
//	bicne r0,r0,#0x04			;@
//	bicne r0,r0,#0x08			;@
	tst r1,#0x1000				;@ NDS L+Up
	biceq r0,r0,#0x10			;@ Door 1 (1 open, 0 close)
	tst r1,#0x0800				;@ NDS L+Left
	biceq r0,r0,#0x20			;@ Door 2 (1 open, 0 close)
//	bicne r0,r0,#0x40			;@
	tst r1,#0x2000				;@ NDS L+Down
	bicne r0,r0,#0x80			;@ Test button (service menu)
	bx lr
;@----------------------------------------------------------------------------
MTUserInput1R:				;@ Coins, service & start inputs.
;@----------------------------------------------------------------------------
	ldr r1,=acExtra
	ldr r1,[r1]
	mov r0,#0xFF				;@ Default output
	tst r1,#0x0200				;@ NDS Select
	bicne r0,r0,#0x01			;@ Coin 1
	tst r1,#0x0100				;@ NDS Select+Down
	bicne r0,r0,#0x02			;@ Coin 2
	tst r1,#0x0080				;@ NDS Select+Up
	bicne r0,r0,#0x04			;@ Coin 3
	tst r1,#0x0040				;@ NDS Select+Left
	bicne r0,r0,#0x08			;@ Coin 4
	tst r1,#0x0020				;@ NDS Select+Right
	bicne r0,r0,#0x10			;@ Service coin
	tst r1,#0x8000				;@ NDS R
	bicne r0,r0,#0x20			;@ Enter Button
	tst r1,#0x0010				;@ NDS Start
	bicne r0,r0,#0x40			;@ Start 1
	tst r1,#0x0008				;@ NDS Start+
	bicne r0,r0,#0x80			;@ Start 2
	bx lr
;@----------------------------------------------------------------------------
MTCartSenseR:				;@ Check which carts are present.
;@----------------------------------------------------------------------------
	mov r0,#0xFC			;@ Cart 0 inserted.
	bx lr
;@----------------------------------------------------------------------------
MTInputCtrlW:				;@ Enable/disable Joysticks?
;@----------------------------------------------------------------------------
					;@ bit 0 = Toggled several times after coin/(credit?) inserted.
					;@ bit 1 = 1 (Always ?)
					;@ bit 2 = Joystick enabled? (0=yes, 1=no) (Timer running.)
					;@ bit 3 = ? Set/reset depending on byte 0xD in game instruction rom (no games use it?).
					;@ bit 4 = Reset pin on Game side? (0=set, 1=not set)
					;@ bit 5 = Game/Instruction ROM mapping? (0=instruction, 1=game), MD SMS mode?
					;@ bit 6 = ? Sound off for Game side?
					;@ bit 7 = IO & Mem mapped to Game side? This input to IC14 pin19 (AM16L8 the adress decoder?).
	mov r11,r11
	strb r0,mtIOSelectReg
//	b r0OutputToEmulator
	bx lr
	;@ F6 46 F6 66 F6 F6 (F2) written at boot & exiting test menu (F2 if timer running).
	;@ 96 86 06 02 written when entering test menu
	;@ BIC 0x90 before writing SMS mapper regs, OR 0x90 after writing SMS mapper regs
	;@ BIC 0xB0 before reading instruction rom, OR 0xB0 after reading instruction rom
	;@ 82 written when testing alarm, 02 when turning off alarm.
	;@ F2 F3 F2 F3 F2 F3 F2 F3 F2 F3 F2 F3 F2 F3 F2 F3 F2 F3 F2 F3 F2
	;@ written after credit inserted
	;@ F6 96 86 06 86 .-. F6 written after time up
	;@ 66 F6 written when selecting game
;@----------------------------------------------------------------------------
MTFlashW:					;@ Flash screen.
;@----------------------------------------------------------------------------
					;@ bit 0 = Input. Checked after setting bit 3.
					;@ bit 1 = Flash screen? 0C & 0E written
					;@ bit 2 = SMS mode for game VDP (0=MD, 1=SMS)? Set/reset depending on byte 0xE & 0xF in game instruction ROM.
					;@ bit 3 = Set and then wait for bit 0 to be zero.
	mov r11,r11
//	b r0OutputToEmulator
//	tst r0,#0x4
//	adrne r0,smsString
//	adreq r0,mdString
//	b debugOutputToEmulator
	bx lr
	;@ 05 0C 04 04 0C written at boot
	;@ 04 0C written when entering test menu
	;@ 04 written after time up, 0C 04 0C 04
	;@ 0C 0C written after credit inserted
;@----------------------------------------------------------------------------
cartSelectReg:
	.byte 0
mtIOSelectReg:
	.byte 0
	.space 2
smsString:
	.string "SMS\n"
mdString:
	.string "MD\n"

	.align 2
;@----------------------------------------------------------------------------
MTVDPVCounterR:
;@----------------------------------------------------------------------------
	ldrb r12,mtIOSelectReg
	tst r12,#0x80
	bne VDP0VCounterR
	b VDP1VCounterR
;@----------------------------------------------------------------------------
MTVDPHCounterR:
;@----------------------------------------------------------------------------
	ldrb r12,mtIOSelectReg
	tst r12,#0x80
	bne VDP0HCounterR
	b VDP1HCounterR
;@----------------------------------------------------------------------------
MTVDPStatR:
;@----------------------------------------------------------------------------
	ldrb r12,mtIOSelectReg
	tst r12,#0x80
	bne VDP0StatR
	b VDP1StatR
;@----------------------------------------------------------------------------
MTVDPDataR:
;@----------------------------------------------------------------------------
	ldrb r12,mtIOSelectReg
	tst r12,#0x80
	bne VDP0DataR
	b VDP1DataR
;@----------------------------------------------------------------------------
MTVDPDataW:
;@----------------------------------------------------------------------------
	ldrb r12,mtIOSelectReg
	tst r12,#0x80
	bne VDP0DataSMSW
	b VDP1DataSMSW
;@----------------------------------------------------------------------------
MTVDPCtrlW:
;@----------------------------------------------------------------------------
	ldrb r12,mtIOSelectReg
	tst r12,#0x80
	bne VDP0CtrlW
	b VDP1CtrlW


;@----------------------------------------------------------------------------
CXD1Reset:
	adr cxdptr,CXD1
	b CXD1095QReset
;@----------------------------------------------------------------------------
CXD1R:
	mov r1,addy
	adr cxdptr,CXD1
	b CXD1095QR
;@----------------------------------------------------------------------------
CXD1W:
	mov r1,addy
	adr cxdptr,CXD1
	b CXD1095QW
;@----------------------------------------------------------------------------
CXD2Reset:
	adr cxdptr,CXD2
	b CXD1095QReset
;@----------------------------------------------------------------------------
CXD2R:
	mov r1,addy
	adr cxdptr,CXD2
	b CXD1095QR
;@----------------------------------------------------------------------------
CXD2W:
	mov r1,addy
	adr cxdptr,CXD2
	b CXD1095QW
;@----------------------------------------------------------------------------

CXD1:
	.space cxdSize
CXD2:
	.space cxdSize
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
