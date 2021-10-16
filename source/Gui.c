#include <nds.h>

#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Main.h"
#include "SG1000.h"
#include "OmvBgr.h"
#include "SG1000-II.h"
#include "SC3000.h"
#include "Mark3.h"
#include "SMS1.h"
#include "SMS2.h"
#include "GG.h"
#include "MD.h"
#include "ColecoNumpad.h"
#include "MSX.h"
#include "SordM5Kb.h"
#include "FileHandling.h"
#include "RomLoading.h"
#include "Equates.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"
#include "Sound.h"
#include "ARMZ80/Version.h"
#include "SN76496/Version.h"
#include "SegaVDP/version.h"
#include "AY38910/Version.h"
#include "SCC/Version.h"

#define EMUVERSION "V1.1.4 2021-10-16"

extern u8 sordM5Input;		// SordM5.s

u8 g_gammaValue = 0;

static void nullUISG1000(int key);
static void nullUIOMV(int key);
static void nullUISC3000(int key);
static void nullUISG1000II(int key);
static void nullUIMark3(int key);
static void nullUISMS1(int key);
static void nullUISMS2(int key);
static void nullUIMD(int key);
static void nullUIColeco(int key);
static void nullUIMSX(int key);
static void nullUISordM5(int key);

static void setupSG1000Background(void);
static void setupOMVBackground(void);
static void setupSC3000Background(void);
static void setupSG1000IIBackground(void);
static void setupMARK3Background(void);
static void setupSMS1Background(void);
static void setupSMS2Background(void);
static void setupGGBackground(void);
static void setupMDBackground(void);
static void setupColecoBackground(void);
static void setupMSXBackground(void);
static void setupSordM5Background(void);

static void powerOnOff(void);
static void resetGame(void);

static void controllerSet(void);
static void swapABSet(void);
static void rffSet(void);
//static void xStartSet(void);
static void joypadSet(void);
static void selectSet(void);

static void scalingSet(void);
static void brightSet(void);
static void colorSet(void);
static void borderSet(void);
static void bgrLayerSet(void);
static void sprLayerSet(void);
static void spriteSet(void);
static void glassesSet(void);

static void countrySet(void);
static void machineSet(void);
static void biosSet(void);
static void ym2413Set(void);
static void selectMachine(void);

static void dip0Set0_1(void);
static void dip0Set1_1(void);
static void dip0Set6_1(void);
static void dip0Set7_1(void);
static void dip0Set4_2(void);
static void dip0Set2_3(void);
static void dip0Sub5_3(void);
static void dip0Set0_4(void);
static void dip0Set4_4(void);
static void dip1Set0_1(void);
static void dip1Set1_1(void);
static void dip1Set3_1(void);
static void dip1Set0_2(void);
static void dip1Set1_2(void);
static void dip1Set2_2(void);
static void dip1Set3_2(void);
static void dip1Set4_2(void);
static void dip1Set5_2(void);
static void dip1Set6_2(void);
static void dip1Sub0_4(void);
static void dip1Set4_4(void);

static void uiFile(void);
static void uiSettings(void);
static void uiAbout(void);
static void uiOptions(void);
static void uiController(void);
static void uiDisplay(void);
static void uiMachine(void);
static void uiSelectMachine(void);
static void uiSettings(void);
static void uiBios(void);
static void uiDipSwitches(void);

static void touchConsoleSet(void);
static void uiDipSwitchesSGAC(void);
static void uiDipSwitchesHangOnJr(void);
static void uiDipSwitchesTransformer(void);
static void uiDipSwitchesPythagoras(void);
static void uiDipSwitchesOpaOpa(void);
static void uiDipSwitchesFantasyZone2(void);
static void uiDipSwitchesTetris(void);
static void uiDipSwitchesMegaTech(void);

static void ui9(void);
static void ui11(void);
static void ui20(void);


const fptr fnMain[] = {nullUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI,subUI};

