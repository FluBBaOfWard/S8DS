#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Gui.h"
#include "RomLoading.h"
#include "Equates.h"
#include "SegaVDP/SegaVDP.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"

static const char *const folderName = "s8ds";
static const char *const settingName = "settings.cfg";


static ConfigData cfg;

//static char tempState[STATESIZE];

//---------------------------------------------------------------------------------
int loadSettings() {
	FILE *file;

	cfg.currentPath[0] = 0;
	cfg.biosUS[0] = 0;
	cfg.biosJP[0] = 0;
	cfg.biosGG[0] = 0;
	cfg.biosCOLECO[0] = 0;
	cfg.biosMSX[0] = 0;
	cfg.biosSORDM5[0] = 0;
	if (findFolder(folderName)) {
		return 1;
	}
	if ( (file = fopen(settingName, "r")) ) {
		fread(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		if (!strstr(cfg.magic, "cfg")) {
			infoOutput("Error in settings file.");
			return 1;
		}
	} else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
		return 1;
	}

//	gDipSwitch0 = cfg.dipSwitch0;
//	gDipSwitch1 = cfg.dipSwitch1;
	SPRS         = cfg.sprites;
	g3DEnable    = cfg.glasses;
	gConfigSet   = cfg.config;
	gScalingSet  = cfg.scaling & 3;
	gFlicker     = cfg.flicker & 1;
	gGammaValue  = cfg.gammaValue & 0x7;
	gColorValue  = (cfg.gammaValue>>4) & 0x7;
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	sleepTime    = cfg.sleepTime;
	joyCfg       = (joyCfg & ~0x400) | ((cfg.controller & 1)<<10);
	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));

	infoOutput("Settings loaded.");
	return 0;
}

void saveSettings() {
	FILE *file;

	strcpy(cfg.magic, "cfg");
//	cfg.dipSwitch0  = gDipSwitch0;
	cfg.sprites     = SPRS;
	cfg.glasses     = g3DEnable;
	cfg.config      = gConfigSet;
	cfg.scaling     = gScalingSet & 3;
	cfg.flicker     = gFlicker & 1;
	cfg.gammaValue  = (gGammaValue & 0x7)|((gColorValue & 0x7)<<4);
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK;		// Clear speed setting.
	cfg.sleepTime   = sleepTime;
	cfg.controller  = (joyCfg>>10) & 1;
	strlcpy(cfg.currentPath, currentDir, sizeof(cfg.currentPath));

	if ( findFolder(folderName) ) {
		return;
	}
	if ( (file = fopen(settingName, "w")) ) {
		fwrite(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		infoOutput("Settings saved.");
	} else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}
}

int loadNVRAM() {
	return loadSRAM();
}
int loadSRAM() {
	FILE *file;
	char sramName[FILENAMEMAXLENGTH];

	if (findFolder(folderName)) {
		return 1;
	}
	setFileExtension(sramName, currentFilename, ".sav", sizeof(sramName));
	if ( (file = fopen(sramName, "r")) ) {
		fread(EMU_SRAM, 1, 0x2000, file);
		fclose(file);
		infoOutput("Loaded SRAM.");
	} else {
		return 1;
	}
	return 0;
}
void saveNVRAM() {
	saveSRAM();
}
void saveSRAM() {
	FILE *file;
	char sramName[FILENAMEMAXLENGTH];

	if (findFolder(folderName)) {
		return;
	}
	setFileExtension(sramName, currentFilename, ".sav", sizeof(sramName));
	if ( (file = fopen(sramName, "w")) ) {
		fwrite(EMU_SRAM, 1, 0x2000, file);
		fclose(file);
		infoOutput("Saved SRAM.");
	}
}

void loadState() {
	loadDeviceState(folderName);
}

void saveState() {
	saveDeviceState(folderName);
}

bool loadGame(const char *gameName) {
	char fileExt[8];
	if (gameName) {
		cls(0);
		drawText("   Please wait, loading.", 11, 0);
		gEmuFlags &= ~(MD_MODE|GG_MODE|SG_MODE|SC_MODE|COL_MODE|MSX_MODE|SORDM5_MODE|SGAC_MODE|SYSE_MODE|MT_MODE);
		g_ROM_Size = loadROM(ROM_Space, gameName, 0x100000);
		if (!g_ROM_Size) {
			g_ROM_Size = loadArcadeROM(ROM_Space, gameName);
		}
		if (g_ROM_Size) {
			getFileExtension(fileExt, currentFilename);
			if (strstr(fileExt, ".gg")) {
				gEmuFlags |= GG_MODE;
			}
			else if (strstr(fileExt, ".sg")) {
				gEmuFlags |= SG_MODE;
			}
			else if (strstr(fileExt, ".sc")) {
				gEmuFlags |= SC_MODE;
			}
			else if ((strstr(fileExt, ".mx1") || strstr(fileExt, ".mx2") || strstr(fileExt, ".rom"))
					 && ((ROM_Space[0] == 0x41 && ROM_Space[1] == 0x42) || (ROM_Space[0x4000] == 0x41 && ROM_Space[0x4001] == 0x42))) {
				gEmuFlags |= MSX_MODE;
			}
			else if ((strstr(fileExt, ".col") || strstr(fileExt, ".rom"))
					 && ((ROM_Space[0] == 0xAA && ROM_Space[1] == 0x55) || (ROM_Space[0] == 0x55 && ROM_Space[1] == 0xAA))) {
				gEmuFlags |= COL_MODE;
			}
			else if (strstr(fileExt, ".rom")
					 && (ROM_Space[0] == 0x00 || ROM_Space[0] == 0x02)) {
				gEmuFlags |= SORDM5_MODE;
			}
			setEmuSpeed(0);
			loadCart(gEmuFlags);
			loadSRAM();
			if (emuSettings & AUTOLOAD_STATE) {
				loadState();
			}
			gameInserted = true;
			powerButton = true;
			closeMenu();
			return false;
		}
	}
	return true;
}

