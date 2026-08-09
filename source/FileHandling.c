#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Shared/IPSPatch.h"
#include "Gui.h"
#include "MasterSystem.h"
#include "MSX.h"
#include "SordM5.h"
#include "RomLoading.h"
#include "Equates.h"
#include "SegaVDP/SegaVDP.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"

static const char *const folderName = "s8ds";
static const char *const settingName = "settings.cfg";

#define CONFIG_DISPLAY_MODE_MASK                 0x03
#define CONFIG_DISPLAY_3D_ENABLE                 0x10
#define CONFIG_DISPLAY_GG_UPSCALER_SHIFT            5
#define CONFIG_DISPLAY_GG_UPSCALER_MASK          0xE0

/*
 *   bits 0-1  SMS display geometry (unscaled, fit or aspect)
 *   bits 2-3  reserved
 *   bit  4    3D Display enabled
 *   bits 5-7  Game Gear upscaler method
 */
static ConfigData cfg;

//static char tempState[STATESIZE];

//---------------------------------------------------------------------------------
void applyConfigData(void) {
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	gScalingSet = cfg.display & CONFIG_DISPLAY_MODE_MASK;
	if (gScalingSet > SCALED_ASPECT) {
		gScalingSet = SCALED_1_1;
	}
	gGGScalingMethod = (cfg.display & CONFIG_DISPLAY_GG_UPSCALER_MASK)
		>> CONFIG_DISPLAY_GG_UPSCALER_SHIFT;
	if (gGGScalingMethod >= GG_UPSCALER_COUNT) {
		gGGScalingMethod = GG_UPSCALER_OFF;
	}
	g3DEnable    = (cfg.display & CONFIG_DISPLAY_3D_ENABLE) != 0;
	gFlicker     = cfg.flicker & 1;
	gGammaValue  = cfg.gammaValue & 0x7;
	gColorValue  = (cfg.gammaValue >> 4) & 0x7;
	SPRS         = cfg.sprites;
	gMachineSet  = cfg.machine & 0x1F;
	gRegion      = (cfg.machine >> 5) & 3;
	gConfigSet   = cfg.config;
	joyCfg       = (joyCfg & ~0x400) | ((cfg.controller & 1) << 10);
	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));
//	gDipSwitch0  = cfg.dipSwitch0;
//	gDipSwitch1  = cfg.dipSwitch1;
	pauseEmulation = (emuSettings & AUTOPAUSE_EMULATION);
	sleepTime    = 0x7F000000; // 360 days...
}

void updateConfigData(void) {
	strcpy(cfg.magic, "cfg");
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	cfg.display     = (gScalingSet & CONFIG_DISPLAY_MODE_MASK)
		| ((gGGScalingMethod << CONFIG_DISPLAY_GG_UPSCALER_SHIFT)
			& CONFIG_DISPLAY_GG_UPSCALER_MASK)
		| (g3DEnable ? CONFIG_DISPLAY_3D_ENABLE : 0);
	cfg.flicker     = gFlicker & 1;
	cfg.gammaValue  = (gGammaValue & 0x7) | ((gColorValue & 0x7) << 4);
	cfg.sprites     = SPRS;
	cfg.machine     = (gMachineSet & 0x1F) | ((gRegion & 3)<<5);
	cfg.config      = gConfigSet;
	cfg.controller  = (joyCfg>>10) & 1;
	strlcpy(cfg.currentPath, currentDir, sizeof(cfg.currentPath));
//	cfg.dipSwitch0  = gDipSwitch0;
}

void initSettings() {
	memset(&cfg, 0, sizeof(cfg));
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | AUTOSAVE_NVRAM | AUTOSLEEP_OFF | ENABLE_LIVE_UI;
	cfg.display     = SCALED_FIT | CONFIG_DISPLAY_3D_ENABLE;
	cfg.flicker     = 1;
	cfg.gammaValue  = 0x40;		// ColorValue = 4
	cfg.sprites     = 1;		// SpriteScanning On/Off;
	cfg.config      = 0x80;		// config, bit 7=BIOS on/off, bit 6=X as GG Start, bit 5=Select as Reset, bit 4=R as FastForward

	applyConfigData();
}

int loadSettings() {
	FILE *file;
	if (!findFolder(folderName)
		&& (file = fopen(settingName, "r"))) {
		int len = fread(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		if (strstr(cfg.magic, "cfg") && len == sizeof(ConfigData)) {
			applyConfigData();
			infoOutput("Settings loaded.");
			return 0;
		}
		updateConfigData();
		infoOutput("Error in settings file.");
	}
	else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}
	return 1;
}

