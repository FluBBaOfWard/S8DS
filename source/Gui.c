#include <nds.h>

#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "EmuBase.h"
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
#include "cpu.h"
#include "Sound.h"
#include "ARMZ80/Version.h"
#include "SN76496/Version.h"
#include "SegaVDP/Version.h"
#include "AY38910/Version.h"
#include "SCC/Version.h"

#define EMUVERSION "V1.1.8 2026-07-06"

extern u8 sordM5Input;		// SordM5.s

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
static void resetConsole(void);

static void controllerSet(void);
static const char *getControllerText(void);
static void swapABSet(void);
static const char *getSwapABText(void);
static void rffSet(void);
static const char *getRffText(void);
//static void xStartSet(void);
static void joypadSet(void);
static const char *getJoypadText(void);
static void selectSet(void);
static const char *getSelectText(void);

static void scalingSet(void);
static const char *getScalingText(void);
static void brightSet(void);
static void colorSet(void);
static const char *getColorText(void);
static void borderSet(void);
static const char *getBorderText(void);
static void bgrLayerSet(void);
static const char *getBgrLayerText(void);
static void sprLayerSet(void);
static const char *getSprLayerText(void);
static void spriteSet(void);
static const char *getSpriteText(void);
static void glassesSet(void);
static const char *getGlassesText(void);

static void countrySet(void);
static const char *getCountryText(void);
static void selectMachine(void);
static const char *getMachineText(void);
static void biosSet(void);
static void ym2413Set(void);
static const char *getYM2413Text(void);
static void refreshChgSet(void);
static const char *getRefreshChgText(void);

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

static void uiAbout(void);
static void uiSelectMachine(void);
static void uiBios(void);
static void uiDipSwitches(void);

static void touchConsoleSet(void);
const char *getTouchConsoleText(void);
static void uiDipSwitchesSGAC(void);
static void uiDipSwitchesHangOnJr(void);
static void uiDipSwitchesSlapShooter(void);
static void uiDipSwitchesTransformer(void);
static void uiDipSwitchesPythagoras(void);
static void uiDipSwitchesOpaOpa(void);
static void uiDipSwitchesFantasyZone2(void);
static void uiDipSwitchesTetris(void);
static void uiDipSwitchesMegumiRescue(void);
static void uiDipSwitchesMegaTech(void);

static void ui11(void);
static void ui12(void);
static void ui13(void);


static const MItem dummyItems[] = {
	{"", uiDummy}
};
static const MItem fileItems[] = {
	{"Load Game", selectGame},
	{"Load State", loadState},
	{"Save State", saveState},
	{"Save SRAM", saveNVRAM},
	{"Save Settings", saveSettings},
	{"Eject Game", ejectGame},
	{"Power On/Off", powerOnOff},
	{"Reset Console", resetConsole},
	{"Quit Emulator", ui9},
};
static const MItem optionItems[] = {
	{"Controller", ui4},
	{"Display", ui5},
	{"Machine", ui6},
	{"Settings", ui7},
	{"Dipswitches", ui13},
	{"Debug", ui12},
};
static const MItem ctrlItems[] = {
	{"B Autofire:", autoBSet, getAutoBText},
	{"A Autofire:", autoASet, getAutoAText},
	{"Controller:", controllerSet, getControllerText},
	{"Swap A-B:  ", swapABSet, getSwapABText},
	{"Joypad Type:", joypadSet, getJoypadText},
	{"Use Select as Reset:", selectSet, getSelectText},
	{"Use R as FastForward:", rffSet, getRffText},
};
static const MItem displayItems[] = {
	{"Display:", scalingSet, getScalingText},
	{"Scaling:", flickSet, getFlickText},
	{"Gamma:", brightSet, getGammaText},
	{"Color:", colorSet, getColorText},
	{"GG Border:", borderSet, getBorderText},
	{"Perfect Sprites:", spriteSet, getSpriteText},
	{"3D Display:", glassesSet, getGlassesText},
};
static const MItem machineItems[] = {
	{"Region:", countrySet, getCountryText},
	{"Machine:", ui11, getMachineText},
	{"Bios Settings", ui8},
	{"YM2413:", ym2413Set, getYM2413Text},
};
static const MItem setItems[] = {
	{"Speed:", speedSet, getSpeedText},
	{"Allow Refresh Change:",refreshChgSet, getRefreshChgText},
	{"Autoload State:", autoStateSet, getAutoStateText},
	{"Autoload SRAM:", autoNVRAMSet, getAutoNVRAMText},
	{"Autosave Settings:", autoSettingsSet, getAutoSettingsText},
	{"Autopause Game:", autoPauseGameSet, getAutoPauseGameText},
	{"Powersave 2nd Screen:", powerSaveSet, getPowerSaveText},
	{"Emulator on Bottom:", screenSwapSet, getScreenSwapText},
	{"Console Touch:", touchConsoleSet, getTouchConsoleText},
};
static const MItem debugItems[] = {
	{"Debug Output:", debugTextSet, getDebugText},
	{"Disable Background:", bgrLayerSet, getBgrLayerText},
	{"Disable Sprites:", sprLayerSet, getSprLayerText},
	{"Step Frame", stepFrame},
};
static const MItem biosItems[] = {
	{"Use BIOS:", biosSet},
	{"Select Export Bios ->", selectUSBios},
	{"Select Japanese Bios ->", selectJPBios},
	{"Select GameGear Bios ->", selectGGBios},
	{"Select Coleco Bios ->", selectCOLECOBios},
	{"Select MSX Bios ->", selectMSXBios},
	{"Select Sord M5 Bios ->", selectSORDM5Bios},
};

