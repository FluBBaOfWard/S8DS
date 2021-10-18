#include <nds.h>

#include "MasterSystem.h"
#include "Cart.h"
#include "Sound.h"
#include "Gfx.h"
#include "ARMZ80/ARMZ80.h"


int packState(void *statePtr) {
	int size = 0;
	size += cartSaveState(statePtr+size);
	size += sn76496SaveState(statePtr+size, &SN76496_0);
	size += VDPSaveState(statePtr+size, &VDP0);
	size += Z80SaveState(statePtr+size, &Z80OpTable);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	size += cartLoadState(statePtr+size);
	size += sn76496LoadState(&SN76496_0, statePtr+size);
	size += VDPLoadState(&VDP0, statePtr+size);
	size += Z80LoadState(&Z80OpTable, statePtr+size);
}

int getStateSize() {
	int size = 0;
	size += cartGetStateSize();
	size += sn76496GetStateSize();
	size += VDPGetStateSize();
	size += Z80GetStateSize();
	return size;
}