static const fptr fnList0[] = {uiDummy};
static const fptr fnList1[] = {selectGame, loadState, saveState, saveSRAM, saveSettings, ejectGame, powerOnOff, resetGame, ui9};
static const fptr fnList2[] = {ui4, ui5, ui6, ui7, ui11};
static const fptr fnList3[] = {uiDummy};
static const fptr fnList4[] = {autoBSet, autoASet, controllerSet, swapABSet, joypadSet, selectSet, rffSet};
static const fptr fnList5[] = {scalingSet, flickSet, brightSet, colorSet, borderSet, spriteSet, glassesSet, bgrLayerSet, sprLayerSet};
static const fptr fnList6[] = {countrySet, ui20, ui8, ym2413Set};
static const fptr fnList7[] = {speedSet, autoStateSet, autoSettingsSet, autoNVRAMSet, autoPauseGameSet, powerSaveSet, screenSwapSet, debugTextSet, touchConsoleSet};
static const fptr fnList8[] = {biosSet, selectUSBios, selectJPBios, selectGGBios, selectCOLECOBios, selectMSXBios, selectSORDM5Bios};
static const fptr fnList9[] = {exitEmulator, backOutOfMenu};
static const fptr fnList10[] = {uiDummy};
static const fptr fnList11[] = {uiDummy};
static const fptr fnList12[] = {dip0Set4_2,dip0Set6_1,dip0Set7_1};
static const fptr fnList13[] = {dip0Set0_4,dip0Set4_4,dip1Set0_1,dip1Set1_2,dip1Set3_2};
static const fptr fnList14[] = {dip0Set0_4,dip0Set4_4,dip1Set0_1,dip1Set1_1,dip1Set2_2,dip1Set4_2,dip1Set6_2};
static const fptr fnList15[] = {dip0Set0_4,dip0Set4_4,dip1Set0_2,dip1Set3_1,dip1Set5_2};
static const fptr fnList16[] = {dip0Set0_4,dip0Set4_4,dip1Set1_1,dip1Set2_2,dip1Set4_2,dip1Set6_2};
static const fptr fnList17[] = {dip0Set0_4,dip0Set4_4,dip1Set1_1,dip1Set2_2,dip1Set4_2,dip1Set6_2};
static const fptr fnList18[] = {dip0Set0_4,dip0Set4_4,dip1Set1_1,dip1Set4_2};
static const fptr fnList19[] = {dip1Sub0_4,dip0Sub5_3,dip0Set1_1,dip0Set0_1,dip0Set2_3,dip1Set4_4};
static const fptr fnList20[] = {selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine,selectMachine};
const fptr *const fnListX[] = {fnList0,fnList1,fnList2,fnList3,fnList4,fnList5,fnList6,fnList7,fnList8,fnList9,fnList10,fnList11,fnList12,fnList13,fnList14,fnList15,fnList16,fnList17,fnList18,fnList19,fnList20};
u8 menuXItems[] = {ARRSIZE(fnList0),ARRSIZE(fnList1),ARRSIZE(fnList2),ARRSIZE(fnList3),ARRSIZE(fnList4),ARRSIZE(fnList5),ARRSIZE(fnList6),ARRSIZE(fnList7),ARRSIZE(fnList8),ARRSIZE(fnList9),ARRSIZE(fnList10),ARRSIZE(fnList11),ARRSIZE(fnList12),ARRSIZE(fnList13),ARRSIZE(fnList14),ARRSIZE(fnList15),ARRSIZE(fnList16),ARRSIZE(fnList17),ARRSIZE(fnList18),ARRSIZE(fnList19),ARRSIZE(fnList20)};
const fptr drawUIX[] = {uiNullNormal,uiFile,uiOptions,uiAbout,uiController,uiDisplay,uiMachine,uiSettings,uiBios,uiYesNo,uiDummy,uiDipSwitches,uiDipSwitchesSGAC,uiDipSwitchesHangOnJr,uiDipSwitchesTransformer,uiDipSwitchesPythagoras,uiDipSwitchesOpaOpa,uiDipSwitchesFantasyZone2,uiDipSwitchesTetris,uiDipSwitchesMegaTech,uiSelectMachine};
const u8 menuXBack[] = {0,0,0,0,2,2,2,2,6,2,1,2,2,2,2,2,2,2,2,2,6};

static int sdscPtr = 0;
static char sdscBuffer[80];


static char *const autoTxt[] = {"Off","On","With R"};
static char *const speedTxt[] = {"Normal","Fast","Max","Slowmo"};
static char *const sleepTxt[] = {"5min","10min","30min","Off"};
static char *const brighTxt[] = {"I","II","III","IIII","IIIII"};
static char *const ctrlTxt[] = {"1P","2P"};
static char *const bordTxt[] = {"Black","Border Color","None"};
static char *const dispTxt[] = {"Scaled 1:1","Scaled to fit","Scaled to aspect"};
static char *const flickTxt[] = {"No Flicker","Flicker"};
static char *const cntrTxt[] = {"Auto","US (NTSC)","Europe (PAL)","Japan (NTSC)"};
static char *const machTxt[] = {"Auto","SG-1000","SC-3000","OMV","SG-1000 II","Mark III","Master System","Master System 2","Game Gear","Mega Drive","Coleco","MSX","Sord M5"};
static char *const joypadTxt[] = {"Auto","SMS 2 Buttton","MD 3 Button","MD 6 Button"};
static char *const biosTxt[] = {"Off","Auto"};

