#ifdef __arm__

#include "Equates.h"
#include "SN76496/SN76496.i"
#include "AY38910/AY38910.i"
#include "YM2413/YM2413.i"
#include "SCC/SCC.i"

	.extern pauseEmulation
	.extern powerIsOn

	.global soundInit
	.global soundReset
	.global soundRender
	.global soundSetMuteGUI
	.global soundSetMuteGame
	.global soundUpdate
	.global soundSetFrequency
	.global SMSJSoundControlW
	.global SMSJSoundControlR
	.global SN76496_0
	.global SN76496_1
	.global SN76496_W
	.global SN76496_0_W
	.global SN76496_1_W
	.global YM2413_0_AddressW
	.global YM2413_0_DataW
	.global YM2413_0_StatusR
	.global AY38910_0
	.global AY38910Index_0_W
	.global AY38910Data_0_W
	.global AY38910Data_0_R
	.global SCC_0
	.global SCCWrite_0
	.global GG_Stereo_W
	.global SoundVariables

#define SHIFTVAL (21)
#define SOUND_BUFFER_SIZE (1<<(32-SHIFTVAL))

#if !defined(SN_UPSHIFT)
	#define SN_UPSHIFT (2)
#endif

	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
soundInit:
	.type soundInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldr r0,=SoundVariables
	ldr r1,=SoundVariablesEnd
	sub r1,r1,r0
	mov r1,r1,lsr#2
	bl memclr_
	bl soundSetFrequency

	ldmfd sp!,{lr}
//	bx lr

;@----------------------------------------------------------------------------
soundReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}

	mov r0,#0
	strb r0,muteSoundGame
	mov r0,#0xFF
	strb r0,muteSoundGUI
	mov r0,#2
	strb r0,SMSJSoundControl
	ldr r4,=gMachine
	ldrb r4,[r4]
	cmp r4,#HW_MSX
	beq isMSX
	cmp r4,#HW_SG1000
	cmpne r4,#HW_SGAC
	cmpne r4,#HW_SC3000
	cmpne r4,#HW_OMV
	cmpne r4,#HW_SG1000II	;@ ?
	cmpne r4,#HW_COLECO
	mov r0,#0
	movne r0,#1
	ldr r1,=SN76496_0
	bl sn76496Reset			;@ Sound
	cmp r4,#HW_SYSE
	bne noSysE
	mov r0,#1
	ldr r1,=SN76496_1
	bl sn76496Reset			;@ Sound
noSysE:
	ldr ymptr,=YM2413_0
	bl YM2413Reset			;@ Sound
	ldmfd sp!,{r4,lr}
	bx lr
