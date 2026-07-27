#include <nds.h>

#include "SordM5.h"
#include "Main.h"
#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "SordM5Kb.h"
#include "Cart.h"
#include "Sound.h"
#include "Gfx.h"
#include "io.h"
#include "ARMZ80/ARMZ80.h"
#include "ARMZ80CTC/ARMZ80CTC.h"

extern u8 sordM5Input;		// SordM5IO.s
extern Z80CTC CTC_0;		// SordM5IO.s

int sordM5PackState(void *statePtr) {
	int size = 0;
	size += Z80SaveState(statePtr+size, &Z80OpTable);
	size += VDPSaveState(statePtr+size, &VDP0);
	size += sn76496SaveState(statePtr+size, &SN76496_0);
	size += Z80CTCSaveState(statePtr+size, &CTC_0);
	size += cartSaveState(statePtr+size);
	return size;
}

void sordM5UnpackState(const void *statePtr) {
	int size = 0;
	size += Z80LoadState(&Z80OpTable, statePtr+size);
	size += VDPLoadState(&VDP0, statePtr+size);
	size += sn76496LoadState(&SN76496_0, statePtr+size);
	size += Z80CTCLoadState(&CTC_0, statePtr+size);
	size += cartLoadState(statePtr+size);
}

int sordM5GetStateSize() {
	int size = 0;
	size += Z80GetStateSize();
	size += VDPGetStateSize();
	size += sn76496GetStateSize();
	size += Z80CTCGetStateSize();
	size += cartGetStateSize();
	return size;
}