static const MItem quitItems[] = {
	{"Yes ", exitEmulator},
	{"No", backOutOfMenu},
};
static const MItem fnList11[] = {
	{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine},{"",selectMachine}
};
static const MItem fnList14[] = {{"",dip0Set4_2},{"",dip0Set6_1},{"",dip0Set7_1}};
static const MItem fnList15[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set0_1},{"",dip1Set1_2},{"",dip1Set3_2}};
static const MItem fnList16[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set1_1}};
static const MItem fnList17[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set0_1},{"",dip1Set1_1},{"",dip1Set2_2},{"",dip1Set4_2},{"",dip1Set6_2}};
static const MItem fnList18[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set0_2},{"",dip1Set3_1},{"",dip1Set5_2}};
static const MItem fnList19[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set1_1},{"",dip1Set2_2},{"",dip1Set4_2},{"",dip1Set6_2}};
static const MItem fnList20[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set1_1},{"",dip1Set2_2},{"",dip1Set4_2},{"",dip1Set6_2}};
static const MItem fnList21[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set1_1},{"",dip1Set4_2}};
static const MItem fnList22[] = {{"",dip0Set0_4},{"",dip0Set4_4},{"",dip1Set2_2},{"",dip1Set4_2}};
static const MItem fnList23[] = {{"",dip1Sub0_4},{"",dip0Sub5_3},{"",dip0Set1_1},{"",dip0Set0_1},{"",dip0Set2_3},{"",dip1Set4_4}};