isMSX:
	ldr r0,=SCC_0
	bl SCCReset
	ldr r4,=AY38910_0
	mov r0,r4
	bl ay38910Reset
	ldr r0,=MSXJoyReadCallBack
	str r0,[r4,#ayPortAInFptr]
	ldmfd sp!,{r4,lr}
	bx lr
;@----------------------------------------------------------------------------
SMSJSoundControlW:
;@----------------------------------------------------------------------------
	and r0,r0,#3
	strb r0,SMSJSoundControl
	bx lr
;@----------------------------------------------------------------------------
SMSJSoundControlR:
;@----------------------------------------------------------------------------
	ldrb r0,SMSJSoundControl
	and r0,r0,#0x01
	orr r0,r0,#0xF8
	bx lr
;@----------------------------------------------------------------------------
soundSetFrequency:
	.type soundSetFrequency STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	ldr r1,=gEmuFlags
	ldrb r1,[r1]
	tst r1,#PALTIMING
	ldr r1,=3579545				;@ NTSC freq
	ldrne r1,=3546893			;@ PAL freq
	ldmfd sp!,{lr}
	bx lr

;@----------------------------------------------------------------------------
soundSetMuteGUI:
	.type   soundSetMuteGUI STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=pauseEmulation		;@ Output silence when emulation paused.
	ldrb r0,[r1]
	ldr r1,=powerIsOn			;@ Output silence when power off.
	ldrb r1,[r1]
	cmp r1,#0
	orreq r0,r0,#0xFF
	strb r0,muteSoundGUI
	bx lr
;@----------------------------------------------------------------------------
soundSetMuteGame:			;@ For System E ?
;@----------------------------------------------------------------------------
	strb r0,muteSoundGame
	bx lr
;@----------------------------------------------------------------------------
soundRender:				;@ r0=length, r1=pointer
;@----------------------------------------------------------------------------
	ldr r2,muteSound
	cmp r2,#0
	bne silenceMix

	stmfd sp!,{r0,r1,r4,r5,lr}

	ldr r1,=mixSpace0

	ldr r2,=gMachine
	ldrb r2,[r2]
	cmp r2,#HW_MSX
	beq MSXMix
	cmp r2,#HW_SYSE
	beq sysEMix
	ldrb r2,SMSJSoundControl
	cmp r2,#1				;@ Only YM2413 enabled?
	ldmfd sp,{r0,r1}
	blne sn76496Render
	ldrb r2,SMSJSoundControl
	tst r2,#1				;@ YM2413 enabled?
	ldmfd sp,{r0}
	bne YM2413Mix
	ldmfd sp!,{r0,r1,r4,r5,lr}
	bx lr
;@----------------------------------------------------------------------------
sn76496Render:
;@----------------------------------------------------------------------------
	ldr r4,pcmReadPtr
	add r5,r4,r0
	str r5,pcmReadPtr

;@------------------------------
soundCopyBuff:				;@ r0=length, r1=destination
	ldr r2,=WAVBUFFER			;@ Source
	mov r4,r4,lsl#SHIFTVAL
sndCopyLoop:
	subs r0,r0,#1
	ldrpl r3,[r2,r4,lsr#SHIFTVAL-2]
	addpl r4,r4,#1<<SHIFTVAL
	strpl r3,[r1],#4
	bhi sndCopyLoop
;@------------------------------

	ldr r2,sndWritePtr
	ldr r0,pcmWritePtr
	sub r2,r2,r0,lsl#SHIFTVAL
	add r0,r0,r2,lsr#SHIFTVAL
	str r0,pcmWritePtr
	sub r0,r5,r0
	add r0,r0,#SOUND_BUFFER_SIZE/2
	ldr r2,neededExtra
	rsb r2,r2,r2,lsl#3			;@ mul 7
	add r0,r2,r0
	mov r0,r0,asr#3
	str r0,neededExtra
	mov r0,r0,asr#3
	adds r0,r0,#228
	movmi r0,#10
	str r0,samplesCnt

//	blx debugIOUnimplR
	bx lr
;@----------------------------------------------------------------------------
sysEMix:
	ldr r2,=SN76496_1
	bl sn76496Mixer
addExtraSN76496:
	ldmfd sp,{r0}
	ldr r1,=mixSpace1
	ldr r2,=SN76496_0
	bl sn76496Mixer
mix2Chips:
	ldmfd sp,{r0,r1}
	ldr r2,=mixSpace0
	ldr r3,=mixSpace1
mixLoop02:
	ldrsh r4,[r2],#4
	ldrsh r5,[r3],#4
	add r4,r4,r5

	mov r4,r4,lsl#16-1
	mov r4,r4,lsr#16
	orr r4,r4,r4,lsl#16
	subs r0,r0,#1
	strpl r4,[r1],#4
	bhi mixLoop02

	ldmfd sp!,{r0,r1,r4,r5,lr}
	bx lr

;@----------------------------------------------------------------------------
YM2413Mix:
;@----------------------------------------------------------------------------
	mov r0,r0,lsl#3
	ldr r1,=mixSpace0
	ldr ymptr,=YM2413_0
	bl YM2413Mixer
	ldmfd sp,{r0,r1}
	ldrb r2,SMSJSoundControl
	tst r2,#2				;@ SN76496 enabled?
	bne mixYMAndSN76496
	ldr r2,=mixSpace0
mixLoop03:
	ldrsh r4,[r2],#2
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2]
	add r4,r4,r5

	mov r4,r4,lsl#16-3
	mov r4,r4,lsr#16
	orr r4,r4,r4,lsl#16

	subs r0,r0,#1
	strpl r4,[r1],#4
	bhi mixLoop03

	ldmfd sp!,{r0,r1,r4,r5,lr}
	bx lr

mixYMAndSN76496:
	ldr r2,=mixSpace0
mixLoop03b:
	ldrsh r4,[r2],#2
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r2],#2
	add r4,r4,r5
	ldrsh r5,[r1]
	add r4,r4,r5,lsl#3

	mov r4,r4,lsl#16-4
	mov r4,r4,lsr#16
	orr r4,r4,r4,lsl#16

	subs r0,r0,#1
	strpl r4,[r1],#4
	bhi mixLoop03b

	ldmfd sp!,{r0,r1,r4,r5,lr}
	bx lr

;@----------------------------------------------------------------------------
YM2413MixC:
;@----------------------------------------------------------------------------
	mov r2,r0,lsr#2
	mov r0,#0
	blx YM2413UpdateOneMono
	ldmfd sp,{r0,r1}
	ldr r2,=mixSpace0
mixLoop03C:
	ldrh r4,[r2],#2
	add r4,r4,r4,lsl#16

	subs r0,r0,#1
	strpl r4,[r1],#4
	bhi mixLoop03C

	ldmfd sp!,{r0,r1,r4,r5,lr}
	bx lr

;@----------------------------------------------------------------------------
MSXMix:
;@----------------------------------------------------------------------------
	ldr r2,=AY38910_0
	bl ay38910Mixer
//	ldrb r0,SCCEnabled
//	cmp r0,#0
//	noSCC
	ldmfd sp,{r0}
	ldr r1,=mixSpace1
	ldr r2,=SCC_0
	bl SCCMixer
//	b mix2Chips
noSCC:
	ldmfd sp,{r0,r1}
	ldr r2,=mixSpace0
	ldr r3,=mixSpace1
//	ldr r12,soundFilter
mixLoop01:
	ldrsh r4,[r2],#2
	ldrsh r5,[r3],#2
	add r4,r4,r5