static char *const sysECreditsTxt[] = {"1Coin 1Credit","1Coin 2Credits","1Coin 3Credits","1Coin 4Credits","1Coin 5Credits","1Coin 6Credits","2Coins 1Credit","3Coins 1Credit",
							  "4Coins 1Credit","2Coins 3Credits","?Coins ?Credits","?Coins ?Credits","?Coins ?Credits","?Coins ?Credits","?Coins ?Credits","Free Play"};

static char *const mtAcceptTxt[] = {"Accepted","Inhibited"};
static char *const mtCoin1Txt[] = {"Inhibited","1Coin 1Credit","1Coin 2Credits","1Coin 3Credits","1Coin 4Credits","1Coin 5Credits","1Coin 6Credits","1Coin 7Credits",
							"1Coin 8Credits","1Coin 9Credits","1Coin 10Credits","1Coin 11Credits","1Coin 12Credits","1Coin 13Credits","1Coin 14Credits","1Coin 15Credits"};
static char *const mtCoin2Txt[] = {"Inhibited","2Coins 1Credit","1Coin 1Credit","1Coin 2Credits","1Coin 3Credits","1Coin 4Credits","1Coin 5Credits","1Coin 6Credits"};
static char *const mtCoin3Txt[] = {"1Coin 1Credit","2Coins 1Credit","3Coins 1Credit","4Coins 1Credit","5Coins 1Credit","6Coins 1Credit","8Coins 1Credit","10Coins 1Credit"};

static char *const creditsSGTxt[] = {"1Coin 1Credit","1Coin 2Credits","1Coin 3Credits","2Coins 1Credit"};
static char *const livesTxt[] = {"3","4","5","Infinite (Cheat)"};
static char *const livesFZ2Txt[] = {"3","4","5","2"};
static char *const bonusTxt[] = {"50k 150k 250k","30k 80k 130k 180k","10k 30k 50k 70k","20k 60k 100k 140k"};
static char *const bonusOpaTxt[] = {"40k 60k 90k","25k 45k 70k","50k 90k","None"};
static char *const bonusPyTxt[] = {"50K 100K 200K 1M 2M 10M 20M 50M","100K 200K 1M 2M 10M 20M 50M","200K 1M 2M 10M 20M 50M","None"};
static char *const difficultyTxt[] = {"Easy","Medium","Hard","Hardest"};
static char *const difficultyTrTxt[] = {"Medium","Hard","Easy","Hardest"};
static char *const difficultyFZ2Txt[] = {"Normal","Easy","Hard","Hardest"};
static char *const timerTxt[] = {"80","90","70","60"};
static char *const mtTimerTxt[] = {"0:30","1:00","1:30","2:00","2:30","3:00","3:30","4:00","4:30","5:00","5:30","6:00","6:30","7:00","7:30","Free Play"};
static char *const languageTxt[] = {"English","Japanese","English"};

//----------------------------------------------------------------------

void setupGUI() {
	emuSettings = AUTOPAUSE_EMULATION | AUTOSLEEP_OFF | (1<<12);
	keysSetRepeat(25, 4);	// Delay, repeat.
	menuXItems[1] = ARRSIZE(fnList1) - (enableExit?0:1);
	openMenu();
}

/// This is called when going from emu to ui.
void enterGUI() {
}

/// This is called going from ui to emu.
void exitGUI() {
}

void quickSelectGame(void) {
	openMenu();
	selectGame();
	closeMenu();
}

void uiNullNormal() {
	if (g_machine == HW_SMS1) {
		setupSMS1Background();
	} else if (g_machine == HW_SMS2) {
		setupSMS2Background();
	} else if (g_machine == HW_GG) {
		setupGGBackground();
	} else if (g_machine == HW_MEGADRIVE) {
		setupMDBackground();
	} else if (g_machine == HW_OMV) {
		setupOMVBackground();
	} else if (g_machine == HW_SC3000) {
		setupSC3000Background();
	} else if (g_machine == HW_SG1000) {
		setupSG1000Background();
	} else if (g_machine == HW_SG1000II) {
		setupSG1000IIBackground();
	} else if (g_machine == HW_MARK3) {
		setupMARK3Background();
	} else if (g_machine == HW_COLECO) {
		setupColecoBackground();
	} else if (g_machine == HW_MSX) {
		setupMSXBackground();
	} else if (g_machine == HW_SORDM5) {
		setupSordM5Background();
	} else {
		uiNullDefault();
		return;
	}
	drawItem("Menu",27,1,0);
}