const Menu menu0 = MENU_M("", uiNullNormal, dummyItems);
Menu menu1 = MENU_M("", uiAuto, fileItems);
const Menu menu2 = MENU_M("", uiAuto, optionItems);
const Menu menu3 = MENU_M("", uiAbout, dummyItems);
const Menu menu4 = MENU_M("Controller Settings", uiAuto, ctrlItems);
const Menu menu5 = MENU_M("Display Settings", uiAuto, displayItems);
const Menu menu6 = MENU_M("Machine Settings", uiAuto, machineItems);
const Menu menu7 = MENU_M("Settings", uiAuto, setItems);
const Menu menu8 = MENU_M("Bios Settings", uiBios, biosItems);
const Menu menu9 = MENU_M("Quit Emulator?", uiAuto, quitItems);
const Menu menu10 = MENU_M("", uiDummy, dummyItems);
const Menu menu11 = MENU_M("Select Machine", uiSelectMachine, fnList11);
const Menu menu12 = MENU_M("Debug", uiAuto, debugItems);
const Menu menu13 = MENU_M("Dip Switches", uiDipSwitches, dummyItems);
const Menu menu14 = MENU_M("Dip Switches", uiDipSwitchesSGAC, fnList14);
const Menu menu15 = MENU_M("Dip Switches", uiDipSwitchesHangOnJr, fnList15);
const Menu menu16 = MENU_M("Dip Switches", uiDipSwitchesSlapShooter, fnList16);
const Menu menu17 = MENU_M("Dip Switches", uiDipSwitchesTransformer, fnList17);
const Menu menu18 = MENU_M("Dip Switches", uiDipSwitchesPythagoras, fnList18);
const Menu menu19 = MENU_M("Dip Switches", uiDipSwitchesOpaOpa, fnList19);
const Menu menu20 = MENU_M("Dip Switches", uiDipSwitchesFantasyZone2, fnList20);
const Menu menu21 = MENU_M("Dip Switches", uiDipSwitchesTetris, fnList21);
const Menu menu22 = MENU_M("Dip Switches", uiDipSwitchesMegumiRescue, fnList22);
const Menu menu23 = MENU_M("Dip Switches", uiDipSwitchesMegaTech, fnList23);

const Menu *const menus[] = {&menu0, &menu1, &menu2, &menu3, &menu4, &menu5, &menu6, &menu7, &menu8, &menu9, &menu10, &menu11, &menu12, &menu13, &menu14, &menu15, &menu16, &menu17, &menu18, &menu19, &menu20, &menu21, &menu22, &menu23 };

static int sdscPtr = 0;
static char sdscBuffer[80];


static char *const ctrlTxt[] = {"1P", "2P"};
static char *const dispTxt[] = {"Unscaled", "Scaled to fit", "Scaled to aspect"};

static char *const machTxt[] = {"Auto", "SG-1000", "SC-3000", "OMV", "SG-1000 II", "Mark III", "Master System", "Master System 2", "Game Gear", "Mega Drive", "Coleco", "MSX", "Sord M5"};
static char *const bordTxt[] = {"Black", "Border Color", "None"};
static char *const cntrTxt[] = {"Auto", "US (NTSC)", "Europe (PAL)", "Japan (NTSC)"};
static char *const joypadTxt[] = {"Auto", "SMS 2 Buttton", "MD 3 Button", "MD 6 Button"};
static char *const biosTxt[] = {"Off", "Auto"};

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
static char *const livesMRTxt[] = {"2","3","4","Cheat"};
static char *const bonusTxt[] = {"50k 150k 250k","30k 80k 130k 180k","10k 30k 50k 70k","20k 60k 100k 140k"};
static char *const bonusOpaTxt[] = {"40k 60k 90k","25k 45k 70k","50k 90k","None"};
static char *const bonusPyTxt[] = {"50K 100K 200K 1M 2M 10M 20M 50M","100K 200K 1M 2M 10M 20M 50M","200K 1M 2M 10M 20M 50M","None"};
static char *const difficultyTxt[] = {"Easy","Medium","Hard","Hardest"};
static char *const difficultyTrTxt[] = {"Medium","Hard","Easy","Hardest"};
static char *const difficultyFZ2Txt[] = {"Normal","Easy","Hard","Hardest"};
static char *const timerTxt[] = {"80","90","70","60"};
static char *const mtTimerTxt[] = {"0:30","1:00","1:30","2:00","2:30","3:00","3:30","4:00","4:30","5:00","5:30","6:00","6:30","7:00","7:30","Free Play"};
static char *const languageTxt[] = {"English","Japanese","English"};
static char *const cabinetTxt[] = {"Cocktail","Upright"};

//----------------------------------------------------------------------

