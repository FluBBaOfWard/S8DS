#ifdef __arm__

#include "Equates.h"
#include "ARMZ80/ARMZ80mac.h"

	.global empty_R
	.global empty_R_SMS1
	.global empty_W
	.global ram_W
	.global ram0_W
	.global ram1_W
	.global ram1S_W
	.global ram1k_W
	.global ram2k_W
	.global ram4k_W
	.global ram16k_W
	.global sram0_W
	.global sram1_W
	.global rom_R
	.global Korean_W
	.global Korean_MSX_W
	.global Codemaster_W
	.global rom_W
	.global biosWrite
	.global cart1Write
	.global cart2Write
	.global memcpy_

	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
empty_R:					;@ Read bad (IO) address, error.
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0xFF				;@ SMS2, GG & MD
	bx lr
;@----------------------------------------------------------------------------
empty_R_SMS1:				;@ Read bad (IO) address, error.
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	ldrb r0,[r9,#-1]			;@ SMS (z80pc-1)
	bx lr
;@----------------------------------------------------------------------------
empty_W:					;@ Write bad address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA debugg
	mov r0,#0xBA
	bx lr
;@----------------------------------------------------------------------------
Korean_MSX_W:				;@ Write ROM address (bankswitch)
;@----------------------------------------------------------------------------
	cmp addy,#0x0002
	beq BankSwitch0MSX_W
	cmp addy,#0x0003
	beq BankSwitch1MSX_W
	cmp addy,#0x0000
	beq BankSwitch2MSX_W
	cmp addy,#0x0001
	beq BankSwitch3MSX_W
	b rom_W
;@----------------------------------------------------------------------------
Korean_W:					;@ Write ROM address (bankswitch)
;@----------------------------------------------------------------------------
	cmp addy,#0xA000
	beq BankSwitch2_W
	b rom_W
;@----------------------------------------------------------------------------
Codemaster_W:				;@ Write ROM address (bankswitch)
;@----------------------------------------------------------------------------
	cmp addy,#0x8000
	beq BankSwitch2C_W
	cmp addy,#0x4000
	beq BankSwitch1C_W
	cmp addy,#0x0000
	beq BankSwitch0C_W
;@----------------------------------------------------------------------------
rom_W:						;@ Write ROM address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA debugg
	mov r0,#0xB0
	bx lr
;@----------------------------------------------------------------------------
biosWrite:					;@ Write Bios address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA debugg
	ldr r0,=0xB105
	bx lr
;@----------------------------------------------------------------------------
cart1Write:					;@ Write Cart1 address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA debugg
	ldr r0,=0xCA271
	bx lr
;@----------------------------------------------------------------------------
cart2Write:					;@ Write Cart2 address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA debugg
	ldr r0,=0xCA272
	bx lr
;@----------------------------------------------------------------------------

	.section .itcm
	.align 2
;@----------------------------------------------------------------------------
ram_W:						;@ Write ram ($0000-$FFFF)
;@----------------------------------------------------------------------------
//	ldr r2,=EMU_RAM
//	strb r0,[r2,addy]
	and r1,addy,#MEM_BANK_MASK
	add r2,z80optbl,#z80MemTbl
	ldr r1,[r2,r1,lsr#MEM_BANK_SHIFT]
	strb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
ram0_W:						;@ Write ram ($C000-$DFFF)
;@----------------------------------------------------------------------------
	ldr r1,=EMU_RAM-0xC000
	strb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
ram1_W:						;@ Write ram ($E000-$FFFF), mirror.
;@----------------------------------------------------------------------------
	ldr r1,=BankMap4
	ldrb r1,[r1]
	tst r1,#0x08				;@ BIOS?
	ldrne r1,=EMU_RAM-0xE000	;@ Skip forward to write with BankSwitch.
	strbne r0,[r1,addy]
	bxne lr
;@----------------------------------------------------------------------------
ram1S_W:					;@ Write ram ($E000-$FFFF), mirror. Bankswitch
;@----------------------------------------------------------------------------
	ldr r1,=EMU_RAM-0xE000
	strb r0,[r1,addy]
	mov r1,#0x00080000
	adds r1,r1,addy,lsl#16
	bxmi lr

	subs r1,r1,#0x00040000
	bmi LCDGlasses
	beq BankSwitchR_W			;@ $FFFC
	cmp r1,#0x20000
	bhi BankSwitch2_W			;@ $FFFF
	beq BankSwitch1_W			;@ $FFFE
	bmi BankSwitch0_W			;@ $FFFD
	mov r11,r11
	bx lr
;@----------------------------------------------------------------------------
LCDGlasses:
;@----------------------------------------------------------------------------
	ldr r1,=g_machine
	ldrb r1,[r1]
	cmp r1,#HW_MARK3
	cmpne r1,#HW_SMS1
	cmpne r1,#HW_MEGADRIVE
	bxne lr
	ldr r1,=g_3DEnable
	ldrb r1,[r1]
	cmp r1,#0
	bxeq lr
	tst r0,#1
	ldreq r0,=0x7FE0
	movne r0,#0x001F
	ldr r1,=paletteMask
	str r0,[r1]
	bx lr
;@----------------------------------------------------------------------------
ram1k_W:					;@ Write ram (SG-1000 $C000-$FFFF,Coleco $6000-$7FFF), mirrored every 1k.
;@----------------------------------------------------------------------------
	bic r2,addy,#0xFC00
	ldr r1,=EMU_RAM
	strb r0,[r1,r2]
	bx lr
;@----------------------------------------------------------------------------
ram2k_W:					;@ Write ram (SC-3000 $C000-$FFFF), mirrored every 2k.
;@----------------------------------------------------------------------------
	bic r2,addy,#0xF800
	ldr r1,=EMU_RAM
	strb r0,[r1,r2]
	bx lr
;@----------------------------------------------------------------------------
ram4k_W:					;@ Write ram (Sord M5 $7000-$7FFF)
;@----------------------------------------------------------------------------
	tst addy,#0x1000
	bic r2,addy,#0xF000
	ldr r1,=EMU_RAM
	strbne r0,[r1,r2]
	bx lr
;@----------------------------------------------------------------------------
ram16k_W:					;@ Write ram (SYS-E $C000-$FFFF), no mirroring.
;@----------------------------------------------------------------------------
	bic r2,addy,#0xC000
	ldr r1,=EMU_RAM
	strb r0,[r1,r2]
//	mov r1,#0x00030000			;@ This is just a test!!!
//	adds r1,r1,addy,lsl#16
//	bxmi lr

//	cmp r1,#0x10000
//	bhi BankSwitch2_W			;@ $FFFF
//	beq BankSwitch1_W			;@ $FFFE
//	bmi BankSwitch0_W			;@ $FFFD
	bx lr
;@----------------------------------------------------------------------------
sram0_W:					;@ Write sram bank0 ($8000-$BFFF)
;@----------------------------------------------------------------------------
	ldr r1,=EMU_SRAM-0x8000
	strb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
sram1_W:					;@ Write sram bank1 ($8000-$BFFF),($C000-$FFFF)
;@----------------------------------------------------------------------------
	ldr r1,=EMU_SRAM-0x4000
	strb r0,[r1,addy]
	bx lr

;@----------------------------------------------------------------------------
rom_R:						;@ Rom read
;@----------------------------------------------------------------------------
	and r1,addy,#MEM_BANK_MASK
	add r2,z80optbl,#z80MemTbl
	ldr r1,[r2,r1,lsr#MEM_BANK_SHIFT]
	ldrb r0,[r1,addy]
	bx lr
;@----------------------------------------------------------------------------
memcpy_:						;@ r0=dest r1=src r2=word count
;@	Exit with r0 & r1 unchanged, r2=0, r3 trashed
;@----------------------------------------------------------------------------
	subs r2,r2,#1
	ldrpl r3,[r1,r2,lsl#2]
	strpl r3,[r0,r2,lsl#2]
	bhi memcpy_
	bx lr
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
