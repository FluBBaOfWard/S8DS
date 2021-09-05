#ifndef EMUBASE
#define EMUBASE

typedef struct {				//(config struct)
	char magic[4];				//="CFG",0
	int emuSettings;
	int sleepTime;				// autoSleepTime
	u8 scaling;					// from gfx.s
	u8 flicker;					// from gfx.s
	u8 gammaValue;				// from gfx.s
	u8 sprites;					// from gfx.s
	u8 glasses;					// from gfx.s
	u8 config;					// from cart.s
	u8 controller;				// from io.s
	u8 dipSwitch0;				// from io.s
	char currentPath[256];
	char biosUS[256];
	char biosJP[256];
	char biosGG[256];
	char biosCOLECO[256];
	char biosMSX[256];
	char biosSORDM5[256];
} ConfigData;

#endif // EMUBASE