int saveSettings() {
	updateConfigData();

	FILE *file;
	if (!findFolder(folderName)
		&& (file = fopen(settingName, "w"))) {
		int len = fwrite(&cfg, 1, sizeof(ConfigData), file);
		fclose(file);
		if (len == sizeof(ConfigData)) {
			infoOutput("Settings saved.");
			return 0;
		}
		infoOutput("Couldn't save settings.");
	}
	else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}
	return 1;
}

int loadNVRAM() {
	return loadSRAM();
}
int loadSRAM() {
	FILE *file;
	char sramName[FILENAME_MAX_LENGTH];

	if (findFolder(folderName)) {
		return 1;
	}
	setFileExtension(sramName, currentFilename, ".sav", sizeof(sramName));
	if ((file = fopen(sramName, "r"))) {
		fread(EMU_SRAM, 1, 0x2000, file);
		fclose(file);
		infoOutput("Loaded SRAM.");
	}
	else {
		return 1;
	}
	return 0;
}
void saveNVRAM() {
	if (gCartFlags & SRAMFLAG) {
		forceSaveNVRAM();
	}
}
void forceSaveNVRAM() {
	FILE *file;
	char sramName[FILENAME_MAX_LENGTH];

	if (findFolder(folderName)) {
		return;
	}
	setFileExtension(sramName, currentFilename, ".sav", sizeof(sramName));
	if ((file = fopen(sramName, "w"))) {
		fwrite(EMU_SRAM, 1, 0x2000, file);
		fclose(file);
		infoOutput("Saved NVRAM.");
	}
}

void loadState() {
	loadDeviceState(folderName);
}

void saveState() {
	saveDeviceState(folderName);
}

int packState(void *statePtr) {
	if (gMachine == HW_MSX) {
		return msxPackState(statePtr);
	}
	else if (gMachine == HW_SORDM5) {
		return sordM5PackState(statePtr);
	}
	return smsPackState(statePtr);
}
void unpackState(const void *statePtr) {
	if (gMachine == HW_MSX) {
		msxUnpackState(statePtr);
	}
	else if (gMachine == HW_SORDM5) {
		sordM5UnpackState(statePtr);
	}
	smsUnpackState(statePtr);
}
int getStateSize(void) {
	if (gMachine == HW_MSX) {
		return msxGetStateSize();
	}
	if (gMachine == HW_SORDM5) {
		return sordM5GetStateSize();
	}
	return smsGetStateSize();
}

bool loadGame(const char *gameName) {
	char fileExt[8];
	if (gameName) {
		cls(0);
		drawText("   Please wait, loading.", 11, 0);
		gEmuFlags &= ~(MD_MODE|GG_MODE|SG_MODE|SC_MODE|COL_MODE|MSX_MODE|SORDM5_MODE|SGAC_MODE|SYSE_MODE|MT_MODE);
		gRomSize = loadROM(romSpacePtr, gameName, 0x100000);
		if (!gRomSize) {
			gRomSize = loadArcadeROM(romSpacePtr, gameName);
		}
		if (gRomSize) {
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
					 && ((romSpacePtr[0] == 0x41 && romSpacePtr[1] == 0x42) || (romSpacePtr[0x4000] == 0x41 && romSpacePtr[0x4001] == 0x42))) {
				gEmuFlags |= MSX_MODE;
			}
			else if ((strstr(fileExt, ".col") || strstr(fileExt, ".rom"))
					 && ((romSpacePtr[0] == 0xAA && romSpacePtr[1] == 0x55) || (romSpacePtr[0] == 0x55 && romSpacePtr[1] == 0xAA))) {
				gEmuFlags |= COL_MODE;
			}
			else if (strstr(fileExt, ".rom")
					 && (romSpacePtr[0] == 0x00 || romSpacePtr[0] == 0x02)) {
				gEmuFlags |= SORDM5_MODE;
			}
			setEmuSpeed(0);
			cartInitSRAM();
			loadCart(gEmuFlags);
			loadSRAM();
			if (emuSettings & AUTOLOAD_STATE) {
				loadState();
			}
			gameInserted = true;
			powerIsOn = true;
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
		strlcpy(dest, currentDir, FILEPATH_MAX_LENGTH);
		strlcat(dest, "/", FILEPATH_MAX_LENGTH);
		strlcat(dest, biosName, FILEPATH_MAX_LENGTH);
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
	char tempString[FILEPATH_MAX_LENGTH];
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

void selectIPS() {
	pauseEmulation = true;
	ui10();
	const char *ipsName = browseForFileType(".ips");
	if (ipsName && patchRom(romSpacePtr, ipsName, gRomSize)) {
		loadCart(gEmuFlags);
	}
	backOutOfMenu();
}
