#include <nds.h>

#include "MSX.h"
#include "Main.h"
#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "MSXKb.h"
#include "Cart.h"
#include "Sound.h"
#include "Gfx.h"
#include "io.h"
#include "ARMZ80/ARMZ80.h"


int msxPackState(void *statePtr) {
	int size = 0;
	size += Z80SaveState(statePtr+size, &Z80OpTable);
	size += VDPSaveState(statePtr+size, &VDP0);
	size += ay38910SaveState(statePtr+size, &AY38910_0);
	size += SCCSaveState(statePtr+size, &SCC_0);
	size += cartSaveState(statePtr+size);
//	size += sn76496SaveState(statePtr+size, &SN76496_0);
	return size;
}

void msxUnpackState(const void *statePtr) {
	int size = 0;
	size += Z80LoadState(&Z80OpTable, statePtr+size);
	size += VDPLoadState(&VDP0, statePtr+size);
	size += ay38910LoadState(&AY38910_0, statePtr+size);
	size += SCCLoadState(&SCC_0, statePtr+size);
	size += cartLoadState(statePtr+size);
//	size += sn76496LoadState(&SN76496_0, statePtr+size);
}

int msxGetStateSize() {
	int size = 0;
	size += Z80GetStateSize();
	size += VDPGetStateSize();
	size += ay38910GetStateSize();
	size += SCCGetStateSize();
	size += cartGetStateSize();
//	size += sn76496GetStateSize();
	return size;
}

