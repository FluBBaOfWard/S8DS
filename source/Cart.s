#ifdef __arm__

#include "Equates.h"
#include "ARMZ80/ARMZ80mac.h"
#include "SegaVDP/SegaVDP.i"

	.global loadCart
	.global ejectCart
	.global cartSaveState
	.global cartLoadState
	.global cartGetStateSize
	.global g_emuFlags
	.global cartBase
//	.global scaling
	.global g_scalingSet
	.global g_cartFlags
	.global g_machine
	.global g_machineSet
	.global g_config
	.global g_configSet
	.global g_region
	.global gArcadeGameSet
	.global BankMap0
	.global BankMap4
	.global g_dipSwitch0
	.global g_dipSwitch1
	.global g_dipSwitch2
	
	.global BankSwitchR_W
	.global BankSwitch0_W
	.global BankSwitch1_W
	.global BankSwitch2_W
	.global BankSwitch0C_W
	.global BankSwitch1C_W
	.global BankSwitch2C_W

	.global BankMSX0_W
	.global BankMSX1_W
	.global BankMSX2_W
	.global BankMSX3_W
	.global MSXMapRAM0W
	.global MSXMapRAM1W
	.global MSXMapRAM2W
	.global MSXMapRAM3W
/*
	.global reBankSwitchB_W
	.global reBankSwitchR_W
	.global reBankSwitch0_W
	.global reBankSwitch1_W
	.global reBankSwitch2_W
	.global reBankSwitch0C_W
*/
	.global BankSwitch0MSX_W
	.global BankSwitch1MSX_W
	.global BankSwitch2MSX_W
	.global BankSwitch3MSX_W

	.global BankSwitchB_GG_W
	.global MemCtrl_SMS_W
	.global MemCtrl_MD_W

	.global EMU_RAM
	.global EMU_SRAM
	.global ROM_Space
	.global BIOS_US_Space
	.global BIOS_JP_Space
	.global BIOS_GG_Space
	.global BIOS_COLECO_Space
	.global BIOS_MSX_Space
	.global BIOS_SORDM5_Space
	.global g_BIOSBASE_US
	.global g_BIOSBASE_JP
	.global g_BIOSBASE_GG
	.global g_BIOSBASE_COLECO
	.global g_BIOSBASE_MSX
	.global g_BIOSBASE_SORDM5
	.global g_ROM_Size


	.syntax unified
	.arm

	.section .rodata
	.align 2

rawRom:
//	.incbin "sms/Ace of Aces (UE) [!].sms"
//	.incbin "sms/Action Fighter (TW).sms"
//	.incbin "sms/After Burner (UE).sms"
//	.incbin "sms/Aladdin (UE) [!].sms"
//	.incbin "sms/Alex Kidd in Miracle World [v1].sms"
//	.incbin "sms/Back to the Future 2 (UE) [!].sms"
//	.incbin "sms/Back to the Future 3 (E) [!].sms"
//	.incbin "sms/Battlemaniacs (BR) [!].sms"
//	.incbin "sms/Cosmic Spacehead (UE) [!].sms"
//	.incbin "sms/Excellent Dizzy Collection, The [SMS-GG].sms"
//	.incbin "sms/Fantastic Dizzy [SMS-GG].sms"
//	.incbin "sms/Fantastic Dizzy.sms"
//	.incbin "sms/Fire & Forget 2 (UE) [!].sms"
//	.incbin "sms/FM_Tool.sms"
//	.incbin "sms/Gauntlet (UE) [!].sms"
//	.incbin "sms/Jang Pung II [SMS-GG] (KR).sms"
//	.incbin "sms/Master of Darkness (UE) [!].sms"
//	.incbin "sms/MD6ButtonTest.sms"
//	.incbin "sms/Out Run.sms"
//	.incbin "sms/PGA Tour Golf (UE) [!].sms"
//	.incbin "sms/Phantasy Star (JP).sms"
//	.incbin "sms/Phantasy Star [v3].sms"
//	.incbin "sms/Pit Fighter (UE) [!].sms"
//	.incbin "sms/Robocop Vs. Terminator (UE) [!].sms"
//	.incbin "sms/Running Battle (UE) [!].sms"
//	.incbin "sms/Sonic the Hedgehog (UE) [!].sms"
//	.incbin "sms/Super Tetris (KR).sms"
//	.incbin "sms/Terminator, The (UE) [!].sms"
//	.incbin "sms/VDPTest.sms"
//	.incbin "sms/Won-Si-In (KR).sms"
//	.incbin "sms/Xenon 2 (Image Works) (UE) [!].sms"
//	.incbin "sms/Zaxxon 3D (UE) [!].sms"
//	.incbin "sms/zexall.sms"
//	.incbin "sms/zexdoc.sms"
//	.incbin "sms/zexdoc_sdsc.sms"
//	.incbin "gg/Addams Family, The (UE) [!].gg"
//	.incbin "gg/Aerial Assault [v0].gg"
//	.incbin "gg/Aerial Assault [v1].gg"
//	.incbin "gg/Barbie Super Model [Proto].gg"
//	.incbin "gg/Battletoads (UE).gg"
//	.incbin "gg/Moldorian (JP).gg"
//	.incbin "gg/Urban Strike (U) [!].gg"
//	.incbin "gg/X-Men - Mojo World (UE) [!].gg"
//	.incbin "gg/Terminator 2 - Judgement Day (JUE).gg"
//	.incbin "gg/twisty.gg"
//	.incbin "gg/WildSnake [Proto].gg"
//	.incbin "sg/Bank Panic.sg"
//	.incbin "sg/Bomb Jack.sg"
//	.incbin "sg/Castle, The (JP).sg"
//	.incbin "sg/Congo Bongo [B].sg"
//	.incbin "sg/Othello (JP).sg"
//	.incbin "sg/Othello Multivision [BIOS].sg"
//	.incbin "sg/Q-Bert.sg"
//	.incbin "sc/Sega Basic Level 2 (JP).sc"
//	.incbin "sg/SG-1000 M2 Check Program.sg"
//	.incbin "col/Amazing Bumpman (1986) (Telegames).rom"
//	.incbin "col/BC's Quest for Tires II - Grog's Revenge (1984) (32k).rom"
//	.incbin "col/Burgertime (1982-84) (Data East).rom"
//	.incbin "col/Chuck Norris - Super Kicks (1983) (Xonox).rom"
//	.incbin "col/Cosmic Avenger (1982) (Universal).rom"
//	.incbin "col/River Raid (1982-84) (Activision) [!].rom"
//	.incbin "col/Smurf - Paint 'n Play Workshop (1983).col"
//	.incbin "col/Spy Hunter (1983-84) (Midway).rom"
//	.incbin "msx/Bokosuka Wars (1984)(Ascii Corp).rom"
//	.incbin "msx/Bosconian (1981-84)(Namco Ltd.)(Jp)[a].rom"
//	.incbin "msx/Bruce Lee (1984)(Datasoft).rom"
//	.incbin "msx/Goonies, The (1986)(Konami)(Jp)[RC-734].rom"
//	.incbin "msx/King's Valley (1985)(Konami)(Jp)[a][RC-727].rom"
//	.incbin "msx/King's Valley II (1988)(Konami)(Jp)[RC-760][h MSX1].rom"
//	.incbin "msx/Knight Mare (1986)(Konami)(Jp)[a][RC-739].rom"
//	.incbin "msx/Nemesis (1986)(Konami)(Jp)[RC-742].rom"
//	.incbin "msx/Nemesis II (1987)(Konami)(Jp)[RC-751].rom"
//	.incbin "msx/Nemesis III - The Eve Of Destruction (1988)(Konami)(Jp)[RC-764].rom"
//	.incbin "msx/Parodius (1988)(Konami)(Jp)[RC-759].rom"
//	.incbin "SordM5/basic-i.rom"
//	.incbin "SordM5/bosconian.rom"
//	.incbin "SordM5/digdug.rom"
//	.incbin "SordM5/mappy.rom"
//	.incbin "SordM5/poo-yan.rom"
//	.incbin "ac/epr-7356.ic1"				// Doki Doki Penguin Arcade
//	.incbin "ac/epr-7357.ic2"
//	.incbin "ac/epr-7358.ic3"
//	.incbin "ac/astroflash.sms"				// Encrypted, doesn't work
//	.incbin "ac/transformer.sms"
//	.incbin "ac/hangonjr/rom5.ic7"			// Hang On Jr Arcade
//	.incbin "ac/hangonjr/rom4.ic5"
//	.incbin "ac/hangonjr/rom3.ic4"
//	.incbin "ac/hangonjr/rom2.ic3"
//	.incbin "ac/hangonjr/rom1.ic2"
//	.incbin "ac/tetrisse/epr12213.7"		// Tetris SE
//	.incbin "ac/tetrisse/epr12212.5"
//	.incbin "ac/tetrisse/epr12211.4"
//	.incbin "ac/transfrm/ic7.top"			// Transformer
//	.incbin "ac/transfrm/epr-7347.ic5"
//	.incbin "ac/transfrm/epr-7348.ic4"
//	.incbin "ac/transfrm/ic3.top"
//	.incbin "ac/transfrm/epr-7350.ic2"
//	.incbin "ac/ridleofp.sms"
//	.incbin "mt/epr12664.20"
endRom:
//	.incbin "mt/epr-12368-01.ic2"
//	.incbin "mt/epr-12368-06.ic2"
//	.incbin "mt/epr-12368-07.ic2"
//	.incbin "mt/epr-12368-10.ic2"
//	.incbin "mt/epr-12368-25.ic2"
//	.incbin "mt/epr-12368-41.ic2"
//	.incbin "mt/epr-12368-44.ic2"
//	.incbin "mt/epr-12368-47.ic2"
//	.incbin "mt/epr-12368-49.ic2"
rawBios:
//	.incbin "SMS BIOS (US v1.3).sms"
//	.incbin "SMS (v2.0) [BIOS].sms"
//	.incbin "SMS BIOS (JP).sms"
//	.incbin "COLECO.ROM"
//	.incbin "MSX BIOS.rom"
//	.incbin "SordM5/sordint.ic21"
miniBios:
	.incbin "MiniBios.sms"