void setupGUI() {
	keysSetRepeat(25, 4);	// Delay, repeat.
	menu1.itemCount = ARRSIZE(fileItems) - (enableExit?0:1);
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
	if (gMachine == HW_SMS1) {
		setupSMS1Background();
	} else if (gMachine == HW_SMS2) {
		setupSMS2Background();
	} else if (gMachine == HW_GG) {
		setupGGBackground();
	} else if (gMachine == HW_MEGADRIVE) {
		setupMDBackground();
	} else if (gMachine == HW_OMV) {
		setupOMVBackground();
	} else if (gMachine == HW_SC3000) {
		setupSC3000Background();
	} else if (gMachine == HW_SG1000) {
		setupSG1000Background();
	} else if (gMachine == HW_SG1000II) {
		setupSG1000IIBackground();
	} else if (gMachine == HW_MARK3) {
		setupMARK3Background();
	} else if (gMachine == HW_COLECO) {
		setupColecoBackground();
	} else if (gMachine == HW_MSX) {
		setupMSXBackground();
	} else if (gMachine == HW_SORDM5) {
		setupSordM5Background();
	} else {
		uiNullDefault();
		return;
	}
	drawItem("Menu",27,1,0);
}

static void uiAbout() {
	char str[32];
	char *s = str+22;

	cls(1);
	drawTabs();
	drawMenuText("S8DS - Sega 8bit emulator", 4, 0);

	drawMenuText("B:      Button 1", 6, 0);
	drawMenuText("A:      Button 2", 7, 0);
	drawMenuText("DPad:   Move character", 8, 0);
	drawMenuText("Start:  Pause button", 9, 0);
	drawMenuText("Select: Reset", 10, 0);

	strcpy(str,"Coin counter0:       ");
	int2Str(coinCounter0,s);
	drawMenuText(str, 13, 0);
	strcpy(str,"Coin counter1:       ");
	int2Str(coinCounter1,s);
	drawMenuText(str, 14, 0);

	drawMenuText("S8DS         " EMUVERSION, 18, 0);
	drawMenuText("ARMZ80       " ARMZ80VERSION, 19, 0);
	drawMenuText("SEGAVDP      " SEGAVDPVERSION, 20, 0);
	drawMenuText("ARMSNGG76496 " ARMSNGGVERSION, 21, 0);
	drawMenuText("ARMAY38910   " ARMAY38910VERSION, 22, 0);
	drawMenuText("ARMSCC       " ARMSCCVERSION, 23, 0);
}

void uiSelectMachine() {
	setupSubMenuText();
	int i;
	for (i=0; i<ARRSIZE(machTxt); i++) {
		drawSubItem(machTxt[i], NULL);
	}
}

static void uiBios() {
	setupSubMenuText();
	drawSubItem("Use BIOS:", biosTxt[(gConfigSet >> 7) & 1]);
	drawMenuItem(" Select Export Bios ->");
	drawMenuItem(" Select Japanese Bios ->");
	drawMenuItem(" Select GameGear Bios ->");
	drawMenuItem(" Select Coleco Bios ->");
	drawMenuItem(" Select MSX Bios ->");
	drawMenuItem(" Select Sord M5 Bios ->");
}

static void uiDipSwitches() {
	setupSubMenuText();
}

static void uiDipSwitchesSGAC() {
	setupSubMenu("Dip Switches");
	drawSubItem("Coin slot 1:", creditsSGTxt[(dipSwitch0 >> 4) & 3]);
	drawSubItem("Demo sound:", autoTxt[(dipSwitch0 >> 6) & 1]);
	drawSubItem("Language:", languageTxt[((dipSwitch0 >> 7) & 1) + 1]);
}

static void setupSysEMenu() {
	setupSubMenuText();
	drawSubItem("Coin slot 1:", sysECreditsTxt[dipSwitch0 & 0xF]);
	drawSubItem("Coin slot 2:", sysECreditsTxt[(dipSwitch0 >> 4) & 0xF]);
}

static void uiDipSwitchesHangOnJr() {
	setupSysEMenu();
	drawSubItem("Demo sound:", autoTxt[dipSwitch1 & 1]);
	drawSubItem("Enemies:", difficultyTxt[(dipSwitch1>>1) & 3]);
	drawSubItem("Time Adj:", difficultyTxt[(dipSwitch1>>3) & 3]);
}

static void uiDipSwitchesSlapShooter() {
	setupSysEMenu();
	drawSubItem("Demo sound:", autoTxt[(dipSwitch1>>1) & 1]);
}

