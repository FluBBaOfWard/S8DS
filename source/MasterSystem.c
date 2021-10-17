#include <nds.h>

#include "MasterSystem.h"
#include "Cart.h"
#include "Sound.h"
#include "SN76496/SN76496.h"
#include "ARMZ80/ARMZ80.h"


int packState(void *statePtr) {
	int size = 0;
	size += cartSaveState(statePtr+size);
	size += sn76496SaveState(statePtr+size, &SN76496_0);
	size += Z80SaveState(statePtr+size, &Z80OpTable);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	size += cartLoadState(statePtr+size);
	size += sn76496LoadState(&SN76496_0, statePtr+size);
	size += Z80LoadState(&Z80OpTable, statePtr+size);
}

int getStateSize() {
	int size = 0;
	size += cartGetStateSize();
	size += sn76496GetStateSize();
	size += Z80GetStateSize();
	return size;
}