void selectGame() {
	pauseEmulation = true;
	ui10();
	const char *gameName = browseForFileType(FILEEXTENSIONS".zip");
	if (loadGame(gameName)) {
		backOutOfMenu();
	}
}

static bool selectBios(const char *fileTypes, char *dest) {
	const char *biosName = browseForFileType(fileTypes);
	cls(0);

	if (biosName) {
		strlcpy(dest, currentDir, FILEPATHMAXLENGTH);
		strlcat(dest, "/", FILEPATHMAXLENGTH);
		strlcat(dest, biosName, FILEPATHMAXLENGTH);
		return true;
	}
	return false;
}

void selectUSBios() {
	if (selectBios(".sms.zip", cfg.biosUS)) {
		loadUSBIOS();
	}
}

void selectJPBios() {
	if (selectBios(".sms.zip", cfg.biosJP)) {
		loadJPBIOS();
	}
}

void selectGGBios() {
	if (selectBios(".gg.zip", cfg.biosGG)) {
		loadGGBIOS();
	}
}

void selectCOLECOBios() {
	if (selectBios(".rom.col.zip", cfg.biosCOLECO)) {
		loadCOLECOBIOS();
	}
}

void selectMSXBios() {
	if (selectBios(".mx1.mx2.rom.zip", cfg.biosMSX)) {
		loadMSXBIOS();
	}
}

void selectSORDM5Bios() {
	if (selectBios(".ic21.rom.zip", cfg.biosSORDM5)) {
		loadSORDM5BIOS();
	}
}

static int loadBIOS(void *dest, const char *fPath, const int maxSize) {
	char tempString[FILEPATHMAXLENGTH];
	char *sPtr;

	strlcpy(tempString, fPath, sizeof(tempString));
	if ((sPtr = strrchr(tempString, '/'))) {
		sPtr[0] = 0;
		sPtr += 1;
		chdir("/");
		chdir(tempString);
		return loadROM(dest, sPtr, maxSize);
	}
	return 0;
}

int loadUSBIOS(void) {
	if (loadBIOS(BIOS_US_Space, cfg.biosUS, sizeof(BIOS_US_Space))) {
		g_BIOSBASE_US = BIOS_US_Space;
		return 1;
	}
	g_BIOSBASE_US = NULL;
	return 0;
}

int loadJPBIOS(void) {
	if (loadBIOS(BIOS_JP_Space, cfg.biosJP, sizeof(BIOS_JP_Space))) {
		g_BIOSBASE_JP = BIOS_JP_Space;
		return 1;
	}
	g_BIOSBASE_JP = NULL;
	return 0;
}

int loadGGBIOS(void) {
	if (loadBIOS(BIOS_GG_Space, cfg.biosGG, sizeof(BIOS_GG_Space))) {
		g_BIOSBASE_GG = BIOS_GG_Space;
		return 1;
	}
	g_BIOSBASE_GG = NULL;
	return 0;
}

int loadCOLECOBIOS(void) {
	if (loadBIOS(BIOS_COLECO_Space, cfg.biosCOLECO, sizeof(BIOS_COLECO_Space))) {
		g_BIOSBASE_COLECO = BIOS_COLECO_Space;
		return 1;
	}
	g_BIOSBASE_COLECO = NULL;
	return 0;
}

int loadMSXBIOS(void) {
	if (loadBIOS(BIOS_MSX_Space, cfg.biosMSX, sizeof(BIOS_MSX_Space))) {
		g_BIOSBASE_MSX = BIOS_MSX_Space;
		return 1;
	}
	g_BIOSBASE_MSX = NULL;
	return 0;
}

int loadSORDM5BIOS(void) {
	if (loadBIOS(BIOS_SORDM5_Space, cfg.biosSORDM5, sizeof(BIOS_SORDM5_Space))) {
		g_BIOSBASE_SORDM5 = BIOS_SORDM5_Space;
		return 1;
	}
	g_BIOSBASE_SORDM5 = NULL;
	return 0;
}