static void uiDipSwitchesTransformer() {
	setupSysEMenu();
	drawSubItem("2 Player:", autoTxt[dipSwitch1 & 1]);
	drawSubItem("Demo sound:", autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Lives:", livesTxt[(dipSwitch1>>2) & 3]);
	drawSubItem("Bonus Life:", bonusTxt[(dipSwitch1>>4) & 3]);
	drawSubItem("Difficulty:", difficultyTrTxt[(dipSwitch1>>6) & 3]);
}

static void uiDipSwitchesPythagoras() {
	setupSysEMenu();
	drawSubItem("Lives:", livesTxt[dipSwitch1 & 3]);
	drawSubItem("Ball Speed:", difficultyTxt[(dipSwitch1>>2) & 2]);
	drawSubItem("Bonus Life:", bonusPyTxt[(dipSwitch1>>5) & 3]);
}

static void uiDipSwitchesOpaOpa() {
	setupSysEMenu();
	drawSubItem("Demo sound:", autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Lives:", livesFZ2Txt[(dipSwitch1>>2) & 3]);
	drawSubItem("Bonus Life:", bonusOpaTxt[(dipSwitch1>>4) & 3]);
	drawSubItem("Difficulty:", difficultyFZ2Txt[(dipSwitch1>>6) & 3]);
}

static void uiDipSwitchesFantasyZone2() {
	setupSysEMenu();
	drawSubItem("Demo sound:",autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Lives:", livesFZ2Txt[(dipSwitch1>>2) & 3]);
	drawSubItem("Timer:", timerTxt[(dipSwitch1>>4) & 3]);
	drawSubItem("Difficulty:", difficultyFZ2Txt[(dipSwitch1>>6) & 3]);
}

static void uiDipSwitchesTetris() {
	setupSysEMenu();
	drawSubItem("Demo sound:", autoTxt[(dipSwitch1>>1) & 1]);
	drawSubItem("Difficulty:", difficultyFZ2Txt[(dipSwitch1>>4) & 3]);
}

static void uiDipSwitchesMegumiRescue() {
	setupSysEMenu();
	drawSubItem("Lives:", livesMRTxt[(dipSwitch1>>2) & 3]);
	drawSubItem("Cabinet:", cabinetTxt[(dipSwitch1>>4) & 1]);
}

static void uiDipSwitchesMegaTech() {
	setupSubMenuText();
	drawSubItem("Coin slot 1:", mtCoin1Txt[~dipSwitch1 & 0xF]);
	drawSubItem("Coin slot 2:", mtCoin2Txt[~(dipSwitch0>>5) & 0x7]);
	drawSubItem("Coin slot 3:", mtAcceptTxt[(dipSwitch0>>1) & 0x1]);
	drawSubItem("Coin slot 4:", mtAcceptTxt[(dipSwitch0) & 0x1]);
	drawSubItem("Coin slot 3/4:", mtCoin3Txt[(dipSwitch0>>2) & 0x7]);
	drawSubItem("Time per credit:", mtTimerTxt[(dipSwitch1>>4) & 0xF]);
}

void ui13() {
	int ds = 13;
	if (gArcadeGameSet == AC_CHAMPION_BOXING || gArcadeGameSet == AC_CHAMPION_WRESTLING || gArcadeGameSet == AC_DOKI_DOKI_PENGUIN) {
		ds = 14;
	} else if (gArcadeGameSet == AC_HANG_ON_JR) {
		ds = 15;
	} else if (gArcadeGameSet == AC_SLAP_SHOOTER) {
		ds = 16;
	} else if (gArcadeGameSet == AC_TRANSFORMER || gArcadeGameSet == AC_ASTRO_FLASH) {
		ds = 17;
	} else if (gArcadeGameSet == AC_RIDDLE_OF_PYTHAGORAS) {
		ds = 18;
	} else if (gArcadeGameSet == AC_OPA_OPA || gArcadeGameSet == AC_OPA_OPA_N) {
		ds = 19;
	} else if (gArcadeGameSet == AC_FANTASY_ZONE_2) {
		ds = 20;
	} else if (gArcadeGameSet == AC_TETRIS) {
		ds = 21;
	} else if (gArcadeGameSet == AC_MEGUMI_RESCUE) {
		ds = 22;
	} else if (gEmuFlags & MT_MODE || gMachine == HW_MEGATECH) {
		ds = 23;
	}

	enterMenu(ds);
}
void ui11() {
	enterMenu(11);
	selected = gMachineSet;
}

void ui12() {
	enterMenu(12);
}

void nullUINormal(int keyHit) {
	if (!(emuSettings & ENABLE_LIVE_UI)) {
		nullUIDebug(keyHit);		// Just check touch, open menu.
		return;
	}
	switch (gMachine) {
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
			if (keyHit & KEY_TOUCH) {
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
		}
		else {
			quickSelectGame();
		}
	}
}

void nullUISG1000(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>4);
		if ((xpos > 51) && (ypos < 2)) {
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
	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>3);
		int ypos = (myTouch.py>>3);
		if ((xpos > 25) && (ypos < 4)) {
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
	sc3Keyboard = 0xFF;				// 0xFF = nokey
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>4);
		if ((xpos > 51) && (ypos < 2)) {
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
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
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
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>3);
		if ((xpos > 51) && (ypos < 4)) {
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
	static bool prePower = false;

	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
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
			if (keyHit & KEY_TOUCH) {
				if (!powerIsOn) {
					powerOnOff();
				} else if (!prePower) {
					prePower = true;
				}
			}
		}
	}
	else if (prePower && powerIsOn) {
		powerOnOff();
		prePower = false;
	}
}

void nullUISMS2(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
			openMenu();
		}
		else if ((ypos > 18 && ypos < 30) && (xpos > 18 && xpos < 52)) { // Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos > 24 && ypos < 32) && (xpos > 4 && xpos < 13)) { // Pause button
			EMUinput |= KEY_START;
		}
		else if (ypos > 40 && ypos < 43 && xpos > 5 && xpos < 12) { // Power button
			if (keyHit & KEY_TOUCH) {
				powerOnOff();
			}
		}
	}
}