mdBios:
	.byte 0x21, 0x01, 0xE1	;@ LD HL, $E101
	.byte 0x25				;@ DEC H
	.byte 0xF9				;@ LD SP,HL
	.byte 0xC7				;@ RST $00
	.byte 0x01, 0x01		;@ dummy data

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
loadCart: 		;@ Called from C:  r0=emuFlags
	.type   loadCart STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}

	ldr z80optbl,=Z80OpTable

//	ldr r3,=rawBios
//	str r3,g_BIOSBASE_US
//	str r3,g_BIOSBASE_JP
//	str r3,g_BIOSBASE_COLECO
//	str r3,g_BIOSBASE_MSX
//	str r3,g_BIOSBASE_SORDM5
//	ldr r3,=rawRom
	ldr r3,=ROM_Space

//	orr r0,r0,#GG_MODE
//	orr r0,r0,#SG_MODE
//	orr r0,r0,#COL_MODE
//	orr r0,r0,#MSX_MODE
//	orr r0,r0,#SORDM5_MODE
//	orr r0,r0,#SGAC_MODE
//	orr r0,r0,#SYSE_MODE
	str r0,g_emuFlags

	ldrb r0,g_machineSet
//	mov r0,#HW_MSX
//	mov r0,#HW_SORDM5
//	mov r0,#HW_SGAC
//	mov r0,#HW_SYSE
//	mov r0,#HW_MEGATECH
	strb r0,g_machine
	ldrb r0,g_configSet
	strb r0,g_config
								;@ r3=romBase til end of loadCart so DON'T FUCK IT UP
	str r3,romBase				;@ Set romBase
	str r3,cartBase				;@ Set cartBase

//	mov r0,#endRom-rawRom
//	str r0,g_ROM_Size
	ldr r0,g_ROM_Size
	bl romSizeToPowerOf2
	movs r2,r0,lsr#13			;@ Rom size in 8k banks
	subne r2,r2,#1
	str r2,romMask				;@ romMask=romsize-1
	str r2,romMaskBackup		;@ romMask=romsize-1

	mov r1,r0
	mov r0,r3
	bl mapperDetect
	mov r5,r0

	ldr r1,=0x02010000
	str r1,BankMap_Bios

	cmp r2,#0x4					;@ 32kB
	ldrmi r1,=0x02010008		;@ RAM for SG1000 games
	cmp r5,#1
	ldreq r1,=0x00010000		;@ Codemaster...
	str r1,BankMap_Cart
	str r1,BankMap0

	ldr r4,=MEMMAPTBL_
	ldr r9,=RDMEMTBL_
	ldr r6,=WRMEMTBL_
	ldr r7,=rom_R
	ldrmi r8,=rom_W
	ldreq r8,=Codemaster_W
	ldrhi r8,=Korean_W
	cmp r5,#3
	ldreq r8,=Korean_MSX_W
	mov r0,#0