static void uiFile() {
	setupMenu();
	drawMenuItem("Load Game ->");
	drawMenuItem("Load State");
	drawMenuItem("Save State");
	drawMenuItem("Save SRAM");
	drawMenuItem("Save Settings");
	drawMenuItem("Eject Game");
	drawMenuItem("Power On/Off");
	drawMenuItem("Reset Console");
	if (enableExit) {
		drawMenuItem("Quit Emulator");
	}
}

static void uiOptions() {
	setupMenu();
	drawMenuItem("Controller ->");
	drawMenuItem("Display ->");
	drawMenuItem("Machine ->");
	drawMenuItem("Settings ->");
	drawMenuItem("Dipswitches ->");
}

static void uiAbout() {
	char str[32];
	char *s = str+22;

	cls(1);
	drawTabs();
	drawSubText("S8DS - Sega 8bit emulator", 4, 0);

	drawSubText("B:      Button 1", 6, 0);
	drawSubText("A:      Button 2", 7, 0);
	drawSubText("DPad:   Move character", 8, 0);
	drawSubText("Start:  Pause button", 9, 0);
	drawSubText("Select: Reset", 10, 0);

	strcpy(str,"Coin counter0:       ");
	int2Str(coinCounter0,s);
	drawSubText(str, 13, 0);
	strcpy(str,"Coin counter1:       ");
	int2Str(coinCounter1,s);
	drawSubText(str, 14, 0);

	drawSubText("S8DS         " EMUVERSION, 18, 0);
	drawSubText("ARMZ80       " ARMZ80VERSION, 19, 0);
	drawSubText("SEGAVDP      " SEGAVDPVERSION, 20, 0);
	drawSubText("ARMSNGG76496 " ARMSNGGVERSION, 21, 0);
	drawSubText("ARMAY38910   " ARMAY38910VERSION, 22, 0);
	drawSubText("ARMSCC       " ARMSCCVERSION, 23, 0);
}

static void uiController() {
	setupSubMenu("Controller Settings");
	drawSubItem("B Autofire: ",autoTxt[autoB]);
	drawSubItem("A Autofire: ",autoTxt[autoA]);
	drawSubItem("Controller: ",ctrlTxt[(joyCfg>>30)&1]);
	drawSubItem("Swap A-B:   ",autoTxt[(~joyCfg>>10)&1]);
	drawSubItem("Joypad Type: ",joypadTxt[inputHW&3]);
	drawSubItem("Use Select as Reset: ",autoTxt[(g_configSet>>5)&1]);
	drawSubItem("Use R as FastForward: ",autoTxt[(g_configSet>>4)&1]);
}

static void uiDisplay() {
	setupSubMenu("Display Settings");
	drawSubItem("Display: ",dispTxt[g_scalingSet]);
	drawSubItem("Scaling: ",flickTxt[g_flicker]);
	drawSubItem("Gamma: ",brighTxt[g_gammaValue]);
	drawSubItem("Color: ",brighTxt[g_colorValue]);
	drawSubItem("GG Border: ",bordTxt[bColor]);
	drawSubItem("Perfect Sprites: ",autoTxt[SPRS&1]);
	drawSubItem("3D Display: ",biosTxt[g_3DEnable&1]);
	drawSubItem("Disable Background: ",autoTxt[g_gfxMask&1]);
	drawSubItem("Disable Sprites: ",autoTxt[(g_gfxMask>>4)&1]);
}

static void uiMachine() {
	setupSubMenu("Machine Settings");
	drawSubItem("Region: ",cntrTxt[g_region]);
	drawSubItem("Machine: ",machTxt[g_machineSet]);
	drawMenuItem(" Bios Settings ->");
	drawSubItem("YM2413: ",biosTxt[ym2413Enabled&1]);
}

static void uiSelectMachine() {
	setupSubMenu("Select Machine");
	drawMenuItem(" Auto");
	drawMenuItem(" SG-1000");
	drawMenuItem(" SC-3000");
	drawMenuItem(" Othello Multi Vision");
	drawMenuItem(" SG-1000 II");
	drawMenuItem(" Mark III");
	drawMenuItem(" Master System");
	drawMenuItem(" Master System 2");
	drawMenuItem(" Game Gear");
	drawMenuItem(" Mega Drive + PBC");
	drawMenuItem(" Coleco");
	drawMenuItem(" MSX");
	drawMenuItem(" Sord M5");
	drawMenuItem("");			// Cheating to remove last row.
}

