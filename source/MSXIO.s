#ifdef __arm__

#include "AY38910/AY38910.i"
#include "PPI8255/PPI8255.i"

	.global msxReset
	.global IO_Params_MSX_R
	.global IO_Params_MSX_W
	.global MSXJoyReadCallBack

	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
;@ MSX
IO_Params_MSX_R:
	.long 0x009800FF, VDP0DataR			;@ 0x98 VDP Data
	.long 0x009900FF, VDP0StatR			;@ 0x99 VDP Ctrl
	.long 0x00A200FF, AY38910Data_0_R	;@ 0xA2 AY38910DataR
	.long 0x00A800FF, PPI1PortAR		;@ 0xA8 PPI Port A, PSLOT_R
	.long 0x00A900FF, PPI1PortBR		;@ 0xA9 PPI Port B, Keyboard_R
	.long 0x00AA00FF, PPI1PortCR		;@ 0xAA PPI Port C, Keyb_Row_W
	.long 0x00AB00FF, PPI1PortDR		;@ 0xAB PPI Ctrl_R
//	.long 0x00FC00FF, empty_R			;@ 0xFC RAM slot for $0000
//	.long 0x00FD00FF, empty_R			;@ 0xFD RAM slot for $4000
//	.long 0x00FE00FF, empty_R			;@ 0xFE RAM slot for $8000
//	.long 0x00FF00FF, empty_R			;@ 0xFF RAM slot for $C000
	.long 0x00000000, empty_R			;@ everything else
IO_Params_MSX_W:
//	.long 0x009000FF, empty_W			;@ 0x90 Cent_W
	.long 0x009800FF, VDP0DataTMSW		;@ 0x98 VDP Data
	.long 0x009900FF, VDP0CtrlW			;@ 0x99 VDP Ctrl
//	.long 0x009A00FF, VDP0DataTMSW		;@ 0x9A MVDP MSX2
//	.long 0x009B00FF, VDP0CtrlW			;@ 0x9B MVDP MSX2
	.long 0x00A000FF, AY38910Index_0_W	;@ 0xA0 AY38910IndexW
	.long 0x00A100FF, AY38910Data_0_W	;@ 0xA1 AY38910DataW
	.long 0x00A800FF, PPI1PortAW		;@ 0xA8 PPI Port A, PSlot_W
	.long 0x00A900FF, PPI1PortBW		;@ 0xA9 PPI Port B
	.long 0x00AA00FF, PPI1PortCW		;@ 0xAA PPI Port C, Keyb_Row_W
	.long 0x00AB00FF, PPI1PortDW		;@ 0xAB PPI Ctrl_W
	.long 0x00FC00FF, MSXMapRAM0W		;@ 0xFC RAM slot for $0000
	.long 0x00FD00FF, MSXMapRAM1W		;@ 0xFD RAM slot for $4000
	.long 0x00FE00FF, MSXMapRAM2W		;@ 0xFE RAM slot for $8000
	.long 0x00FF00FF, MSXMapRAM3W		;@ 0xFF RAM slot for $C000
	.long 0x00000000, empty_W			;@ everything else

;@----------------------------------------------------------------------------
msxReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}

	bl PPI1Reset
	ldr r0,=PSlot_W
	str r0,[ppiptr,#ppiPortAOutFptr]
	ldr r0,=IOPortB_MSX_R
	str r0,[ppiptr,#ppiPortBInFptr]

	ldmfd sp!,{r4,lr}
	bx lr

;@----------------------------------------------------------------------------
IOPortB_MSX_R:				;@ KB/Printer/Cassette input, PPI_PortB
;@----------------------------------------------------------------------------
	mov r0,#0xFF				;@ Default output
	ldrb r1,[ppiptr,#ppiPortCOut]
	and r1,r1,#0xF
	ldr r2,=sc3Keyboard
	ldrb r2,[r2]
	cmp r1,r2,lsr#4				;@ check row
	bxne lr
	and r2,r2,#0x07
	mov r1,#1
	bic r0,r0,r1,lsl r2
	bx lr
;@----------------------------------------------------------------------------
PSlot_W:					;@ MSX PSLOT, PPI_PortA
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}
	ldr r1,=BankMap0
	ldrb r3,[r1]
	strb r0,[r1]
	eor r3,r3,r0				;@ Which Banks are changed?
	tst r3,#0x03
	blne BankMSX0_W
	tst r3,#0x0C
	blne BankMSX1_W
	tst r3,#0x30
	blne BankMSX2_W
	tst r3,#0xC0
	blne BankMSX3_W
	ldmfd sp!,{r3,lr}
	bx lr
;@----------------------------------------------------------------------------
MSXJoyReadCallBack:			;@ r0=ayptr
;@----------------------------------------------------------------------------
	ldrb r1,[r0,#ayPortBOut]
	tst r1,#0x40
	ldreq r0,=joy0State
	ldrne r0,=joy1State
	ldrb r0,[r0]
	bx lr

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