tbLoop1:
	and r1,r0,r2
	add r1,r3,r1,lsl#14
	str r1,[r4,r0,lsl#2]
	str r7,[r9,r0,lsl#2]
	str r8,[r6,r0,lsl#2]
	add r0,r0,#1
	cmp r0,#8
	bne tbLoop1

	bl fillROMBANKMAP
	bl checkMachine				;@ Returns HW_Machine in r0.
	mov r9,r0

	cmp r9,#HW_SYSE
	ldreq r0,=SystemEVDPRAMWrite
	streq r0,[r6]

	ldr r0,g_emuFlags
	ldrb r2,g_config
	mov r1,#0
	cmp r9,#HW_SMS1
	cmpne r9,#HW_SMS2
	ldreq r1,g_BIOSBASE_US		;@ SMS US
	tst r0,#COUNTRY
	ldrne r1,g_BIOSBASE_JP		;@ SMS JP
	cmp r9,#HW_COLECO
	ldreq r1,g_BIOSBASE_COLECO	;@ COLECO
	cmp r9,#HW_MSX
	ldreq r1,g_BIOSBASE_MSX		;@ MSX
	cmp r9,#HW_SORDM5
	ldreq r1,g_BIOSBASE_SORDM5	;@ SORD M5
	cmp r9,#HW_GG
	ldreq r1,g_BIOSBASE_GG		;@ GG
	bicne r2,r2,#0x40			;@ X as Start/Pause only on HW_GG
	bicne r0,r0,#GG_MODE
	cmpne r9,#HW_SMS2
	cmpne r9,#HW_MEGADRIVE
	biceq r2,r2,#0x20			;@ Reset unavailable on HW_GG, HW_SMS2 & HW_MEGADRIVE
	str r0,g_emuFlags
	strb r2,g_config
	cmp r1,#0
	ldreq r1,=miniBios
	str r1,biosBase
	mov r0,#7					;@ 7 BIOS
	str r1,[r4,r0,lsl#2]		;@ MemMap
	str r8,[r6,r0,lsl#2]		;@ WrMem

	ldr r8,=ram0_W
	cmp r9,#HW_SYSE
	ldreq r8,=ram16k_W			;@ 16k RAM no mirroring
	cmp r9,#HW_MSX
	ldreq r8,=ram_W				;@ 64k RAM
	cmp r9,#HW_SG1000
	cmpne r9,#HW_SGAC
	cmpne r9,#HW_SG1000II
	ldreq r8,=ram1k_W			;@ 1k RAM mirrored
	cmp r9,#HW_SC3000
	cmpne r9,#HW_OMV
	ldreq r8,=ram2k_W			;@ 2k RAM mirrored
	ldr r1,=EMU_RAM
	mov r0,#3					;@ 3 RAM
	str r1,[r4,r0,lsl#2]		;@ MemMap
	str r8,[r6,r0,lsl#2]		;@ WrMem

	ble isSGRam
	cmp r9,#HW_SYSE
	beq isSGRam
	ldr r8,=ram1_W				;@ RAM without bankswitch
	cmp r5,#0
	bne isSGRam
	ldr r0,romMask
	cmp r0,#4
	ldrpl r8,=ram1S_W			;@ RAM with bank switch
isSGRam:
	mov r0,#4					;@ 4 RAM mirror
	str r1,[r4,r0,lsl#2]		;@ MemMap
	str r8,[r6,r0,lsl#2]		;@ WrMem

	ldr r1,=EMU_SRAM-0x8000
	ldr r8,=sram0_W				;@ 5 SRAM
	mov r0,#5					;@ 5 SRAM page 0
	str r1,[r4,r0,lsl#2]		;@ MemMap
	str r8,[r6,r0,lsl#2]		;@ WrMem

	ldr r1,=EMU_SRAM-0x4000
	ldr r8,=sram1_W				;@ 6 SRAM
	mov r0,#6					;@ 6 SRAM page 1
	str r1,[r4,r0,lsl#2]		;@ MemMap
	str r8,[r6,r0,lsl#2]		;@ WrMem


	ldrb r1,g_config
	tst r1,#0x80				;@ BIOS on/off
	cmpne r9,#HW_SG1000
	cmpne r9,#HW_SGAC
	cmpne r9,#HW_OMV			;@ ?
	cmpne r9,#HW_SG1000II
	cmpne r9,#HW_SC3000
	cmpne r9,#HW_MARK3
	cmpne r9,#HW_MEGADRIVE		;@ MegaDrive "bios" is handled later
	cmpne r9,#HW_SYSE
	cmpne r9,#HW_MEGATECH
	moveq r4,#0xAB				;@ Map in cartridge
	movne r4,#0xE3				;@ Map in BIOS

	strb r4,BankMap4			;@ Address bus chipselects.
	bl initMapper
	bl reBankSwitch0_W
	bl reBankSwitch1_W
	bl reBankSwitch2_W
	bl reBankSwitchB_W

	ldrb r9,g_machine
	cmp r9,#HW_COLECO
	bleq WRAMColeco
	cmp r9,#HW_MSX
	bleq initMSXMemory
	cmp r9,#HW_SORDM5
	bleq initSordM5Memory
//	cmp r9,#HW_MEGATECH
//	bleq initMTMemory

	bl memReset
noMemClear:
	bl gfxReset
	bl ioReset
	bl soundReset
	bl cpuReset
	cmp r9,#HW_MEGADRIVE
	bleq setupMDBios			;@ This needs to run after cpu reset, enables MD Bios without bankswitch
	ldmfd sp!,{r4-r11,lr}
	bx lr

;@----------------------------------------------------------------------------
ejectCart:
	.type   ejectCart STT_FUNC
;@----------------------------------------------------------------------------
	mov r2,#0x100000
	str r2,g_ROM_Size
	movs r1,r2,lsr#13			;@ Rom size in 8k banks
	subne r1,r1,#1
	str r1,romMask				;@ romMask=romsize-1
	str r1,romMaskBackup		;@ romMask=romsize-1

	ldr r0,=ROM_Space
	mov r1,#-1
	mov r2,#0x100000/4
	b memset_

;@----------------------------------------------------------------------------
romSizeToPowerOf2:				;@ r0 = in & out
;@----------------------------------------------------------------------------
//	cmp r0,#0x006000			;@ Mahjong, SG-1000 fix
//	moveq r0,#0x008000			;@ -||-
//	cmp r0,#0x00A000			;@ Monaco GP, SG-1000 fix
//	moveq r0,#0x010000			;@ -||-
//	cmp r0,#0x00C000			;@ Home Mahjong, SG-1000 fix
//	moveq r0,#0x010000			;@ -||-
//	cmp r0,#0x028000			;@ Transformers SYSE fix
//	moveq r0,#0x040000			;@ -||-
//	cmp r0,#0x018000			;@ Tetris SYSE fix
//	moveq r0,#0x020000			;@ -||-
//	cmp r0,#0x06C000			;@ Dodgeball fix
//	moveq r0,#0x080000			;@ -||-
//	cmp r0,#0x090000			;@ SF2 fix
//	movhi r0,#0x100000			;@ -||-

	sub r0,r0,#1
	orr r0,r0,r0,lsr#1
	orr r0,r0,r0,lsr#2
	orr r0,r0,r0,lsr#4
	orr r0,r0,r0,lsr#8
	orr r0,r0,r0,lsr#16
	add r0,r0,#1
	bx lr

;@----------------------------------------------------------------------------
memReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r9,r11,lr}

	ldr r0,=DISABLEDMEM			;@ Reset disabled memory
	mov r1,#-1
	mov r2,#0x400/4
	bl memset_

	ldr r0,=WRAP_RAM			;@ Fix for "Aerial Assault [v0].gg"
	ldr r1,=0xC7C7C7C7
	mov r2,#8/4
	bl memset_

	ldr r0,=EMU_SRAM			;@ Clear SRAM
	mov r1,#0
	mov r2,#0x8000/4
	bl memset_

	ldrb r11,g_machineSet
	cmp r11,#HW_AUTO
	moveq r6,#0
	movne r6,#-1
	mov r2,#0
	mov r3,r2
	mov r7,r6
	stmfd sp!,{r2,r3,r6,r7}
	ldmfd sp!,{r4,r5,r8,r9}
	ldr r0,=EMU_RAM				;@ Clear SMS RAM.
	cmp r11,#HW_SMS2
	beq WRAM_SMS2
	mov r1,#0x2000/64
wramLoop0:
	subs r1,r1,#1
	stmiapl r0!,{r2-r5}
	stmiapl r0!,{r2-r5}
	stmiapl r0!,{r6-r9}
	stmiapl r0!,{r6-r9}
	bhi wramLoop0

	ldmfd sp!,{r4-r9,r11,lr}
	bx lr

WRAM_SMS2:
	mov r2,#0x2000/0x100
wramLoop2:
	mov r1,#0x100/32
wramLoop1:
	subs r1,r1,#1
	stmiapl r0!,{r6-r9}
	stmiapl r0!,{r6-r9}
	bhi wramLoop1
	mvn r6,r6
	mvn r7,r7
	mvn r8,r8
	mvn r9,r9
	subs r2,r2,#1
	bhi wramLoop2

	ldmfd sp!,{r4-r9,r11,lr}
	bx lr
;@----------------------------------------------------------------------------
checkMachine:				;@ Returns machine in r0.
;@----------------------------------------------------------------------------
	ldr r0,romBase
	add r0,r0,#0x8000
	ldrb r2,[r0,#-1]
	and r2,r2,#0xF0
	ldrb r0,g_machine
	cmp r0,#HW_AUTO
	bne cmNoCheck
	ldr r1,g_emuFlags
	tst r1,#SG_MODE
	movne r0,#HW_SG1000
	bne cmSetMachine
	tst r1,#SC_MODE
	movne r0,#HW_SC3000
	bne cmSetMachine
	tst r1,#SGAC_MODE
	movne r0,#HW_SGAC
	bne cmSetMachine
	tst r1,#MD_MODE
	movne r0,#HW_MEGADRIVE
	bne cmSetMachine
	tst r1,#MT_MODE
	movne r0,#HW_MEGATECH
	bne cmSetMachine
	tst r1,#COL_MODE
	movne r0,#HW_COLECO
	bne cmSetMachine
	tst r1,#MSX_MODE
	movne r0,#HW_MSX
	bne cmSetMachine
	tst r1,#SORDM5_MODE
	movne r0,#HW_SORDM5
	bne cmSetMachine
	tst r1,#SYSE_MODE
	movne r0,#HW_SYSE
	bne cmSetMachine
	and r1,r1,#GG_MODE
	cmp r1,#GG_MODE
	beq cmNoGGCheck
	stmfd sp!,{r2,lr}
	bl checkForHeader
	ldmfd sp!,{r2,lr}
	cmp r0,#0
	movne r2,#0
	cmp r2,#0x50				;@ GG Japan
	cmpne r2,#0x60				;@ GG Export
	cmpne r2,#0x70				;@ GG International
cmNoGGCheck:
	moveq r0,#HW_GG
	beq cmSetMachine
	cmp r2,#0x40				;@ SMS Export
	moveq r0,#HW_SMS2
	movne r0,#HW_SMS1
cmSetMachine:
	strb r0,g_machine
cmNoCheck:
	ldrb r1,g_region
	cmp r1,#REGION_AUTO
	beq cmCheckRegion
	sub r1,r1,#1
	cmp r0,#HW_GG
	biceq r1,r1,#PALTIMING
	ldrb r2,g_emuFlags
	bic r2,r2,#3
	orr r2,r2,r1
	strb r2,g_emuFlags
	bx lr
cmCheckRegion:
	ldrb r1,g_emuFlags
	cmp r2,#0x40				;@ SMS Export
	bicne r1,r1,#PALTIMING		;@ Clear PAL bit
	cmpne r2,#0x60				;@ GG Export
	cmpne r2,#0x70				;@ GG International
	orrne r1,r1,#COUNTRY		;@ Set JP bit
	biceq r1,r1,#COUNTRY		;@ Clear JP bit
	strb r1,g_emuFlags

	bx lr

;@----------------------------------------------------------------------------
checkForHeader:				;@ Returns r0=0 if header found.
;@----------------------------------------------------------------------------
	stmfd sp!,{r3}
	ldrb r3,g_cartFlags
	bic r3,#SRAMFLAG

	ldr r1,romBase
	add r1,r1,#0x8000
	ldr r0,[r1,#-0x10]
	ldr r2,=0x20524D54			;@ "TMR "
	subs r0,r0,r2
	ldreq r0,[r1,#-0x0C]
	ldreq r2,=0x41474553		;@ "SEGA"
	subseq r0,r0,r2
	bne headerCheckExit

	ldrb r2,[r1,#-0x01]
	and r2,r2,#0xF0
	ldrb r1,[r1,#-0x03]
	cmp r2,#0x40
	bne headerCheckExit
	and r1,r1,#0x1F
	cmp r1,#0x15
	orreq r3,#SRAMFLAG
headerCheckExit:
	strb r3,g_cartFlags

	ldmfd sp!,{r3}
	bx lr
;@----------------------------------------------------------------------------
fillROMBANKMAP:
;@----------------------------------------------------------------------------
	stmfd sp!,{r2-r4,lr}

	ldr r3,romBase				;@ Get rom base
	ldr r2,romMask				;@ romMask=romsize-1

	ldr r4,=ROMBANKMAP
	mov r0,#0xFF
tbLoop0:
	and r1,r0,r2
	add r1,r3,r1,lsl#13			;@ 8k banks
	str r1,[r4,r0,lsl#2]
	subs r0,r0,#1
	bpl tbLoop0

	ldmfd sp!,{r2-r4,lr}
	bx lr

;@----------------------------------------------------------------------------
initMapper:					;@ Rom paging..
;@----------------------------------------------------------------------------
	ldr r2,=WRMEMTBL_
	add r1,z80optbl,#z80WriteTbl

	ldr r0,[r2]
	str r0,[r1],#4				;@ Z80MemWriteTbl, WRITE_ROM
	str r0,[r1],#4				;@ Z80MemWriteTbl
	str r0,[r1],#4				;@ Z80MemWriteTbl
	str r0,[r1],#4				;@ Z80MemWriteTbl
	str r0,[r1],#4				;@ Z80MemWriteTbl
	str r0,[r1],#4				;@ Z80MemWriteTbl


	ldr r2,=RDMEMTBL_
	add r1,z80optbl,#z80ReadTbl

	ldr r0,[r2]
	str r0,[r1],#4				;@ Z80MemReadTbl, READ_ROM
	str r0,[r1],#4				;@ Z80MemReadTbl
	str r0,[r1],#4				;@ Z80MemReadTbl
	str r0,[r1],#4				;@ Z80MemReadTbl
	str r0,[r1],#4				;@ Z80MemReadTbl
	str r0,[r1],#4				;@ Z80MemReadTbl
	str r0,[r1],#4				;@ Z80MemReadTbl
	str r0,[r1],#4				;@ Z80MemReadTbl
	bx lr
;@----------------------------------------------------------------------------
WRAMEnable:					;@ Internal RAM enable/disable.
;@----------------------------------------------------------------------------
	ldr r2,=WRMEMTBL_
	add r1,z80optbl,#z80WriteTbl

	ldrb r0,BankMap4
	tst r0,#0x10
	ldrne r0,[r2]
	ldreq r0,[r2,#3*4]
	str r0,[r1,#6*4]			;@ Z80MemWriteTbl, WRITE_RAM
	ldreq r0,[r2,#4*4]
	str r0,[r1,#7*4]			;@ Z80MemWriteTbl, WRITE_RAM mirror
	add r1,z80optbl,#z80MemTbl+48*4
	bne ramDisabled

	ldr r0,[r2,#32+3*4]			;@ MEMMAPTBL_
	sub r0,r0,#0xC000

	ldrb r2,g_machine
	cmp r2,#HW_SG1000
	cmpne r2,#HW_SGAC
	cmpne r2,#HW_SG1000II
	beq initRamMapSG1000
	cmp r2,#HW_SC3000
	cmpne r2,#HW_OMV
	beq initRamMapSC3000
	cmp r2,#HW_SYSE
	beq initRamMapSYSE
	mov r2,#2
initM0Loop:
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	sub r0,r0,#0x2000
	subs r2,r2,#1
	bne initM0Loop
	bx lr

initRamMapSC3000:
	mov r2,#8
initM1Loop:
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	sub r0,r0,#0x800
	subs r2,r2,#1
	bne initM1Loop
	bx lr

initRamMapSYSE:
	mov r2,#8
initM5Loop:
	str r0,[r1],#4				;@ rommap
	str r0,[r1],#4				;@ rommap
	subs r2,r2,#1
	bne initM5Loop
	bx lr

ramDisabled:
	ldr r0,=DISABLEDMEM-0xC000
initRamMapSG1000:
	mov r2,#16
initM2Loop:
	str r0,[r1],#4				;@ rommap
	sub r0,r0,#0x400
	subs r2,r2,#1
	bne initM2Loop
	bx lr
;@----------------------------------------------------------------------------
setupMDBios:	;@ This needs to run after cpu reset, enables MD Bios without bankswitch
;@----------------------------------------------------------------------------
	ldrb r1,g_config
	tst r1,#0x80				;@ BIOS on/off
	bxeq lr
	ldr r0,=mdBios
	storeLastBank r0
	str r0,[z80optbl,#z80Regs + 6*4]
	bx lr
;@----------------------------------------------------------------------------
initMSXMemory:
;@----------------------------------------------------------------------------
	ldr r1,g_ROM_Size
	ldr r2,romMaskBackup
	ldr r3,cartBase
	str r3,romBase0k
	str r3,romBase2k

	sub r0,r3,#0x4000
	str r0,romBase4k
	str r0,romBase6k
	and r0,r2,#2
	add r0,r3,r0,lsl#13
	sub r0,r0,#0x8000
	str r0,romBase8k
	str r0,romBaseAk
	sub r0,r0,#0x4000
	str r0,romBaseCk
	str r0,romBaseEk

	cmp r1,#0xC000
	bne no48k
	ldrb r0,[r3,#3]				;@ High byte of start address in msx rom
	and r0,r0,#0xF0
	cmp r0,#0x40
	beq no48k
	str r3,romBase4k			;@ Set rom base
	str r3,romBase6k			;@ Set rom base
	str r3,romBase8k			;@ Set rom base
	str r3,romBaseAk			;@ Set rom base
no48k:
	ands r0,r2,r2
	ldrbeq r0,[r3,#3]			;@ High byte of start address in msx rom
	and r0,r0,#0xF0
;@	cmp r0,#0x40
;@	streq r3,romBase8k
;@	streq r3,romBaseAk
	cmp r0,#0x80
	streq r3,romBase4k
	streq r3,romBase6k

	ldr r1,=EMU_RAM
	str r1,ramBase0k
	str r1,ramBase4k
	str r1,ramBase8k
	str r1,ramBaseCk

	add r1,z80optbl,#z80WriteTbl
	ldr r0,=WRMEMTBL_
	ldr r2,=biosWrite
	str r2,[r0,#0*4]			;@ Bios
	ldr r2,=Konami5
	str r2,[r0,#1*4]			;@ Cart1
	str r2,[r0,#2*4]			;@ Cart2

	str r2,[r1,#2*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r2,[r1,#3*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r2,[r1,#4*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r2,[r1,#5*4]			;@ Z80MemWriteTbl, WRITE_ROM

	bx lr
;@----------------------------------------------------------------------------
initSordM5Memory:
;@----------------------------------------------------------------------------
	add r1,z80optbl,#z80WriteTbl

	ldr r2,=rom_W
	str r2,[r1,#0*4]			;@ Z80MemWriteTbl, Bios ROM
	str r2,[r1,#1*4]			;@ Z80MemWriteTbl, Cart ROM
	str r2,[r1,#2*4]			;@ Z80MemWriteTbl, Cart ROM
	ldr r2,=ram4k_W
	str r2,[r1,#3*4]			;@ Z80MemWriteTbl, Internal RAM
	ldr r2,=ram_W
	str r2,[r1,#4*4]			;@ Z80MemWriteTbl, External RAM
	str r2,[r1,#5*4]			;@ Z80MemWriteTbl, External RAM
	str r2,[r1,#6*4]			;@ Z80MemWriteTbl, External RAM
	str r2,[r1,#7*4]			;@ Z80MemWriteTbl, External RAM

	ldr r3,cartBase
	sub r0,r3,#0x2000
	str r0,romBase2k
	str r0,romBase4k
	add r1,z80optbl,#z80MemTbl+8*4

	mov r2,#16+4
sM5Loop1:
	str r0,[r1],#4				;@ rommap
	subs r2,r2,#1
	bhi sM5Loop1

	ldr r0,=EMU_RAM
	sub r0,r0,#0x7000
	mov r2,#4+32
sM5Loop:
	str r0,[r1],#4				;@ rommap
	subs r2,r2,#1
	bhi sM5Loop

	bx lr
;@----------------------------------------------------------------------------
initMTMemory:
;@----------------------------------------------------------------------------
	add r1,z80optbl,#z80WriteTbl
	ldr r2,=MTIOWrite

	str r2,[r1,#0*4]			;@ Z80MemWriteTbl, ROM
	str r2,[r1,#1*4]			;@ Z80MemWriteTbl, ROM RAM
	str r2,[r1,#2*4]			;@ Z80MemWriteTbl, RAM
	str r2,[r1,#3*4]			;@ Z80MemWriteTbl, IO ROM
	ldr r2,=rom_W
	str r2,[r1,#4*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r2,[r1,#5*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r2,[r1,#6*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r2,[r1,#7*4]			;@ Z80MemWriteTbl, WRITE_ROM

	add r1,z80optbl,#z80ReadTbl
	ldr r2,=MTIORead

	str r2,[r1,#0*4]			;@ Z80MemReadTbl, ROM
	str r2,[r1,#1*4]			;@ Z80MemReadTbl, ROM RAM
	str r2,[r1,#2*4]			;@ Z80MemReadTbl, RAM
	str r2,[r1,#3*4]			;@ Z80MemReadTbl, IO ROM
	str r2,[r1,#4*4]			;@ Z80MemReadTbl, READ_GAME_ROM
	str r2,[r1,#5*4]			;@ Z80MemReadTbl, READ_GAME_ROM
	ldr r2,=rom_R
	str r2,[r1,#6*4]			;@ Z80MemReadTbl, READ_GAME ROM/RAM?
	str r2,[r1,#7*4]			;@ Z80MemReadTbl, READ_GAME ROM/RAM?

	bx lr
;@----------------------------------------------------------------------------
WRAMColeco:					;@ Setup Coleco RAM.
;@----------------------------------------------------------------------------
	add r1,z80optbl,#z80WriteTbl
	ldr r0,=WRMEMTBL_
	ldr r0,[r0]
	ldr r2,=ram1k_W
	str r0,[r1,#0*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r0,[r1,#1*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r0,[r1,#2*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r2,[r1,#3*4]			;@ Z80MemWriteTbl, WRITE_RAM
	str r0,[r1,#4*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r0,[r1,#5*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r0,[r1,#6*4]			;@ Z80MemWriteTbl, WRITE_ROM
	str r0,[r1,#7*4]			;@ Z80MemWriteTbl, WRITE_ROM

	add r1,z80optbl,#z80MemTbl+24*4
	ldr r2,=MEMMAPTBL_+3*4
	ldr r0,[r2]					;@ MEMMAPTBL_ RAM
	sub r0,r0,#0x6000
	mov r2,#8
initM3Loop:
	str r0,[r1],#4				;@ rommap
	sub r0,r0,#0x400
	subs r2,r2,#1
	bne initM3Loop

	ldr r2,=MEMMAPTBL_
	ldr r0,[r2]					;@ MEMMAPTBL_ ROM
	sub r0,r0,#0x8000
	mov r2,#32
initM4Loop:
	str r0,[r1],#4				;@ rommap
	subs r2,r2,#1
	bne initM4Loop

	bx lr

;@----------------------------------------------------------------------------
mapperDetect:		;@ In r0=romBase, r1=size.
					;@ Out r0=mapper, 0=Sega, 1=Codemaster, 2=Korean, 3=Korean MSX.
					;@ Adapted from Meka.
;@----------------------------------------------------------------------------
	cmp r1,#0x8000
	movle r0,#0
	bxle lr

	stmfd sp!,{r2-r8}
	mov r3,#0					;@ 0x8000
	mov r4,#0					;@ 0xA000
	mov r5,#0					;@ 0xFFFF
	mov r6,#0					;@ 0x0002
	add r7,r0,#0x8000			;@ compare size
	mov r8,#-1
mdLoop:
	ldrb r1,[r0],#1
	cmp r1,#0x32				;@ Z80 opcode for: LD (xxxx), A
	beq mdE
mdE2:
	cmp r7,r0
	bhi mdLoop
	mov r0,#0

	add r1,r5,#1				;@ 2?
	cmp r3,r1
	movhi r0,#1
	movhi r3,r1
	cmp r4,r1
	movhi r0,#2
	movhi r4,r1
	cmp r6,r1
	movhi r0,#3

	ldmfd sp!,{r2-r8}
	bx lr

mdE:
	ldrb r1,[r0]
	ldrb r2,[r0,#1]
	orr r1,r1,r2,lsl#8
	cmp r1,#0x8000				;@ Codemaster mapper
	cmpne r1,#0x4000			;@ For Ernie Els Golf. Hopefully doesn't break anything.
	addeq r3,r3,#1
	addeq r0,r0,#2
	cmp r1,#0xA000				;@ Korean mapper
	addeq r4,r4,#1
	addeq r0,r0,#2
	cmp r1,r8,lsr#16			;@ SEGA mapper
	addeq r5,r5,#1
	addeq r0,r0,#2
	cmp r1,#0x0002				;@ Korean MSX mapper
	cmpne r1,#0x0003
//	cmpne r1,#0x0001
	addeq r6,r6,#1
	addeq r0,r0,#2
	b mdE2
;@----------------------------------------------------------------------------
cartSaveState:	;@ Called from C code.
	.type   cartSaveState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r5,lr}

	mov r5,r0					;@ r5=where to copy state

	mov r0,#0					;@ r0 holds total size (return value)

	mov r12,#(lstEnd-saveLst)/8	;@ r4=items in list
	adr r3,saveLst				;@ r3=list of stuff to copy
ss1:
	ldmia r3!,{r1,r2}			;@ r1=what to copy, r2=how much to copy
	add r0,r0,r2
ss0:
	ldr r4,[r1],#4
	str r4,[r5],#4
	subs r2,r2,#4
	bne ss0
	subs r12,r12,#1
	bne ss1

	ldmfd sp!,{r4-r5,lr}
	bx lr

saveLst:
	.long EMU_SRAM,0x8000
	.long EMU_RAM,0x2000
	.long romInfo,12
	.long BankState,24
lstEnd:

;@----------------------------------------------------------------------------
cartLoadState:	;@ Called from C code.
	.type   cartLoadState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r5,z80pc,z80optbl,lr}
	ldr z80optbl,=Z80OpTable

	mov r12,#(lstEnd-saveLst)/8	;@ Read entire state
	adr r3,saveLst
	mov r5,#0
ls1:
	ldmia r3!,{r1,r2}
	add r5,r5,r2
ls0:
	ldr r4,[r0],#4
	str r4,[r1],#4
	subs r2,r2,#4
	bne ls0
	subs r12,r12,#1
	bne ls1

	ldrb r0,g_machine
	cmp r0,#HW_MARK3
	cmpne r0,#HW_SMS1
	cmpne r0,#HW_SMS2
	cmpne r0,#HW_GG
	cmpne r0,#HW_MEGADRIVE
	bne dontInitMappers

	ldrb r0,BankMap4
	ldr r1,biosBase
	tst r0,#0x40				;@ ROM?
	ldreq r1,cartBase
	tst r0,#0x08				;@ BIOS?
	ldrne r1,cartBase

	str r1,romBase

	bl fillROMBANKMAP
	bl reBankSwitch0_W
	bl reBankSwitch1_W
	bl reBankSwitch2_W
dontInitMappers:
	mov r0,r5
	ldmfd sp!,{r4-r5,z80pc,z80optbl,lr}
	bx lr
;@----------------------------------------------------------------------------
cartGetStateSize:	;@ Called from C code.
	.type   cartGetStateSize STT_FUNC
;@----------------------------------------------------------------------------
	mov r12,#(lstEnd-saveLst)/8	;@ Read entire state
	adr r3,saveLst
	mov r0,#0
sizeLoop1:
	ldmia r3!,{r1,r2}
	add r0,r0,r2
	subs r12,r12,#1
	bne sizeLoop1

	bx lr



;@----------------------------------------------------------------------------
//	.section itcm
;@----------------------------------------------------------------------------


;@------------------------------------------------------------------------------
MemCtrl_MD_W:
;@------------------------------------------------------------------------------
	bic r0,r0,#0x50			;@ WRAM & CART allways enabled on MD?
	b MemCtrl_SMS_W

;@------------------------------------------------------------------------------
reBankSwitchB_W:			;@ Bankswitch BIOS
;@------------------------------------------------------------------------------
	ldrb r0,BankMap4
;@------------------------------------------------------------------------------
BankSwitchB_W:				;@ Switch to BIOS
;@------------------------------------------------------------------------------
	ldrb r1,g_machine
	cmp r1,#HW_GG
	beq BankSwitchB_GG_W

;@------------------------------------------------------------------------------
MemCtrl_SMS_W:
;@------------------------------------------------------------------------------
;@------------------------------------------------------------------------------
BankSwitchB_SMS_W:			;@ Switch to BIOS (SMS)
;@------------------------------------------------------------------------------
;@	mov r11,r11
;@	ldrb r1,BankMap4
;@	cmp r0,r1
;@	bxeq lr
	strb r0,BankMap4
	stmfd sp!,{r0,lr}
	bl WRAMEnable
	ldmfd sp!,{r0,lr}

	ldr r1,biosBase
	tst r0,#0x40				;@ CART ROM?
	ldreq r1,cartBase
	tstne r0,#0x08				;@ BIOS?
	ldrne r1,cartBase

	ldr r0,romBase
	cmp r1,r0
	bxeq lr


	ldr r0,biosBase
	cmp r0,r1
	ldr r0,BankMap0
	streq r0,BankMap_Cart
	strne r0,BankMap_Bios
	ldreq r0,BankMap_Bios
	ldrne r0,BankMap_Cart
	str r0,BankMap0
	str r1,romBase

	ldrne r0,romMaskBackup
	moveq r0,#0x7F
	str r0,romMask

	stmfd sp!,{lr}
	bl fillROMBANKMAP
	bl reBankSwitch0_W
	bl reBankSwitch1_W
	ldmfd sp!,{lr}
	b reBankSwitch2_W

;@------------------------------------------------------------------------------
BankSwitchB_GG_W:			;@ Switch to BIOS for 0x0000-0x03FF
;@------------------------------------------------------------------------------
	strb r0,BankMap4
	stmfd sp!,{r0,lr}
	bl WRAMEnable
	ldmfd sp!,{r0,lr}

	tst r0,#0x08				;@ BIOS?
	ldreq r0,biosBase
	ldrne r0,romBase
	add r1,z80optbl,#z80MemTbl
	str r0,[r1]					;@ rommap
	b flush




;@------------------------------------------------------------------------------
;@						Codemasters mapper stuff
;@------------------------------------------------------------------------------
;@------------------------------------------------------------------------------
reBankSwitch0C_W:			;@ 0x0000-0x3FFF, Codemasters mapper
;@------------------------------------------------------------------------------
	ldrb r0,BankMap1
;@------------------------------------------------------------------------------
BankSwitch0C_W:				;@ 0x0000-0x3FFF
;@------------------------------------------------------------------------------
	strb r0,BankMap1
	and r0,r0,#0x7F
	ldrb r1,BankMap4
	tst r1,#0x08
	ldreq r1,biosBase
	ldrne r1,=ROMBANKMAP
	ldrne r1,[r1,r0,lsl#3]
	add r2,z80optbl,#z80MemTbl
	str r1,[r2],#4				;@ rommap
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#3]
	b Map15k
;@------------------------------------------------------------------------------
reBankSwitch1C_W:			;@ 0x4000-0x7FFF, Codemasters mapper
;@------------------------------------------------------------------------------
	ldrb r0,BankMap2
;@------------------------------------------------------------------------------
BankSwitch1C_W:				;@ 0x4000-0x7FFF
;@------------------------------------------------------------------------------
	stmfd sp!,{lr}
	bl BankSwitch1_W
	ldmfd sp!,{lr}
;@------------------------------------------------------------------------------
reBankSwitch2C_W:			;@ 0x8000-0xBFFF
;@------------------------------------------------------------------------------
	ldrb r0,BankMap3
;@------------------------------------------------------------------------------
BankSwitch2C_W:				;@ 0x8000-0xBFFF
;@------------------------------------------------------------------------------
	strb r0,BankMap3
	ldrb r1,BankMap2
	tst r1,#0x80
	beq BankSwitch2_W			;@ Normal Mapping.

	and r0,r0,#0x7F
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#3]
	sub r0,r0,#0x8000
	add r2,z80optbl,#z80MemTbl+32*4
	stmfd sp!,{lr}
	mov r1,r0
	bl Map8k					;@ Only map Rom to 0x8000-0x9FFF
	ldmfd sp!,{lr}


	ldr r2,=MEMMAPTBL_
	ldr r0,[r2,#20]!
	ldr r1,[r2,#-8*4]			;@ WRMEMTBL_
	add r2,z80optbl,#z80WriteTbl+16
	str r1,[r2,#4]				;@ Z80_memwritetbl
	add r2,z80optbl,#z80MemTbl+40*4
	mov r1,r0
	b Map8k						;@ Only map RAM to 0xA000-0xBFFF

;@------------------------------------------------------------------------------
BankSwitch0MSX_W:			;@ 0x4000-0x5FFF
;@------------------------------------------------------------------------------
	and r0,r0,#0xFF
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#2]
	sub r0,r0,#0x4000
	add r2,z80optbl,#z80MemTbl+16*4
	mov r1,r0
	b Map8k						;@ Only map Rom to 0x4000-0x5FFF
;@------------------------------------------------------------------------------
BankSwitch1MSX_W:			;@ 0x6000-0x7FFF
;@------------------------------------------------------------------------------
	and r0,r0,#0xFF
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#2]
	sub r0,r0,#0x6000
	add r2,z80optbl,#z80MemTbl+24*4
	mov r1,r0
	b Map8k						;@ Only map Rom to 0x6000-0x7FFF
;@------------------------------------------------------------------------------
BankSwitch2MSX_W:			;@ 0x8000-0x9FFF
;@------------------------------------------------------------------------------
	and r0,r0,#0xFF
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#2]
	sub r0,r0,#0x8000
	add r2,z80optbl,#z80MemTbl+32*4
	mov r1,r0
	b Map8k						;@ Only map Rom to 0x8000-0x9FFF
;@------------------------------------------------------------------------------
BankSwitch3MSX_W:			;@ 0xA000-0xBFFF
;@------------------------------------------------------------------------------
	and r0,r0,#0xFF
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#2]
	sub r0,r0,#0xA000
	add r2,z80optbl,#z80MemTbl+40*4
	mov r1,r0
	b Map8k						;@ Only map Rom to 0xA000-0xBFFF

;@----------------------------------------------------------------------------
;@----------------------------------------------------------------------------
reBankSwitch0_W:			;@ 0x0000-0x3FFF
;@----------------------------------------------------------------------------
	ldrb r0,BankMap1
;@----------------------------------------------------------------------------
BankSwitch0_W:				;@ 0x0000-0x3FFF
;@----------------------------------------------------------------------------
	strb r0,BankMap1
	ldrb r1,BankMap4
	tst r1,#0x08
	and r0,r0,#0x7F
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#3]
	ldreq r1,biosBase
	ldrne r1,[r1]
	add r2,z80optbl,#z80MemTbl
	str r1,[r2],#4				;@ rommap
	b Map15k

;@----------------------------------------------------------------------------
reBankSwitch1_W:			;@ 0x4000-0x7FFF
;@----------------------------------------------------------------------------
	ldrb r0,BankMap2
;@----------------------------------------------------------------------------
BankSwitch1_W:				;@ 0x4000-0x7FFF
;@----------------------------------------------------------------------------
	strb r0,BankMap2
	and r0,r0,#0x7F
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#3]
	sub r0,r0,#0x4000
	add r2,z80optbl,#z80MemTbl+16*4
	b Map16k

;@----------------------------------------------------------------------------
reBankSwitchR_W:			;@ 0x8000-0xBFFF
;@----------------------------------------------------------------------------
	ldrb r0,BankMap0
;@----------------------------------------------------------------------------
BankSwitchR_W:				;@ Switch between ROM & RAM for 0x8000
;@----------------------------------------------------------------------------
	strb r0,BankMap0
	tst r0,#0x08				;@ RAM or ROM?
	beq reBankSwitch2_W

	and r0,r0,#0x04				;@ Bank 0/1?

	ldr r2,=MEMMAPTBL_+20
	ldr r0,[r2,r0]!
	ldr r1,[r2,#-8*4]			;@ WRMEMTBL_
	b doBank2
;@----------------------------------------------------------------------------
reBankSwitch2_W:			;@ 0x8000-0xBFFF
;@----------------------------------------------------------------------------
	ldrb r0,BankMap3
;@----------------------------------------------------------------------------
BankSwitch2_W:				;@ 0x8000-0xBFFF
;@----------------------------------------------------------------------------
	strb r0,BankMap3
	ldrb r1,BankMap0
	tst r1,#8					;@ RAM or ROM?
	bne reBankSwitchR_W

	and r0,r0,#0x7F
	ldr r1,=ROMBANKMAP
	ldr r0,[r1,r0,lsl#3]
	sub r0,r0,#0x8000
	ldr r1,=WRMEMTBL_
	ldr r1,[r1]
doBank2:
	add r2,z80optbl,#z80WriteTbl+16
	str r1,[r2],#4				;@ Z80MemWriteTbl
	str r1,[r2],#4				;@ Z80MemWriteTbl
	add r2,z80optbl,#z80MemTbl+32*4
Map16k:
	str r0,[r2],#4				;@ rommap
Map15k:
	str r0,[r2],#4				;@ rommap
	mov r1,r0
	stmia r2!,{r0-r1}			;@ rommap
	stmia r2!,{r0-r1}			;@ rommap
	stmia r2!,{r0-r1}			;@ rommap
Map8k:
	stmia r2!,{r0-r1}			;@ rommap
	stmia r2!,{r0-r1}			;@ rommap
	stmia r2!,{r0-r1}			;@ rommap
	stmia r2!,{r0-r1}			;@ rommap
;@----------------------------------------------------------------------------
flush:						;@ Update cpu_pc & lastbank
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	reEncodePC
	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
Map8k_2:
	mov r1,r0
	stmia r2!,{r0-r1}			;@ z80MemMapTbl
	stmia r2!,{r0-r1}			;@ z80MemMapTbl
	stmia r2!,{r0-r1}			;@ z80MemMapTbl
	stmia r2!,{r0-r1}			;@ z80MemMapTbl
	bx lr
;@----------------------------------------------------------------------------


;@----------------------------------------------------------------------------
Konami4:
;@----------------------------------------------------------------------------
	and r1,r0,#0x1F				;@ romMask=32(8k)-1
	ldr r2,cartBase
	add r1,r2,r1,lsl#13
	and r2,addy,#0xE000
	cmp r2,#0x6000				;@ Bankswitch 0x6000
	subeq r1,r1,#0x6000
	streq r1,romBase6k
	beq BankMSX1_W
	cmp r2,#0x8000				;@ Bankswitch 0x8000
	subeq r1,r1,#0x8000
	streq r1,romBase8k
	beq BankMSX2_W
	cmp r2,#0xA000				;@ Bankswitch 0xA000
	subeq r1,r1,#0xA000
	streq r1,romBaseAk
	beq BankMSX2_W
	b cart2Write
;@----------------------------------------------------------------------------
Konami5:					;@ Mapper + SCC
;@----------------------------------------------------------------------------
	ldr r1,romMask				;@ romMask=romsize(8k)-1
	and r1,r1,r0
	ldr r2,cartBase
	add r1,r2,r1,lsl#13
	and r2,addy,#0xF800
	cmp r2,#0x5000				;@ Bankswitch 0x4000
	subeq r1,r1,#0x4000
	streq r1,romBase4k
	beq BankMSX1_W
	cmp r2,#0x7000				;@ Bankswitch 0x6000
	subeq r1,r1,#0x6000
	streq r1,romBase6k
	beq BankMSX1_W
	cmp r2,#0x9000				;@ Bankswitch 0x8000
	subeq r1,r1,#0x8000
	streq r1,romBase8k
	beq BankMSX2_W
	cmp r2,#0x9800				;@ Write to SCC
	beq SCCWrite_0
	cmp r2,#0xB000				;@ Bankswitch 0xA000
	subeq r1,r1,#0xA000
	streq r1,romBaseAk
	beq BankMSX2_W
	b cart2Write
;@----------------------------------------------------------------------------
MSXMapRAM0W:				;@ IO-Port FC
;@----------------------------------------------------------------------------
//	ldr r1,ramMask				;@ ramMask=ramSize(16k)-1
	and r1,r0,#0x03
	ldr r2,=EMU_RAM
	add r1,r2,r1,lsl#14
	str r1,ramBase0k
	b BankMSX0_W
;@----------------------------------------------------------------------------
MSXMapRAM1W:				;@ IO-Port FD
;@----------------------------------------------------------------------------
//	ldr r1,ramMask				;@ ramMask=ramSize(16k)-1
	and r1,r0,#0x03
	ldr r2,=EMU_RAM-0x4000
	add r1,r2,r1,lsl#14
	str r1,ramBase4k
	b BankMSX1_W
;@----------------------------------------------------------------------------
MSXMapRAM2W:				;@ IO-Port FE
;@----------------------------------------------------------------------------
//	ldr r1,ramMask				;@ ramMask=ramSize(16k)-1
	and r1,r0,#0x03
	ldr r2,=EMU_RAM-0x8000
	add r1,r2,r1,lsl#14
	str r1,ramBase8k
	b BankMSX2_W
;@----------------------------------------------------------------------------
MSXMapRAM3W:				;@ IO-Port FF
;@----------------------------------------------------------------------------
//	ldr r1,ramMask				;@ ramMask=ramSize(16k)-1
	and r1,r0,#0x03
	ldr r2,=EMU_RAM-0xC000
	add r1,r2,r1,lsl#14
	str r1,ramBaseCk
	b BankMSX3_W
;@----------------------------------------------------------------------------
BankMSX0_W:					;@ 0x0000-0x3FFF
;@----------------------------------------------------------------------------
	ldrb r1,BankMap0
	ands r1,r1,#0x03

	ldr r2,=WRMEMTBL_			;@ RAM writeprotection
	ldr r0,[r2,r1,lsl#2]
	add r2,z80optbl,#z80WriteTbl
	str r0,[r2,#4*0]
	str r0,[r2,#4*1]

	add r2,z80optbl,#z80MemTbl
	ldr r0,biosBase
	beq Map16k
	cmp r1,#0x03
	ldreq r0,ramBase0k
	beq Map16k

	ldr r0,romBase0k
	stmfd sp!,{lr}
	bl Map8k_2
	ldmfd sp!,{lr}
	ldr r0,romBase2k
	mov r1,r0
	b Map8k

;@----------------------------------------------------------------------------
BankMSX1_W:					;@ 0x4000-0x7FFF
;@----------------------------------------------------------------------------
;@	mov r11,r11
	ldrb r1,BankMap0
	ands r1,r1,#0x0C

	ldr r2,=WRMEMTBL_			;@ RAM writeprotection
	ldr r0,[r2,r1]
	add r2,z80optbl,#z80WriteTbl
	str r0,[r2,#4*2]
	str r0,[r2,#4*3]

	add r2,z80optbl,#z80MemTbl+16*4
	ldr r0,biosBase
	beq Map16k
	cmp r1,#0x0C
	ldreq r0,ramBase4k
	beq Map16k

	ldr r0,romBase4k
	stmfd sp!,{lr}
	bl Map8k_2
	ldmfd sp!,{lr}
	ldr r0,romBase6k
	mov r1,r0
	b Map8k

;@----------------------------------------------------------------------------
BankMSX2_W:					;@ 0x8000-0xBFFF
;@----------------------------------------------------------------------------
	ldrb r1,BankMap0
	ands r1,r1,#0x30

	ldr r2,=WRMEMTBL_			;@ RAM writeprotection
	ldr r0,[r2,r1,lsr#2]
	add r2,z80optbl,#z80WriteTbl
	str r0,[r2,#4*4]
	str r0,[r2,#4*5]

	add r2,z80optbl,#z80MemTbl+32*4
	ldr r0,biosBase				;@ Undefined actually.
	beq Map16k
	cmp r1,#0x30
	ldreq r0,ramBase8k
	beq Map16k

	ldr r0,romBase8k
	stmfd sp!,{lr}
	bl Map8k_2
	ldmfd sp!,{lr}
	ldr r0,romBaseAk
	mov r1,r0
	b Map8k

;@----------------------------------------------------------------------------
BankMSX3_W:					;@ 0xC000-0xFFFF
;@----------------------------------------------------------------------------
	ldrb r1,BankMap0
	ands r1,r1,#0xC0

	ldr r2,=WRMEMTBL_			;@ RAM writeprotection
	ldr r0,[r2,r1,lsr#4]
	add r2,z80optbl,#z80WriteTbl
	str r0,[r2,#4*6]
	str r0,[r2,#4*7]

	add r2,z80optbl,#z80MemTbl+48*4
	ldr r0,biosBase				;@ Undefined actually
	beq Map16k
	cmp r1,#0xC0
	ldreq r0,ramBaseCk
	beq Map16k

	ldr r0,romBaseCk
	stmfd sp!,{lr}
	bl Map8k_2
	ldmfd sp!,{lr}
	ldr r0,romBaseEk
	mov r1,r0
	b Map8k

;@----------------------------------------------------------------------------
romBase0k:	.long 0
romBase2k:	.long 0
romBase4k:	.long 0
romBase6k:	.long 0
romBase8k:	.long 0
romBaseAk:	.long 0
romBaseCk:	.long 0
romBaseEk:	.long 0

ramBase0k:	.long 0
ramBase4k:	.long 0
ramBase8k:	.long 0
ramBaseCk:	.long 0

romBase:	.long 0
biosBase:	.long 0
cardBase:	.long 0
cartBase:	.long 0
expBase:	.long 0
g_BIOSBASE_US:
	.long 0						;@ biosbase_sms, SMS
g_BIOSBASE_JP:
	.long 0						;@ biosbase_sms, SMS
g_BIOSBASE_GG:
	.long 0						;@ biosbase_gg, GG
g_BIOSBASE_COLECO:
	.long 0						;@ COLECO
g_BIOSBASE_MSX:
	.long 0						;@ MSX
g_BIOSBASE_SORDM5:
	.long 0						;@ Sord M5
g_ROM_Size:
	.long 0
romMask:
	.long 0
romMaskBackup:
	.long 0

romInfo:						;@ Keep emuflags together for savestate/loadstate
g_emuFlags:
	.long 0						;@ emuflags      (label this so UI.C can take a peek) see equates.h for bitfields
g_scalingSet:
	.byte SCALED_FIT			;@ scalemode(saved display type), default scale to fit
g_cartFlags:
	.byte 0 					;@ cartflags
g_machine:
	.byte 0
g_machineSet:
	.byte 0
g_config:
	.byte 0						;@ config, bit 7=BIOS on/off, bit 6=X as GG Start, bit 5=Select as Reset, bit 4=R as FastForward
g_configSet:
	.byte 0x80
g_region:
	.byte 0						;@ 0=Auto, 1=USA, 2=Euro, 3=Japan.
gArcadeGameSet:
	.byte 0						;@ see equates for defines, 0 = none.
//	.space 4		;@ alignment.

BankState:
BankMap_Exp:
	.long 0
BankMap_Card:
	.long 0
BankMap_Cart:
	.long 0
BankMap_Bios:
	.long 0
BankMap0:	.byte 0
BankMap1:	.byte 0
BankMap2:	.byte 0
BankMap3:	.byte 0
BankMap4:	.byte 0
	.byte 0,0,0		;@ alignment.

	.pool
	.section .bss
EMU_RAM:
	.space 0x2000
WRAP_RAM:						;@ Fix for "Aerial Assault [v0].gg" which jump to $FFFx and then goes from $FFFF to $0000
	.space 8
	.space 0x2000				;@ Extra RAM for System-E
	.space 0xC000				;@ MSX RAM
EMU_SRAM:
	.space 0x8000
DISABLEDMEM:
	.space 0x400
WRMEMTBL_:
	.space 8*4
MEMMAPTBL_:
	.space 8*4
RDMEMTBL_:
	.space 8*4
ROMBANKMAP:
	.space 256*4
ROM_Space:
	.space 0x100000
BIOS_US_Space:
	.space 0x40000				;@ US/EU 256kB BIOS max
BIOS_JP_Space:
	.space 0x2000				;@ JP 8kB BIOS max
BIOS_GG_Space:
	.space 0x400				;@ GG 1kB BIOS max
BIOS_COLECO_Space:
	.space 0x2000				;@ Coleco 8kB BIOS max
BIOS_MSX_Space:
	.space 0x8000				;@ MSX 32kB BIOS max
BIOS_SORDM5_Space:
	.space 0x2000				;@ Sord M5 8kB BIOS max

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