//	sub r12,r12,r12,asr#2
//	add r12,r12,r4,lsl#16-3-2
//	mov r4,r12,lsr#16

	mov r4,r4,lsl#16-1
	mov r4,r4,lsr#16
	orr r4,r4,r4,lsl#16
	subs r0,r0,#1
	strpl r4,[r1],#4
	bhi mixLoop01

//	str r12,soundFilter
	ldmfd sp!,{r0,r1,r4,r5,lr}
	bx lr

silenceMix:
	mov r12,r0
	mov r2,#0
silenceLoop:
	subs r12,r12,#1
	strpl r2,[r1],#4
	bhi silenceLoop

	bx lr

soundFilter:
	.long 0
;@----------------------------------------------------------------------------
GG_Stereo_W:
;@----------------------------------------------------------------------------
	ldr r1,=SN76496_0
	b sn76496GGW
;@----------------------------------------------------------------------------
SN76496_W:
SN76496_0_W:
;@----------------------------------------------------------------------------
	ldr r1,=SN76496_0
	b sn76496W
;@----------------------------------------------------------------------------
SN76496_1_W:
;@----------------------------------------------------------------------------
	ldr r1,=SN76496_1
	b sn76496W
;@----------------------------------------------------------------------------
YM2413_0_AddressW:
;@----------------------------------------------------------------------------
	ldr ymptr,=YM2413_0
	b YM2413AddressW
;@----------------------------------------------------------------------------
YM2413_0_DataW:
;@----------------------------------------------------------------------------
	ldr ymptr,=YM2413_0
	b YM2413DataW

	stmfd sp!,{r3,lr}
	mov r2,r0
	mov r1,r12
	mov r0,#0
	blx YM2413Write
	ldmfd sp!,{r3,lr}
	bx lr
;@----------------------------------------------------------------------------
YM2413_0_StatusR:
;@----------------------------------------------------------------------------
	ldr ymptr,=YM2413_0
	b YM2413StatusR
	stmfd sp!,{r3,lr}
	mov r1,r12
	mov r0,#0
	blx YM2413Read
	ldmfd sp!,{r3,lr}
	bx lr
;@----------------------------------------------------------------------------
AY38910Index_0_W:
;@----------------------------------------------------------------------------
	ldr r1,=AY38910_0
	b ay38910IndexW
;@----------------------------------------------------------------------------
AY38910Data_0_W:
;@----------------------------------------------------------------------------
	ldr r1,=AY38910_0
	b ay38910DataW
;@----------------------------------------------------------------------------
AY38910Data_0_R:
;@----------------------------------------------------------------------------
	ldr r0,=AY38910_0
	b ay38910DataR
;@----------------------------------------------------------------------------
SCCWrite_0:
;@----------------------------------------------------------------------------
	mov r1,r12
	ldr r2,=SCC_0
	b SCCWrite

;@----------------------------------------------------------------------------
soundUpdate:				;@ r0 = samples to render
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,r4,lr}
	ldr r1,=WAVBUFFER
	ldr r2,sndWritePtr
//	mov r0,#228
	ldr r0,samplesCnt
	mov r4,#0
	adds r3,r2,r0,lsl#SHIFTVAL-(8-SN_UPSHIFT)	;@ Only use top 11 bits
	movcs r4,r3,lsr#SHIFTVAL
	subcs r3,r3,r4,lsl#SHIFTVAL
	str r3,sndWritePtr
	mov r2,r2,lsr#SHIFTVAL
	rsb r0,r2,r3,lsr#SHIFTVAL
	and r0,#7
	add r1,r1,r2,lsl#2
//	subcs r0,r0,r3,lsr#SHIFTVAL-(8-SN_UPSHIFT)

	ldr r2,=SN76496_0
	bl sn76496Mixer
	cmp r4,#0
	ldmfdeq sp!,{r3,r4,pc}

	ldr r1,=WAVBUFFER
	ldr r3,sndWritePtr
	add r3,r3,r4,lsl#SHIFTVAL
	str r3,sndWritePtr
	mov r0,r4
	bl sn76496Mixer
	ldmfd sp!,{r3,r4,pc}

;@----------------------------------------------------------------------------
sndWritePtr:	.long 0
pcmWritePtr:	.long 0
pcmReadPtr:		.long 0
neededExtra:	.long 0
samplesCnt:		.long 228

muteSound:
muteSoundGUI:
	.byte 0
muteSoundGame:
	.byte 0
	.space 2
SMSJSoundControl:
	.byte 0
	.skip 3


	.section .bss
	.align 2
SoundVariables:
SN76496_0:
	.space snSize
SN76496_1:
	.space snSize
YM2413_0:
	.space ymSize
AY38910_0:
	.space aySize
SCC_0:
	.space sccSize
mixSpace0:
	.space 0x8000
mixSpace1:
	.space 0x8000
WAVBUFFER:
	.space SOUND_BUFFER_SIZE*4
SoundVariablesEnd:
;@----------------------------------------------------------------------------
	.end
#endif // __arm__
