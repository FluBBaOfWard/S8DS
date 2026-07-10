#include <nds.h>

#include "MasterSystem.h"
#include "Main.h"
#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "SG1000.h"
#include "OmvBgr.h"
#include "SG1000-II.h"
#include "SC3000.h"
#include "Mark3.h"
#include "SMS1.h"
#include "SMS2.h"
#include "GG.h"
#include "MD.h"
#include "Cart.h"
#include "Sound.h"
#include "Gfx.h"
#include "io.h"
#include "ARMZ80/ARMZ80.h"


int smsPackState(void *statePtr) {
	int size = 0;
	size += Z80SaveState(statePtr+size, &Z80OpTable);
	size += VDPSaveState(statePtr+size, &VDP0);
	size += sn76496SaveState(statePtr+size, &SN76496_0);
	size += cartSaveState(statePtr+size);
	return size;
}

void smsUnpackState(const void *statePtr) {
	int size = 0;
	size += Z80LoadState(&Z80OpTable, statePtr+size);
	size += VDPLoadState(&VDP0, statePtr+size);
	size += sn76496LoadState(&SN76496_0, statePtr+size);
	size += cartLoadState(statePtr+size);
}

int smsGetStateSize() {
	int size = 0;
	size += Z80GetStateSize();
	size += VDPGetStateSize();
	size += sn76496GetStateSize();
	size += cartGetStateSize();
	return size;
}

void nullUISG1000(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>4);
		if ((xpos > 51) && (ypos < 2)) {
			openMenu();
		}
		else if ((ypos > 4 && ypos < 6) && (xpos > 21 && xpos < 43)) {	// Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos == 10) && (xpos > 46 && xpos < 52)) {	// Hold button
			EMUinput |= KEY_START;
		}
	}
}

void nullUIOMV(int keyHit) {
	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>3);
		int ypos = (myTouch.py>>3);
		if ((xpos > 25) && (ypos < 4)) {
			openMenu();
		}
		else if ((ypos > 5 && ypos < 9) && (xpos > 10 && xpos < 21)) {	// Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if (ypos == 13 || ypos == 14) {
			if (xpos > 3 && xpos < 6)
				sc3Keyboard = 0x13;			// Port 1, Bit 3 = 0 *
			else if (xpos > 5 && xpos < 8)
				sc3Keyboard = 0x20;			// Port 2, Bit 0 = A
			else if (xpos > 7 && xpos < 10)
				sc3Keyboard = 0x21;			// Port 2, Bit 1 = B
			else if (xpos > 9 && xpos < 12)
				sc3Keyboard = 0x22;			// Port 2, Bit 2 = C
			else if (xpos > 11 && xpos < 14)
				sc3Keyboard = 0x23;			// Port 2, Bit 3 = D
			else if (xpos > 13 && xpos < 16)
				sc3Keyboard = 0x24;			// Port 2, Bit 4 = E
			else if (xpos > 15 && xpos < 18)
				sc3Keyboard = 0x25;			// Port 2, Bit 5 = F
			else if (xpos > 17 && xpos < 20)
				sc3Keyboard = 0x30;			// Port 3, Bit 0 = G
			else if (xpos > 19 && xpos < 22)
				sc3Keyboard = 0x31;			// Port 3, Bit 1 = H
			else if (xpos > 21 && xpos < 24)
				sc3Keyboard = 0x12;			// Port 1, Bit 2 = 9 #
			else if (xpos > 24 && xpos < 30) {
				EMUinput |= KEY_START;		// NMI = Reset
			}
		} else if (ypos == 15 ||ypos == 16) {
			if (xpos > 3 && xpos < 6)
				sc3Keyboard = 0x13;			// Port 1, Bit 3 = 0 *
			else if (xpos > 5 && xpos < 8)
				sc3Keyboard = 0x00;			// Port 0, Bit 0 = 1
			else if (xpos > 7 && xpos < 10)
				sc3Keyboard = 0x01;			// Port 0, Bit 1 = 2
			else if (xpos > 9 && xpos < 12)
				sc3Keyboard = 0x02;			// Port 0, Bit 2 = 3
			else if (xpos > 11 && xpos < 14)
				sc3Keyboard = 0x03;			// Port 0, Bit 3 = 4
			else if (xpos > 13 && xpos < 16)
				sc3Keyboard = 0x04;			// Port 0, Bit 4 = 5
			else if (xpos > 15 && xpos < 18)
				sc3Keyboard = 0x05;			// Port 0, Bit 5 = 6
			else if (xpos > 17 && xpos < 20)
				sc3Keyboard = 0x10;			// Port 1, Bit 0 = 7
			else if (xpos > 19 && xpos < 22)
				sc3Keyboard = 0x11;			// Port 1, Bit 1 = 8
			else if (xpos > 21 && xpos < 24)
				sc3Keyboard = 0x12;			// Port 1, Bit 2 = 9 #
		} else if (ypos == 18 || ypos ==19) {
			if (xpos > 7 && xpos < 12) {
				EMUinput |= KEY_B;			// Player 1, button 1
			}
			if (xpos > 15 && xpos < 20) {
				EMUinput |= KEY_A;			// Player 1, button 2
			}
		}
	}
}