static void uiSettings() {
	setupSubMenu("Settings");
	drawSubItem("Speed: ", speedTxt[(emuSettings>>6)&3]);
	drawSubItem("Autoload State: ", autoTxt[(emuSettings>>2)&1]);
	drawSubItem("Autosave Settings: ", autoTxt[(emuSettings>>9)&1]);
	drawSubItem("Autosave SRAM: ", autoTxt[(emuSettings>>10)&1]);
	drawSubItem("Autopause Game: ", autoTxt[emuSettings&1]);
	drawSubItem("Powersave 2nd Screen: ", autoTxt[(emuSettings>>1)&1]);
	drawSubItem("Emulator on Bottom: ", autoTxt[(emuSettings>>8)&1]);
	drawSubItem("Debug Output: ", autoTxt[gDebugSet&1]);
	drawSubItem("Console Touch: ", autoTxt[(emuSettings>>12)&1]);
}

static void uiBios() {
	setupSubMenu("Bios Settings");
	drawSubItem("Use BIOS: ",biosTxt[(g_configSet>>7)&1]);
	drawMenuItem(" Select Export Bios ->");
	drawMenuItem(" Select Japanese Bios ->");
	drawMenuItem(" Select GameGear Bios ->");
	drawMenuItem(" Select Coleco Bios ->");
	drawMenuItem(" Select MSX Bios ->");
	drawMenuItem(" Select Sord M5 Bios ->");
}

static void uiDipSwitches() {
	setupSubMenu("Dip Switches");
}

static void uiDipSwitchesSGAC() {
	setupSubMenu("Dip Switches");
	drawSubItem("Coin slot 1: ",creditsSGTxt[(dipSwitch0>>4) & 3]);
	drawSubItem("Demo sound: ",autoTxt[(dipSwitch0>>6) & 1]);
	drawSubItem("Language: ",languageTxt[((dipSwitch0>>7) & 1) + 1]);
}

static void setupSysEMenu() {
	setupSubMenu("Dip Switches");
	drawSubItem("Coin slot 1: ",sysECreditsTxt[dipSwitch0 & 0xF]);
	drawSubItem("Coin slot 2: ",sysECreditsTxt[(dipSwitch0>>4) & 0xF]);
}

static void uiDipSwitchesHangOnJr() {
	setupSysEMenu();
	drawSubItem("Demo sound: ",autoTxt[dipSwitch1 & 1]);
	drawSubItem("Enemies: ",difficultyTxt[(dipSwitch1>>1) & 3]);
	drawSubItem("Time Adj: ",difficultyTxt[(dipSwitch1>>3) & 3]);
}