void nullUIMSX(int keyHit) {
	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>4);
		if ((xpos > 51) && (ypos < 2)) {
			openMenu();
		}
		else if (ypos == 5) {
			if (xpos > 1 && xpos < 8)
				sc3Keyboard = 0x65;			// Row 6, Col 5 = F1
			else if (xpos > 7 && xpos < 14)
				sc3Keyboard = 0x66;			// Row 6, Col 6 = F2
			else if (xpos > 13 && xpos < 20)
				sc3Keyboard = 0x67;			// Row 6, Col 7 = F3
			else if (xpos > 19 && xpos < 26)
				sc3Keyboard = 0x70;			// Row 7, Col 0 = F4
			else if (xpos > 25 && xpos < 32)
				sc3Keyboard = 0x71;			// Row 7, Col 1 = F5
			else if (xpos > 31 && xpos < 38)
				sc3Keyboard = 0x76;			// Row 7, Col 6 = Select
			else if (xpos > 37 && xpos < 44)
				sc3Keyboard = 0x74;			// Row 7, Col 4 = Stop
			else if (xpos > 43 && xpos < 50)
				sc3Keyboard = 0x81;			// Row 8, Col 1 = Home
			else if (xpos > 49 && xpos < 56)
				sc3Keyboard = 0x82;			// Row 8, Col 2 = Insert
			else if (xpos > 55 && xpos < 62)
				sc3Keyboard = 0x83;			// Row 8, Col 3 = Delete
		}
		else if (ypos == 6) {
			if (xpos > 1 && xpos < 6)
				sc3Keyboard = 0x72;			// Row 7, Col 2 = ESC
			else if (xpos > 5 && xpos < 10)
				sc3Keyboard = 0x01;			// Row 0, Col 1 = 1
			else if (xpos > 9 && xpos < 14)
				sc3Keyboard = 0x02;			// Row 0, Col 2 = 2
			else if (xpos > 13 && xpos < 18)
				sc3Keyboard = 0x03;			// Row 0, Col 3 = 3
			else if (xpos > 17 && xpos < 22)
				sc3Keyboard = 0x04;			// Row 0, Col 4 = 4
			else if (xpos > 21 && xpos < 26)
				sc3Keyboard = 0x05;			// Row 0, Col 5 = 5
			else if (xpos > 25 && xpos < 30)
				sc3Keyboard = 0x06;			// Row 0, Col 6 = 6
			else if (xpos > 29 && xpos < 34)
				sc3Keyboard = 0x07;			// Row 0, Col 7 = 7
			else if (xpos > 33 && xpos < 38)
				sc3Keyboard = 0x10;			// Row 1, Col 0 = 8
			else if (xpos > 37 && xpos < 42)
				sc3Keyboard = 0x11;			// Row 1, Col 1 = 9
			else if (xpos > 41 && xpos < 46)
				sc3Keyboard = 0x00;			// Row 0, Col 0 = 0
			else if (xpos > 45 && xpos < 50)
				sc3Keyboard = 0x12;			// Row 1, Col 2 = -
			else if (xpos > 49 && xpos < 54)
				sc3Keyboard = 0x13;			// Row 1, Col 3 = =
			else if (xpos > 53 && xpos < 58)
				sc3Keyboard = 0x14;			// Row 1, Col 4 = "\"
			else if (xpos > 57 && xpos < 62)
				sc3Keyboard = 0x75;			// Row 7, Col 5 = BackSpace
		}
		else if (ypos == 7) {
			if (xpos > 1 && xpos < 8)
				sc3Keyboard = 0x73;			// Row 7, Col 3 = Tab
			else if (xpos > 7 && xpos < 12)
				sc3Keyboard = 0x46;			// Row 4, Col 6 = Q
			else if (xpos > 11 && xpos < 16)
				sc3Keyboard = 0x54;			// Row 5, Col 4 = W
			else if (xpos > 15 && xpos < 20)
				sc3Keyboard = 0x32;			// Row 3, Col 2 = E
			else if (xpos > 19 && xpos < 24)
				sc3Keyboard = 0x47;			// Row 4, Col 7 = R
			else if (xpos > 23 && xpos < 28)
				sc3Keyboard = 0x51;			// Row 5, Col 1 = T
			else if (xpos > 27 && xpos < 32)
				sc3Keyboard = 0x56;			// Row 5, Col 6 = Y
			else if (xpos > 31 && xpos < 36)
				sc3Keyboard = 0x52;			// Row 5, Col 2 = U
			else if (xpos > 35 && xpos < 40)
				sc3Keyboard = 0x36;			// Row 3, Col 6 = I
			else if (xpos > 39 && xpos < 44)
				sc3Keyboard = 0x44;			// Row 4, Col 4 = O
			else if (xpos > 43 && xpos < 48)
				sc3Keyboard = 0x45;			// Row 4, Col 5 = P
			else if (xpos > 47 && xpos < 52)
				sc3Keyboard = 0x15;			// Row 1, Col 5 = [
			else if (xpos > 51 && xpos < 56)
				sc3Keyboard = 0x16;			// Row 1, Col 6 = ]
			else if (xpos > 55 && xpos < 62)
				sc3Keyboard = 0x77;			// Row 7, Col 7 = Ret (Enter)
		}
		else if (ypos == 8) {
			if (xpos > 1 && xpos < 9)
				sc3Keyboard = 0x61;			// Row 6, Col 1 = CTRL
			else if (xpos > 8 && xpos < 13)
				sc3Keyboard = 0x26;			// Row 2, Col 6 = A
			else if (xpos > 12 && xpos < 17)
				sc3Keyboard = 0x50;			// Row 5, Col 0 = S
			else if (xpos > 16 && xpos < 21)
				sc3Keyboard = 0x31;			// Row 3, Col 1 = D
			else if (xpos > 20 && xpos < 25)
				sc3Keyboard = 0x33;			// Row 3, Col 3 = F
			else if (xpos > 24 && xpos < 29)
				sc3Keyboard = 0x34;			// Row 3, Col 4 = G
			else if (xpos > 28 && xpos < 33)
				sc3Keyboard = 0x35;			// Row 3, Col 5 = H
			else if (xpos > 32 && xpos < 37)
				sc3Keyboard = 0x37;			// Row 3, Col 7 = J
			else if (xpos > 36 && xpos < 41)
				sc3Keyboard = 0x40;			// Row 4, Col 0 = K
			else if (xpos > 40 && xpos < 45)
				sc3Keyboard = 0x41;			// Row 4, Col 1 = L
			else if (xpos > 44 && xpos < 49)
				sc3Keyboard = 0x17;			// Row 1, Col 7 = ;
			else if (xpos > 48 && xpos < 53)
				sc3Keyboard = 0x20;			// Row 2, Col 0 = `
			else if (xpos > 52 && xpos < 57)
				sc3Keyboard = 0x21;			// Row 2, Col 1 = '
			else if (xpos > 56 && xpos < 62)
				sc3Keyboard = 0x77;			// Row 7, Col 7 = Ret (Enter)
		}
		else if (ypos == 9) {
			if (xpos > 1 && xpos < 11)
				sc3Keyboard = 0x60;			// Row 6, Col 0 = Shift
			else if (xpos > 10 && xpos < 15)
				sc3Keyboard = 0x57;			// Row 5, Col 7 = Z
			else if (xpos > 14 && xpos < 19)
				sc3Keyboard = 0x55;			// Row 5, Col 5 = X
			else if (xpos > 18 && xpos < 23)
				sc3Keyboard = 0x30;			// Row 3, Col 0 = C
			else if (xpos > 22 && xpos < 27)
				sc3Keyboard = 0x53;			// Row 5, Col 3 = V
			else if (xpos > 26 && xpos < 31)
				sc3Keyboard = 0x27;			// Row 2, Col 7 = B
			else if (xpos > 30 && xpos < 35)
				sc3Keyboard = 0x43;			// Row 4, Col 3 = N
			else if (xpos > 34 && xpos < 39)
				sc3Keyboard = 0x42;			// Row 4, Col 2 = M
			else if (xpos > 38 && xpos < 43)
				sc3Keyboard = 0x22;			// Row 2, Col 2 = ,
			else if (xpos > 42 && xpos < 47)
				sc3Keyboard = 0x23;			// Row 2, Col 3 = .
			else if (xpos > 46 && xpos < 51)
				sc3Keyboard = 0x24;			// Row 2, Col 4 = /
			else if (xpos > 50 && xpos < 55)
				sc3Keyboard = 0x25;			// Row 2, Col 5 = ?
			else if (xpos > 54 && xpos < 62)
				sc3Keyboard = 0x60;			// Row 6, Col 0 = Shift
		}
		else if (ypos == 10) {
			if (xpos > 8 && xpos < 13)
				sc3Keyboard = 0x63;			// Row 6, Col 3 = Cap
			else if (xpos > 12 && xpos < 17)
				sc3Keyboard = 0x62;			// Row 6, Col 2 = Graph
			else if (xpos > 16 && xpos < 49)
				sc3Keyboard = 0x80;			// Row 8, Col 0 = Space
			else if (xpos > 48 && xpos < 53)
				sc3Keyboard = 0x64;			// Row 6, Col 4 = Code/Kana
			else if (xpos > 55 && xpos < 60)
				sc3Keyboard = 0x84;			// Row 8, Col 4 = Left
			else if (xpos > 59 && xpos < 64)
				sc3Keyboard = 0x87;			// Row 8, Col 7 = Right
		}
		else if (ypos == 11) {
			if (xpos > 55 && xpos < 60)
				sc3Keyboard = 0x85;			// Row 8, Col 5 = Up
			else if (xpos > 59 && xpos < 64)
				sc3Keyboard = 0x86;			// Row 8, Col 6 = Down
		}
	}
}

void msxSetupBackground(void) {
	setupCompressedBackground(MSXKbTiles, MSXKbMap, 6);
	memcpy(BG_PALETTE_SUB+0x80, MSXKbPal, MSXKbPalLen);
}