void nullUISC3000(int keyHit) {
	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>4);
		if ((xpos > 51) && (ypos < 2)) {
			openMenu();
		}
		else if (ypos == 6) {
			if (xpos > 1 && xpos < 6)
				sc3Keyboard = 0x00;			// Row 0, Col 0 = 1
			else if (xpos > 5 && xpos < 10)
				sc3Keyboard = 0x10;			// Row 1, Col 0 = 2
			else if (xpos > 9 && xpos < 14)
				sc3Keyboard = 0x20;			// Row 2, Col 0 = 3
			else if (xpos > 13 && xpos < 18)
				sc3Keyboard = 0x30;			// Row 3, Col 0 = 4
			else if (xpos > 17 && xpos < 22)
				sc3Keyboard = 0x40;			// Row 4, Col 0 = 5
			else if (xpos > 21 && xpos < 26)
				sc3Keyboard = 0x50;			// Row 5, Col 0 = 6
			else if (xpos > 25 && xpos < 30)
				sc3Keyboard = 0x60;			// Row 6, Col 0 = 7
			else if (xpos > 29 && xpos < 34)
				sc3Keyboard = 0x08;			// Row 0, Col 8 = 8
			else if (xpos > 33 && xpos < 38)
				sc3Keyboard = 0x18;			// Row 1, Col 8 = 9
			else if (xpos > 37 && xpos < 42)
				sc3Keyboard = 0x28;			// Row 2, Col 8 = 0
			else if (xpos > 41 && xpos < 46)
				sc3Keyboard = 0x38;			// Row 3, Col 8 = -
			else if (xpos > 45 && xpos < 50)
				sc3Keyboard = 0x48;			// Row 4, Col 8 = ^
			else if (xpos > 49 && xpos < 54)
				sc3Keyboard = 0x58;			// Row 5, Col 8 = Yen
			else if (xpos > 54 && xpos < 59)
				sc3Keyboard = 0x68;			// Row 6, Col 8 = Brk
			else if (xpos > 59 && xpos < 64) {
				EMUinput |= KEY_START;		// NMI = Reset
			}
		}
		else if (ypos == 7) {
			if (xpos < 4)
				sc3Keyboard = 0x5B;			// Row 5, Col 11 = Func
			else if (xpos > 3 && xpos < 8)
				sc3Keyboard = 0x01;			// Row 0, Col 1 = Q
			else if (xpos > 7 && xpos < 12)
				sc3Keyboard = 0x11;			// Row 1, Col 1 = W
			else if (xpos > 11 && xpos < 16)
				sc3Keyboard = 0x21;			// Row 2, Col 1 = E
			else if (xpos > 15 && xpos < 20)
				sc3Keyboard = 0x31;			// Row 3, Col 1 = R
			else if (xpos > 19 && xpos < 24)
				sc3Keyboard = 0x41;			// Row 4, Col 1 = T
			else if (xpos > 23 && xpos < 28)
				sc3Keyboard = 0x51;			// Row 5, Col 1 = Y
			else if (xpos > 27 && xpos < 32)
				sc3Keyboard = 0x61;			// Row 6, Col 1 = U
			else if (xpos > 31 && xpos < 36)
				sc3Keyboard = 0x07;			// Row 0, Col 7 = I
			else if (xpos > 35 && xpos < 40)
				sc3Keyboard = 0x17;			// Row 1, Col 7 = O
			else if (xpos > 39 && xpos < 44)
				sc3Keyboard = 0x27;			// Row 2, Col 7 = P
			else if (xpos > 43 && xpos < 48)
				sc3Keyboard = 0x37;			// Row 3, Col 7 = @
			else if (xpos > 47 && xpos < 52)
				sc3Keyboard = 0x47;			// Row 4, Col 7 = [
			else if (xpos > 51 && xpos < 57)
				sc3Keyboard = 0x56;			// Row 5, Col 6 = CR (Enter)
		}
		else if (ypos == 8) {
			if (xpos > 0 && xpos < 5)
				sc3Keyboard = 0x6A;			// Row 6, Col 10 = CTRL
			else if (xpos > 4 && xpos < 9)
				sc3Keyboard = 0x02;			// Row 0, Col 2 = A
			else if (xpos > 8 && xpos < 13)
				sc3Keyboard = 0x12;			// Row 1, Col 2 = S
			else if (xpos > 12 && xpos < 17)
				sc3Keyboard = 0x22;			// Row 2, Col 2 = D
			else if (xpos > 16 && xpos < 21)
				sc3Keyboard = 0x32;			// Row 3, Col 2 = F
			else if (xpos > 20 && xpos < 25)
				sc3Keyboard = 0x42;			// Row 4, Col 2 = G
			else if (xpos > 24 && xpos < 29)
				sc3Keyboard = 0x52;			// Row 5, Col 2 = H
			else if (xpos > 28 && xpos < 33)
				sc3Keyboard = 0x62;			// Row 6, Col 2 = J
			else if (xpos > 32 && xpos < 37)
				sc3Keyboard = 0x06;			// Row 0, Col 6 = K
			else if (xpos > 36 && xpos < 41)
				sc3Keyboard = 0x16;			// Row 1, Col 6 = L
			else if (xpos > 40 && xpos < 45)
				sc3Keyboard = 0x26;			// Row 2, Col 6 = ;
			else if (xpos > 44 && xpos < 49)
				sc3Keyboard = 0x36;			// Row 3, Col 6 = :
			else if (xpos > 48 && xpos < 53)
				sc3Keyboard = 0x46;			// Row 4, Col 6 = ]
			else if (xpos > 52 && xpos < 57)
				sc3Keyboard = 0x56;			// Row 5, Col 6 = CR (Enter
			else if (xpos > 57 && xpos < 62)
				sc3Keyboard = 0x66;			// Row 6, Col 6 = Up Arrow
		}
		else if (ypos == 9) {
			if (xpos > 0 && xpos < 7)
				sc3Keyboard = 0x6B;			// Row 6, Col 11 = Shift
			else if (xpos > 6 && xpos < 11)
				sc3Keyboard = 0x03;			// Row 0, Col 3 = Z
			else if (xpos > 10 && xpos < 15)
				sc3Keyboard = 0x13;			// Row 1, Col 3 = X
			else if (xpos > 14 && xpos < 19)
				sc3Keyboard = 0x23;			// Row 2, Col 3 = C
			else if (xpos > 18 && xpos < 23)
				sc3Keyboard = 0x33;			// Row 3, Col 3 = V
			else if (xpos > 22 && xpos < 27)
				sc3Keyboard = 0x43;			// Row 4, Col 3 = B
			else if (xpos > 26 && xpos < 31)
				sc3Keyboard = 0x53;			// Row 5, Col 3 = N
			else if (xpos > 30 && xpos < 35)
				sc3Keyboard = 0x63;			// Row 6, Col 3 = M
			else if (xpos > 34 && xpos < 39)
				sc3Keyboard = 0x05;			// Row 0, Col 5 = ,
			else if (xpos > 38 && xpos < 43)
				sc3Keyboard = 0x15;			// Row 1, Col 5 = .
			else if (xpos > 42 && xpos < 47)
				sc3Keyboard = 0x25;			// Row 2, Col 5 = /
			else if (xpos > 46 && xpos < 51)
				sc3Keyboard = 0x35;			// Row 3, Col 5 = PI
			else if (xpos > 50 && xpos < 57)
				sc3Keyboard = 0x6B;			// Row 6, Col 11 = Shift
			else if (xpos > 57 && xpos < 62)
				sc3Keyboard = 0x45;			// Row 4, Col 5 = Down Arrow
		}
		else if (ypos == 10) {
			if (xpos > 4 && xpos < 9)
				sc3Keyboard = 0x69;			// Row 6, Col 9 = Graph
			else if (xpos > 8 && xpos < 13)
				sc3Keyboard = 0x04;			// Row 0, Col 4 = ENG DIER'S
			else if (xpos > 12 && xpos < 45)
				sc3Keyboard = 0x14;			// Row 1, Col 4 = Space
			else if (xpos > 44 && xpos < 49)
				sc3Keyboard = 0x24;			// Row 2, Col 4 = HOME CLR
			else if (xpos > 48 && xpos < 53)
				sc3Keyboard = 0x34;			// Row 3, Col 4 = INS DEL
			else if (xpos > 55 && xpos < 60)
				sc3Keyboard = 0x55;			// Row 5, Col 5 = Left Arrow
			else if (xpos > 59 && xpos < 64)
				sc3Keyboard = 0x65;			// Row 6, Col 5 = Right Arrow
		}
	}
}

