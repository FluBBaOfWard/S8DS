#include <nds.h>

#include "Coleco.h"
#include "Main.h"
#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "ColecoNumpad.h"
#include "Cart.h"
#include "Sound.h"
#include "Gfx.h"
#include "io.h"
#include "ARMZ80/ARMZ80.h"


int colecoPackState(void *statePtr) {
	int size = 0;
	size += Z80SaveState(statePtr+size, &Z80OpTable);
	size += VDPSaveState(statePtr+size, &VDP0);
	size += sn76496SaveState(statePtr+size, &SN76496_0);
	size += cartSaveState(statePtr+size);
	return size;
}

void colecoUnpackState(const void *statePtr) {
	int size = 0;
	size += Z80LoadState(&Z80OpTable, statePtr+size);
	size += VDPLoadState(&VDP0, statePtr+size);
	size += sn76496LoadState(&SN76496_0, statePtr+size);
	size += cartLoadState(statePtr+size);
}

int colecoGetStateSize() {
	int size = 0;
	size += Z80GetStateSize();
	size += VDPGetStateSize();
	size += sn76496GetStateSize();
	size += cartGetStateSize();
	return size;
}

void nullUIColeco(int keyHit) {
	colecoKey = 0;
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>3);
		int ypos = (myTouch.py>>3);
		if ((xpos > 25) && (ypos < 3)) {
			openMenu();
		}
		else if (xpos > 7 && xpos < 12) {
			if (ypos > 0 && ypos < 5)
				colecoKey = 0x02;
			else if (ypos > 6 && ypos < 11)
				colecoKey = 0x0D;
			else if (ypos > 12 && ypos < 17)
				colecoKey = 0x0A;
			else if (ypos > 18 && ypos < 23)
				colecoKey = 0x06;
		}
		else if (xpos > 13 && xpos < 18) {
			if (ypos > 0 && ypos < 5)
				colecoKey = 0x08;
			else if (ypos > 6 && ypos < 11)
				colecoKey = 0x0C;
			else if (ypos > 12 && ypos < 17)
				colecoKey = 0x0E;
			else if (ypos > 18 && ypos < 23)
				colecoKey = 0x05;
		}
		else if (xpos > 19 && xpos < 24) {
			if (ypos > 0 && ypos < 5)
				colecoKey = 0x03;
			else if (ypos > 6 && ypos < 11)
				colecoKey = 0x01;
			else if (ypos > 12 && ypos < 17)
				colecoKey = 0x04;
			else if (ypos > 18 && ypos < 23)
				colecoKey = 0x09;
		}
	}
}

void colecoSetupBackground(void) {
	setupCompressedBackground(ColecoNumpadTiles, ColecoNumpadMap, 0);
	memcpy(BG_PALETTE_SUB+0x80, ColecoNumpadPal, ColecoNumpadPalLen);
}