static void uiDipSwitchesTransformer() {
	setupSysEMenu();
	drawSubItem("2 Player: ",autoTxt[dipSwitch1 & 1]);
	drawSubItem("Demo sound: ",autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Lives: ",livesTxt[(dipSwitch1>>2) & 3]);
	drawSubItem("Bonus Life: ",bonusTxt[(dipSwitch1>>4) & 3]);
	drawSubItem("Difficulty: ", difficultyTrTxt[(dipSwitch1>>6) & 3]);
}

static void uiDipSwitchesPythagoras() {
	setupSysEMenu();
	drawSubItem("Lives: ",livesTxt[dipSwitch1 & 3]);
	drawSubItem("Ball Speed: ",difficultyTxt[(dipSwitch1>>2) & 2]);
	drawSubItem("Bonus Life: ",bonusPyTxt[(dipSwitch1>>5) & 3]);
}

static void uiDipSwitchesOpaOpa() {
	setupSysEMenu();
	drawSubItem("Demo sound: ",autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Lives: ",livesFZ2Txt[(dipSwitch1>>2) & 3]);
	drawSubItem("Bonus Life: ",bonusOpaTxt[(dipSwitch1>>4) & 3]);
	drawSubItem("Difficulty: ", difficultyFZ2Txt[(dipSwitch1>>6) & 3]);
}

static void uiDipSwitchesFantasyZone2() {
	setupSysEMenu();
	drawSubItem("Demo sound: ",autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Lives: ",livesFZ2Txt[(dipSwitch1>>2) & 3]);
	drawSubItem("Timer: ",timerTxt[(dipSwitch1>>4) & 3]);
	drawSubItem("Difficulty: ", difficultyFZ2Txt[(dipSwitch1>>6) & 3]);
}

static void uiDipSwitchesTetris() {
	setupSysEMenu();
	drawSubItem("Demo sound: ",autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Difficulty: ", difficultyFZ2Txt[(dipSwitch1>>4) & 3]);
}

static void uiDipSwitchesMegaTech() {
	setupSubMenu("Dip Switches");
	drawSubItem("Coin slot 1: ",mtCoin1Txt[~dipSwitch1 & 0xF]);
	drawSubItem("Coin slot 2: ",mtCoin2Txt[~(dipSwitch0>>5) & 0x7]);
	drawSubItem("Coin slot 3: ",mtAcceptTxt[(dipSwitch0>>1) & 0x1]);
	drawSubItem("Coin slot 4: ",mtAcceptTxt[(dipSwitch0) & 0x1]);
	drawSubItem("Coin slot 3/4: ",mtCoin3Txt[(dipSwitch0>>2) & 0x7]);
	drawSubItem("Time per credit: ", mtTimerTxt[(dipSwitch1>>4) & 0xF]);
}

void ui9() {
	setSelectedMenu(9);
}
void ui11() {
	int ds = 11;
	if (gArcadeGameSet == AC_CHAMPION_BOXING || gArcadeGameSet == AC_CHAMPION_WRESTLING || gArcadeGameSet == AC_DOKI_DOKI_PENGUIN) {
		ds = 12;
	} else if (gArcadeGameSet == AC_HANG_ON_JR) {
		ds = 13;
	} else if (gArcadeGameSet == AC_TRANSFORMER || gArcadeGameSet == AC_ASTRO_FLASH) {
		ds = 14;
	} else if (gArcadeGameSet == AC_RIDDLE_OF_PYTHAGORAS) {
		ds = 15;
	} else if (gArcadeGameSet == AC_OPA_OPA) {
		ds = 16;
	} else if (gArcadeGameSet == AC_FANTASY_ZONE_2) {
		ds = 17;
	} else if (gArcadeGameSet == AC_TETRIS) {
		ds = 18;
	} else if (g_emuFlags & MT_MODE || g_machine == HW_MEGATECH) {
		ds = 19;
	}

	setSelectedMenu(ds);
}
void ui20() {
	setSelectedMenu(20);
	selected = g_machineSet;
}

void nullUINormal(int keyHit) {
	if (!(emuSettings & (1<<12))) {
		nullUIDebug(keyHit);		// Just check touch, open menu.
		return;
	}
	switch (g_machine) {
		case HW_SMS1:
			nullUISMS1(keyHit);
			break;
		case HW_SMS2:
			nullUISMS2(keyHit);
			break;
		case HW_MEGADRIVE:
			nullUIMD(keyHit);
			break;
		case HW_SG1000:
			nullUISG1000(keyHit);
			break;
		case HW_OMV:
			nullUIOMV(keyHit);
			break;
		case HW_SC3000:
			nullUISC3000(keyHit);
			break;
		case HW_SG1000II:
			nullUISG1000II(keyHit);
			break;
		case HW_MARK3:
			nullUIMark3(keyHit);
			break;
		case HW_COLECO:
			nullUIColeco(keyHit);
			break;
		case HW_MSX:
			nullUIMSX(keyHit);
			break;
		case HW_SORDM5:
			nullUISordM5(keyHit);
			break;
		default:
			if (keyHit&KEY_TOUCH) {
				openMenu();
			}
			break;
	}
}

void nullUIDebug(int keyHit) {
	if (keyHit & KEY_TOUCH) {
		openMenu();
	}
}

static void cartridgePortTouched(int keyHit) {
	if (keyHit & KEY_TOUCH) {
		if (gameInserted) {
			ejectGame();
		} else {
			quickSelectGame();
		}
	}
}

void nullUISG1000(int keyHit) {
	int xpos, ypos;

	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>4);
		if ( (xpos > 51) && (ypos < 2) ) {
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
	int xpos, ypos;

	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>3);
		ypos = (myTouch.py>>3);
		if ( (xpos > 25) && (ypos < 4) ) {
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
	int xpos, ypos;

	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>4);
		if ( (xpos > 51) && (ypos < 2) ) {
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
	int xpos, ypos;

	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>2);
		if ( (xpos > 51) && (ypos < 8) ) {
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
	int xpos, ypos;

	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>3);
		if ( (xpos > 51) && (ypos < 4) ) {
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
	int xpos, ypos;
	static bool prePower = false;

	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>2);
		if ( (xpos > 51) && (ypos < 8) ) {
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
			if (keyHit&KEY_TOUCH) {
				if (!powerButton) {
					powerOnOff();
				} else if (!prePower) {
					prePower = true;
				}
			}
		}
	}
	else if (prePower && powerButton) {
		powerOnOff();
		prePower = false;
	}
}

void nullUISMS2(int keyHit) {
	int xpos, ypos;

	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>2);
		if ( (xpos > 51) && (ypos < 8) ) {
			openMenu();
		}
		else if ((ypos > 18 && ypos < 30) && (xpos > 18 && xpos < 52)) {	// Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos > 24 && ypos < 32) && (xpos > 4 && xpos < 13)) {	// Pause button
			EMUinput |= KEY_START;
		}
		else if (ypos > 40 && ypos < 43 && xpos > 5 && xpos < 12) {	// Power button
			if (keyHit&KEY_TOUCH) {
				powerOnOff();
			}
		}
	}
}