void nullUISG1000II(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
			openMenu();
		}
		else if ((ypos > 27 && ypos < 33) && (xpos > 37 && xpos < 55)) {	// Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos > 36 && ypos < 39) && (xpos > 42 && xpos < 49)) {	// Hold button
			EMUinput |= KEY_START;
		}
	}
}

void nullUIMark3(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>3);
		if ((xpos > 51) && (ypos < 4)) {
			openMenu();
		}
		else if ((ypos > 11 && ypos < 15) && (xpos > 35 && xpos < 57)) {	// Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos == 16) && (xpos > 36 && xpos < 41)) {	// Hold button
			EMUinput |= KEY_START;
		}
	}
}

void nullUISMS1(int keyHit) {
	static bool prePower = false;

	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
			openMenu();
		}
		else if ((ypos > 25 && ypos < 30) && (xpos > 31 && xpos < 54)) {	// Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if (ypos > 30 && ypos < 34) {
			if (xpos > 32 && xpos < 38) {	// Reset button
				EMUinput |= KEY_SELECT;
			}
			else if (xpos > 48 && xpos < 54) {	// Pause button
				EMUinput |= KEY_START;
			}
		}
		else if (ypos > 45 && xpos > 4 && xpos < 11) {	// Power button
			if (keyHit & KEY_TOUCH) {
				if (!powerIsOn) {
					powerOnOff();
				} else if (!prePower) {
					prePower = true;
				}
			}
		}
	}
	else if (prePower && powerIsOn) {
		powerOnOff();
		prePower = false;
	}
}