void nullUIMD(int keyHit) {
	if (EMUinput & KEY_TOUCH) {
		touchPosition myTouch;
		touchRead(&myTouch);
		int xpos = (myTouch.px>>2);
		int ypos = (myTouch.py>>2);
		if ((xpos > 51) && (ypos < 8)) {
			openMenu();
		}
		else if ((ypos > 12 && ypos < 18) && (xpos > 26 && xpos < 50)) { // Cartridge port
			cartridgePortTouched(keyHit);
		}
		else if ((ypos > 25 && ypos < 29) && (xpos > 28 && xpos < 33)) { // Pause button
			EMUinput |= KEY_START;
		}
		else if ((ypos > 41 && ypos < 46) && (xpos > 14 && xpos < 19)) { // Reset button
			EMUinput |= KEY_SELECT;
		}
		else if ((ypos > 34 && ypos < 38) && (xpos > 14 && xpos < 19)) { // Power button
			if (keyHit & KEY_TOUCH) {
				powerOnOff();
			}
		}
	}
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
void debugIO(u16 port, u8 val, const char *message) {
	char debugString[32];

	debugString[0] = 0;
	strlcat(debugString, message, sizeof(debugString));
	short2HexStr(&debugString[strlen(debugString)], port);
	strlcat(debugString, " val:", sizeof(debugString));
	char2HexStr(&debugString[strlen(debugString)], val);
	debugOutput(debugString);
}
//---------------------------------------------------------------------------------
void debugIOUnimplR(u16 port) {
	debugIO(port, 0, "Unimpl R port:");
}
void debugIOUnimplW(u8 val, u16 port) {
	debugIO(port, val, "Unimpl W port:");
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
	powerIsOn = !powerIsOn;
	loadCart(gEmuFlags);			// This resets the graphics.
	soundSetMuteGUI();
	if (!isMenuOpen()) {
		cls(0);
		uiNullNormal();
	}
	if (!powerIsOn) {
		antWarsInit();
	}
}

void ejectGame() {
	ejectCart();
	gameInserted = false;
}

void resetConsole() {
	loadCart(gEmuFlags);
}

//---------------------------------------------------------------------------------
/// Switch between Player 1 & Player 2 controls
void controllerSet() {				// See io.s: refreshEMUjoypads
	joyCfg ^= 0x40000000;
}
const char *getControllerText() {
	return ctrlTxt[(joyCfg>>30)&1];
}

void swapABSet() {
	joyCfg ^= 0x400;
}
const char *getSwapABText() {
	return autoTxt[(joyCfg>>10)&1];
}

void rffSet() {
	gConfigSet ^= 0x10;
	settingsChanged = 1;
}
const char *getRffText() {
	return autoTxt[(gConfigSet>>4)&1];
}
/*
void xStartSet() {
	gConfigSet ^= 0x40;
}
*/
void joypadSet() {
	inputHW++;
	if (inputHW >= 3) {
		inputHW = 0;
	}
}
const char *getJoypadText() {
	return joypadTxt[inputHW&3];
}

void selectSet() {
	gConfigSet ^= 0x20;
}
const char *getSelectText() {
	return autoTxt[(gConfigSet>>5)&1];
}

void scalingSet() {
	gScalingSet++;
	if (gScalingSet >= 3) {
		gScalingSet = 0;
	}
	setupScaling();
	VDP0ApplyScaling();
}
const char *getScalingText() {
	return dispTxt[gScalingSet];
}

void brightSet() {
	gammaSet();
	paletteInit(gGammaValue);
	mapSGPalette(gGammaValue);
	paletteTxAll();					// Make new palette visible
	setupMenuPalette();
}

void colorSet() {
	gColorValue++;
	if (gColorValue > 4) {
		gColorValue = 0;
	}
	paletteInit(gGammaValue);
	mapSGPalette(gGammaValue);
	paletteTxAll();					// Make new palette visible
}
const char *getColorText() {
	return brighTxt[gColorValue];
}

void borderSet() {
	bColor++;
	if (bColor >= 3) {
		bColor = 0;
	}
	makeBorder();
}
const char *getBorderText() {
	return bordTxt[bColor];
}

void bgrLayerSet() {
	gGfxMask ^= 0x03;
}
const char *getBgrLayerText() {
	return autoTxt[gGfxMask&1];
}

void sprLayerSet() {
	gGfxMask ^= 0x10;
}
const char *getSprLayerText() {
	return autoTxt[(gGfxMask>>4)&1];
}

void refreshChgSet() {
	emuSettings ^= ALLOW_REFRESH_CHG;
	updateLCDRefresh();
}
const char *getRefreshChgText() {
	return autoTxt[(emuSettings&ALLOW_REFRESH_CHG)>>19];
}

void spriteSet() {
	SPRS ^= 1;
}
const char *getSpriteText() {
	return autoTxt[SPRS & 1];
}

void glassesSet() {
	g3DEnable ^= 1;
}
const char *getGlassesText() {
	return biosTxt[g3DEnable & 1];
}

void biosSet() {
	gConfigSet ^= 0x80;
}

void countrySet() {
	int i;
	gRegion = (gRegion+1)&3;
	i = (gRegion)-1;
	if (i < 0) {
		i = 0;
	}
	if (gMachine == HW_GG) {
		i &= ~PALTIMING;
	}
	gEmuFlags = (gEmuFlags & ~3) | i;
	soundSetFrequency();
	VDP0ScanlineBPReset();
	setupScaling();
	VDP0ApplyScaling();
	VDP0SetMode();
}
const char *getCountryText() {
	return cntrTxt[gRegion];
}

void selectMachine() {
	gMachineSet = selected;
	backOutOfMenu();
}
const char *getMachineText() {
	return machTxt[gMachineSet];
}

void ym2413Set() {
	ym2413Enabled ^= 0x01;
}
const char *getYM2413Text() {
	return biosTxt[ym2413Enabled&1];
}

void touchConsoleSet() {
	emuSettings ^= ENABLE_LIVE_UI;
}
const char *getTouchConsoleText() {
	return autoTxt[(emuSettings & ENABLE_LIVE_UI)>>12];
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
