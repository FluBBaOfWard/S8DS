#ifndef EMUBASE
#define EMUBASE

// For emuSettings
#define ENABLE_LIVE_UI			(1<<12)
#define ALLOW_REFRESH_CHG		(1<<19)

// For config
#define CFG_R_AS_FASTFORWARD	(1<<4)
#define CFG_SELECT_AS_RESET		(1<<5)
#define CFG_X_AS_START			(1<<6)
#define CFG_USE_BIOS			(1<<7)

typedef struct {				//(config struct)
	char magic[4];				//="CFG",0
	int emuSettings;
	int unused;					// unused
	u8 display;					// from gfx.s, see ConfigData.display in FileHandling.c
	u8 flicker;					// from gfx.s
	u8 gammaValue;				// from gfx.s
	u8 sprites;					// from gfx.s
	u8 machine;					// from cart.s
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

#endif // !EMUBASE