void nullUISMS2(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
			openMenu();
		}
		else if ((ypos > 18 && ypos < 30) && (xpos > 18 && xpos < 52)) { // Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos > 24 && ypos < 32) && (xpos > 4 && xpos < 13)) { // Pause button
			EMUinput |= KEY_START;
		}
		else if (ypos > 40 && ypos < 43 && xpos > 5 && xpos < 12) { // Power button
			if (keyHit & KEY_TOUCH) {
				powerOnOff();
			}
		}
	}
}

void nullUIMD(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
			openMenu();
		}
		else if ((ypos > 12 && ypos < 18) && (xpos > 26 && xpos < 50)) { // Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos > 25 && ypos < 29) && (xpos > 28 && xpos < 33)) { // Pause button
			EMUinput |= KEY_START;
		}
		else if ((ypos > 41 && ypos < 46) && (xpos > 14 && xpos < 19)) { // Reset button
			EMUinput |= KEY_SELECT;
		}
		else if ((ypos > 34 && ypos < 38) && (xpos > 14 && xpos < 19)) { // Power button
			if (keyHit & KEY_TOUCH) {
				powerOnOff();
			}
		}
	}
}

void setupSG1000Background(void) {
	setupCompressedBackground(SG1000Tiles, SG1000Map, 6);
	memcpy(BG_PALETTE_SUB+0x80, SG1000Pal, SG1000PalLen);
}

void setupOMVBackground(void) {
	setupCompressedBackground(OmvBgrTiles, OmvBgrMap, 4);
	memcpy(BG_PALETTE_SUB+0x80, OmvBgrPal, OmvBgrPalLen);
}

void setupSC3000Background(void) {
	setupCompressedBackground(SC3000Tiles, SC3000Map, 6);
	memcpy(BG_PALETTE_SUB+0x80, SC3000Pal, SC3000PalLen);
}

void setupSG1000IIBackground(void) {
	setupCompressedBackground(SG1000_IITiles, SG1000_IIMap, 8);
	memcpy(BG_PALETTE_SUB+0x80, SG1000_IIPal, SG1000_IIPalLen);
}

void setupMARK3Background(void) {
	setupCompressedBackground(Mark3Tiles, Mark3Map, 9);
	memcpy(BG_PALETTE_SUB+0x80, Mark3Pal, Mark3PalLen);
}

void setupSMS1Background(void) {
	setupCompressedBackground(SMS1Tiles, SMS1Map, 9);
	memcpy(BG_PALETTE_SUB+0x80, SMS1Pal, SMS1PalLen);
}

void setupSMS2Background(void) {
	setupCompressedBackground(SMS2Tiles, SMS2Map, 2);
	memcpy(BG_PALETTE_SUB+0x80, SMS2Pal, SMS2PalLen);
}

void setupGGBackground(void) {
	setupCompressedBackground(GGTiles, GGMap, 6);
	memcpy(BG_PALETTE_SUB+0x80, GGPal, GGPalLen);
}

void setupMDBackground(void) {
	setupCompressedBackground(MDTiles, MDMap, 0);
	memcpy(BG_PALETTE_SUB+0x80, MDPal, MDPalLen);
}