void nullUISordM5(int keyHit) {
	keyboardRows[0] &= ~0xC0;
	keyboardRows[1] = 0;
	keyboardRows[2] = 0;
	keyboardRows[3] = 0;
	keyboardRows[4] = 0;
	keyboardRows[5] = 0;
	keyboardRows[6] = 0;
	keyboardRows[7] = 0;
	sordM5Input &= ~0x80;
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>4);
		if ((xpos > 51) && (ypos < 2)) {
			openMenu();
		}
		else if (ypos == 7) {
			if (xpos > 3 && xpos < 8)
				keyboardRows[1] |= 0x01;	// Row 1, Col 0 = 1
			else if (xpos > 7 && xpos < 12)
				keyboardRows[1] |= 0x02;	// Row 1, Col 1 = 2
			else if (xpos > 11 && xpos < 16)
				keyboardRows[1] |= 0x04;	// Row 1, Col 2 = 3
			else if (xpos > 15 && xpos < 20)
				keyboardRows[1] |= 0x08;	// Row 1, Col 3 = 4
			else if (xpos > 19 && xpos < 24)
				keyboardRows[1] |= 0x10;	// Row 1, Col 4 = 5
			else if (xpos > 23 && xpos < 28)
				keyboardRows[1] |= 0x20;	// Row 1, Col 5 = 6
			else if (xpos > 27 && xpos < 32)
				keyboardRows[1] |= 0x40;	// Row 1, Col 6 = 7
			else if (xpos > 31 && xpos < 36)
				keyboardRows[1] |= 0x80;	// Row 1, Col 7 = 8
			else if (xpos > 35 && xpos < 40)
				keyboardRows[5] |= 0x01;	// Row 5, Col 0 = 9
			else if (xpos > 39 && xpos < 44)
				keyboardRows[5] |= 0x02;	// Row 5, Col 1 = 0
			else if (xpos > 43 && xpos < 48)
				keyboardRows[5] |= 0x04;	// Row 5, Col 2 = -
			else if (xpos > 47 && xpos < 52)
				keyboardRows[5] |= 0x08;	// Row 5, Col 3 = =
			else if (xpos > 51 && xpos < 56)
				keyboardRows[5] |= 0x80;	// Row 5, Col 7 = "\"
			else if (xpos > 56 && xpos < 61)
				sordM5Input |= 0x80;		// Row -, Col - = Reset
		}
		else if (ypos == 8) {
			if (xpos > 1 && xpos < 6 && (keyHit & KEY_TOUCH))
				keyboardRows[0] ^= 0x02;	// Row 0, Col 1 = Func
			else if (xpos > 5 && xpos < 10)
				keyboardRows[2] |= 0x01;	// Row 2, Col 0 = Q
			else if (xpos > 9 && xpos < 14)
				keyboardRows[2] |= 0x02;	// Row 2, Col 1 = W
			else if (xpos > 13 && xpos < 18)
				keyboardRows[2] |= 0x04;	// Row 2, Col 2 = E
			else if (xpos > 17 && xpos < 22)
				keyboardRows[2] |= 0x08;	// Row 2, Col 3 = R
			else if (xpos > 21 && xpos < 26)
				keyboardRows[2] |= 0x10;	// Row 2, Col 4 = T
			else if (xpos > 25 && xpos < 30)
				keyboardRows[2] |= 0x20;	// Row 2, Col 5 = Y
			else if (xpos > 29 && xpos < 34)
				keyboardRows[2] |= 0x40;	// Row 2, Col 6 = U
			else if (xpos > 33 && xpos < 38)
				keyboardRows[2] |= 0x80;	// Row 2, Col 7 = I
			else if (xpos > 37 && xpos < 42)
				keyboardRows[6] |= 0x01;	// Row 6, Col 0 = O
			else if (xpos > 41 && xpos < 46)
				keyboardRows[6] |= 0x02;	// Row 6, Col 1 = P
			else if (xpos > 45 && xpos < 50)
				keyboardRows[6] |= 0x04;	// Row 6, Col 2 = @
			else if (xpos > 49 && xpos < 54)
				keyboardRows[6] |= 0x08;	// Row 6, Col 3 = [
			else if (xpos > 53 && xpos < 61)
				keyboardRows[0] |= 0x80;	// Row 0, Col 7 = Ret (Enter)
		}
		else if (ypos == 9) {
			if (xpos > 2 && xpos < 7 && (keyHit & KEY_TOUCH))
				keyboardRows[0] ^= 0x01;	// Row 0, Col 0 = CTRL
			else if (xpos > 6 && xpos < 11)
				keyboardRows[3] |= 0x01;	// Row 3, Col 0 = A
			else if (xpos > 10 && xpos < 15)
				keyboardRows[3] |= 0x02;	// Row 3, Col 1 = S
			else if (xpos > 14 && xpos < 19)
				keyboardRows[3] |= 0x04;	// Row 3, Col 2 = D
			else if (xpos > 18 && xpos < 23)
				keyboardRows[3] |= 0x08;	// Row 3, Col 3 = F
			else if (xpos > 22 && xpos < 27)
				keyboardRows[3] |= 0x10;	// Row 3, Col 4 = G
			else if (xpos > 26 && xpos < 31)
				keyboardRows[3] |= 0x20;	// Row 3, Col 5 = H
			else if (xpos > 30 && xpos < 35)
				keyboardRows[3] |= 0x40;	// Row 3, Col 6 = J
			else if (xpos > 34 && xpos < 39)
				keyboardRows[3] |= 0x80;	// Row 3, Col 7 = K
			else if (xpos > 38 && xpos < 43)
				keyboardRows[6] |= 0x10;	// Row 6, Col 4 = L
			else if (xpos > 42 && xpos < 47)
				keyboardRows[6] |= 0x20;	// Row 6, Col 5 = ;
			else if (xpos > 46 && xpos < 51)
				keyboardRows[6] |= 0x40;	// Row 6, Col 6 = :
			else if (xpos > 50 && xpos < 55)
				keyboardRows[6] |= 0x80;	// Row 6, Col 7 = ]
			else if (xpos > 54 && xpos < 61)
				keyboardRows[0] |= 0x40;	// Row 0, Col 6 = Space
		}
		else if (ypos == 10) {
			if (xpos > 2 && xpos < 9 && (keyHit & KEY_TOUCH))
				keyboardRows[0] ^= 0x04;	// Row 0, Col 2 = LShift
			else if (xpos > 8 && xpos < 13)
				keyboardRows[4] |= 0x01;	// Row 4, Col 0 = Z
			else if (xpos > 12 && xpos < 17)
				keyboardRows[4] |= 0x02;	// Row 4, Col 1 = X
			else if (xpos > 16 && xpos < 21)
				keyboardRows[4] |= 0x04;	// Row 4, Col 2 = C
			else if (xpos > 20 && xpos < 25)
				keyboardRows[4] |= 0x08;	// Row 4, Col 3 = V
			else if (xpos > 24 && xpos < 29)
				keyboardRows[4] |= 0x10;	// Row 4, Col 4 = B
			else if (xpos > 28 && xpos < 33)
				keyboardRows[4] |= 0x20;	// Row 4, Col 5 = N
			else if (xpos > 32 && xpos < 37)
				keyboardRows[4] |= 0x40;	// Row 4, Col 6 = M
			else if (xpos > 36 && xpos < 41)
				keyboardRows[4] |= 0x80;	// Row 4, Col 7 = ,
			else if (xpos > 40 && xpos < 45)
				keyboardRows[5] |= 0x10;	// Row 5, Col 4 = .
			else if (xpos > 44 && xpos < 49)
				keyboardRows[5] |= 0x20;	// Row 5, Col 5 = /
			else if (xpos > 48 && xpos < 53)
				keyboardRows[5] |= 0x40;	// Row 5, Col 6 = _
			else if (xpos > 52 && xpos < 60 && (keyHit & KEY_TOUCH))
				keyboardRows[0] ^= 0x08;	// Row 0, Col 3 = RShift
		}
	}
}

void sordM5SetupBackground(void) {
	setupCompressedBackground(SordM5KbTiles, SordM5KbMap, 8);
	memcpy(BG_PALETTE_SUB+0x80, SordM5KbPal, SordM5KbPalLen);
}