void nullUIMD(int keyHit) {
	int xpos, ypos;
	
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>2);
		if ( (xpos > 51) && (ypos < 8) ) {
			openMenu();
		}
		else if ((ypos > 12 && ypos < 18) && (xpos > 26 && xpos < 50)) {	// Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos > 25 && ypos < 29) && (xpos > 28 && xpos < 33)) {	// Pause button
			EMUinput |= KEY_START;
		}
		else if ((ypos > 41 && ypos < 46) && (xpos > 14 && xpos < 19)) {	// Reset button
			EMUinput |= KEY_SELECT;
		}
		else if ((ypos > 34 && ypos < 38) && (xpos > 14 && xpos < 19)) {	// Power button
			if (keyHit&KEY_TOUCH) {
				powerOnOff();
			}
		}
	}
}

void nullUIColeco(int keyHit) {
	int xpos, ypos;
	
	colecoKey = 0;
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>3);
		ypos = (myTouch.py>>3);
		if ( (xpos > 25) && (ypos < 3) ) {
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

void nullUIMSX(int keyHit) {
	int xpos, ypos;

	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>4);
		if ( (xpos > 51) && (ypos < 2) ) {
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

void nullUISordM5(int keyHit) {
	int xpos, ypos;

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
		xpos = (myTouch.px>>2);
		ypos = (myTouch.py>>4);
		if ( (xpos > 51) && (ypos < 2) ) {
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


/**
 * Handles debug output for SDSC
 */
void sdscHandler(const unsigned char sdscChar) {
	sdscBuffer[sdscPtr++] = sdscChar;
	if (sdscChar == 10 || sdscChar == 13 || sdscPtr >= 79) {
		sdscBuffer[sdscPtr] = 0;
		sdscPtr = 0;
		debugOutput(sdscBuffer);
	}
}

//---------------------------------------------------------------------------------
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

void setupColecoBackground(void) {
	setupCompressedBackground(ColecoNumpadTiles, ColecoNumpadMap, 0);
	memcpy(BG_PALETTE_SUB+0x80, ColecoNumpadPal, ColecoNumpadPalLen);
}

void setupMSXBackground(void) {
	setupCompressedBackground(MSXTiles, MSXMap, 6);
	memcpy(BG_PALETTE_SUB+0x80, MSXPal, MSXPalLen);
}

void setupSordM5Background(void) {
	setupCompressedBackground(SordM5KbTiles, SordM5KbMap, 8);
	memcpy(BG_PALETTE_SUB+0x80, SordM5KbPal, SordM5KbPalLen);
}

//---------------------------------------------------------------------------------

void powerOnOff() {
	powerButton = !powerButton;
	loadCart(g_emuFlags);			// This resets the graphics.
	setMuteSoundGUI();
	if (!isMenuOpen()) {
		cls(0);
		uiNullNormal();
	}
}

void ejectGame() {
	ejectCart();
	gameInserted = false;
}

void resetGame() {
	loadCart(g_emuFlags);
}

//---------------------------------------------------------------------------------
void controllerSet() {				// See io.s: refreshEMUjoypads
	joyCfg ^= 0x40000000;
}

void swapABSet() {
	joyCfg ^= 0x400;
}

void rffSet() {
	g_configSet ^= 0x10;
	settingsChanged = 1;
}
/*
void xStartSet() {
	g_config_set ^= 0x40;
}
*/
void joypadSet(){
	inputHW++;
	if (inputHW >= 3) {
		inputHW = 0;
	}
}

void selectSet() {
	g_configSet ^= 0x20;
}


void scalingSet(){
	g_scalingSet++;
	if (g_scalingSet >= 3) {
		g_scalingSet = 0;
	}
	setupScaling();
	VDP0ApplyScaling();
}

void brightSet() {
	g_gammaValue++;
	if (g_gammaValue > 4) {
		g_gammaValue = 0;
	}
	paletteInit(g_gammaValue);
	mapSGPalette(g_gammaValue);
	paletteTxAll();					// Make new palette visible
	setupMenuPalette();
}

void colorSet() {
	g_colorValue++;
	if (g_colorValue > 4) {
		g_colorValue = 0;
	}
	paletteInit(g_gammaValue);
	mapSGPalette(g_gammaValue);
	paletteTxAll();					// Make new palette visible
}

void borderSet() {
	bColor++;
	if (bColor >= 3) {
		bColor = 0;
	}
	makeBorder();
}

void bgrLayerSet() {
	g_gfxMask ^= 0x03;
}

void sprLayerSet() {
	g_gfxMask ^= 0x10;
}

void spriteSet() {
	SPRS ^= 1;
}

void glassesSet() {
	g_3DEnable ^= 1;
}


void biosSet() {
	g_configSet ^= 0x80;
}

void countrySet() {
	int i;
	g_region = (g_region+1)&3;
	i = (g_region)-1;
	if (i < 0) {
		i = 0;
	}
	if (g_machine == HW_GG) {
		i &= ~PALTIMING;
	}
	g_emuFlags = (g_emuFlags & ~3) | i;
	soundSetFrequency();
	VDP0ScanlineBPReset();
	setupScaling();
	VDP0ApplyScaling();
	VDP0SetMode();
}

void machineSet() {
	g_machineSet++;
	if (g_machineSet >= HW_SELECT_END){
		g_machineSet = 0;
	}
}

void selectMachine() {
	g_machineSet = selected;
	backOutOfMenu();
}

void ym2413Set() {
	ym2413Enabled ^= 0x01;
}

void touchConsoleSet() {
	emuSettings ^= (1<<12);
}


void dip0Set0_1() {
	dipSwitch0 ^= 0x01;
}

void dip0Set1_1() {
	dipSwitch0 ^= 0x02;
}

void dip0Set6_1() {
	dipSwitch0 ^= 0x40;
}

void dip0Set7_1() {
	dipSwitch0 ^= 0x80;
}

void dip0Set4_2() {
	int i = (dipSwitch0+0x10) & 0x30;
	dipSwitch0 = (dipSwitch0 & ~0x30) | i;
}

void dip0Set2_3() {
	int i = (dipSwitch0+0x04) & 0x1C;
	dipSwitch0 = (dipSwitch0 & ~0x1C) | i;
}

void dip0Sub5_3() {
	int i = (dipSwitch0-0x20) & 0xE0;
	dipSwitch0 = (dipSwitch0 & ~0xE0) | i;
}

void dip0Set0_4() {
	int i = (dipSwitch0+0x1) & 0xF;
	dipSwitch0 = (dipSwitch0 & ~0xF) | i;
}

void dip0Set4_4() {
	int i = (dipSwitch0+0x10) & 0xF0;
	dipSwitch0 = (dipSwitch0 & ~0xF0) | i;
}

void dip1Set0_1() {
	dipSwitch1 ^= 0x01;
}

void dip1Set1_1() {
	dipSwitch1 ^= 0x02;
}

void dip1Set3_1() {
	dipSwitch1 ^= 0x08;
}

void dip1Set0_2() {
	int i = (dipSwitch1+0x01) & 0x03;
	dipSwitch1 = (dipSwitch1 & ~0x03) | i;
}

void dip1Set1_2() {
	int i = (dipSwitch1+0x02) & 0x06;
	dipSwitch1 = (dipSwitch1 & ~0x06) | i;
}

void dip1Set2_2() {
	int i = (dipSwitch1+0x04) & 0x0C;
	dipSwitch1 = (dipSwitch1 & ~0x0C) | i;
}

void dip1Set3_2() {
	int i = (dipSwitch1+0x08) & 0x18;
	dipSwitch1 = (dipSwitch1 & ~0x18) | i;
}

void dip1Set4_2() {
	int i = (dipSwitch1+0x10) & 0x30;
	dipSwitch1 = (dipSwitch1 & ~0x30) | i;
}

void dip1Set5_2() {
	int i = (dipSwitch1+0x20) & 0x60;
	dipSwitch1 = (dipSwitch1 & ~0x60) | i;
}

void dip1Set6_2() {
	int i = (dipSwitch1+0x40) & 0xC0;
	dipSwitch1 = (dipSwitch1 & ~0xC0) | i;
}

void dip1Sub0_4() {
	int i = (dipSwitch1-0x1) & 0xF;
	dipSwitch1 = (dipSwitch1 & ~0xF) | i;
}

void dip1Set4_4() {
	int i = (dipSwitch1+0x10) & 0xF0;
	dipSwitch1 = (dipSwitch1 & ~0xF0) | i;
}
