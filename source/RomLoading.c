#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>

#include "RomLoading.h"
#include "FileHandling.h"
#include "Shared/EmuMenu.h"
#include "Shared/FileHelper.h"
#include "Shared/Unzip/unzipnds.h"
#include "Gui.h"
#include "Equates.h"
#include "Cart.h"
#include "io.h"

// Mega-Tech SMS games
//--------------------
// After Burner
// Alien Syndrome
// Astro Warrior
// Black Belt?
// Enduro Racer?
// Fantasy Zone
// Fantasy Zone II: The Tears of Opa Opa?
// Great Football
// Great Golf
// Great Soccer
// Kung Fu Kid?
// Out Run
// Parlour Games
// Power Strike II?
// Shinobi
// Spellcaster?

const ArcadeGame systemeGames[10];

static char *const megatechNames[] ={"Great Golf MT",   "Great Soccer MT", "Out Run MT",      "Alien Syndrom MT","Shinobi MT",      "Fantasy Zone MT", "After Burner MT", "Astro Warrior MT","Great Football MT","Parlour Games MT"};
static char *const megatechFiles[] ={"mt_ggolf.zip",    "mt_gsocr.zip",    "mt_orun.zip",     "mt_asyn.zip",     "mt_shnbi.zip",    "mt_fz.zip",       "mt_aftrb.zip",    "mt_astro.zip",    "mt_gfoot.zip",     "mt_parlg.zip"};
static char *const megatechGames[] ={"mp11129f.ic1",    "mp10747f.ic1",    "mpr-11078.ic1",   "mpr-11194.ic1",   "mp11706.ic1",     "mpr-10118.ic1",   "mp11271.ic1",     "ep13817.ic2",     "mpr-10576f.ic1",   "mp11404.ic1"};
//static char *const megatechGuides[]={"epr-12368-04.ic2","epr-12368-05.ic2","epr-12368-06.ic2","epr-12368-07.ic2","epr-12368-08.ic2","epr-12368-09.bin","epr-12368-10.ic2","epr-12368-13.ic1","epr-12368-19.ic2", "epr-12368-29.ic2"};
static const int   megatechSizes[] ={0x20000,           0x20000,           0x40000,           0x40000,           0x40000,           0x20000,           0x80000,           0x20000,           0x20000,            0x20000};

static const char syseDefines[] = {AC_HANG_ON_JR, AC_SLAP_SHOOTER, AC_TRANSFORMER, AC_ASTRO_FLASH, AC_RIDDLE_OF_PYTHAGORAS, AC_OPA_OPA, AC_OPA_OPA_N, AC_FANTASY_ZONE_2, AC_TETRIS, AC_MEGUMI_RESCUE};

int loadArcadeROM(void *dest, const char *fName) {
	int i;
	int size = 0;
	char fNameLowercase[16];
	char zipSubName[32];

	gArcadeGameSet = AC_NONE;
	dipSwitch0 = 0;
	dipSwitch1 = 0;

//	drawtext("Checking for Arcade Roms", 23, 0);
	strcasel(fNameLowercase, fName);
	for (i = 0; i < 10; i++) {
		if (strstr(fNameLowercase, megatechFiles[i])) {
			if (loadFileInZip(dest, fName, megatechGames[i], megatechSizes[i]) == 0) {
				size = cenHead.ucSize;
				strlcpy(currentFilename, megatechNames[i], sizeof(currentFilename));
				gEmuFlags |= MT_MODE;
				return size;
			}
		}
	}

	for (i = 0; i < 10; i++) {
		strlMerge(zipSubName, systemeGames[i].gameName, ".zip", sizeof(zipSubName));
		if (strstr(fNameLowercase, zipSubName)) {
			if (!loadACRoms(dest, systemeGames, i, ARRSIZE(systemeGames), true)) {
				gEmuFlags |= SYSE_MODE;
				gArcadeGameSet = syseDefines[i];
				return 0x80000;
			}
		}
	}

	if (strstr(fNameLowercase, "chboxing.zip")) {
		if (loadFileInZip(dest, fName, "cb6105.bin", 0x4000) == 0) {
			if (loadFileInZip(dest+0x4000, fName, "cb6106.bin", 0x4000) == 0) {
				if (loadFileInZip(dest+0x8000, fName, "cb6107.bin", 0x2000) == 0) {
					size = 0x10000;
					strlcpy(currentFilename, "Champion Boxing SG-AC", sizeof(currentFilename));
					gEmuFlags |= SGAC_MODE;
					gArcadeGameSet = AC_CHAMPION_BOXING;
					dipSwitch0 = 0x40;
				}
			}
		}
	}
	else if (strstr(fNameLowercase, "chwrestl.zip")) {
		if (loadFileInZip(dest, fName, "5732", 0x4000) == 0) {
			if (loadFileInZip(dest+0x4000, fName, "5733", 0x4000) == 0) {
				if (loadFileInZip(dest+0x8000, fName, "5734", 0x4000) == 0) {
					size = 0x10000;
					strlcpy(currentFilename, "Champion Wrestling SG-AC", sizeof(currentFilename));
					gEmuFlags |= SGAC_MODE;
					gArcadeGameSet = AC_CHAMPION_WRESTLING;
					dipSwitch0 = 0xC0;
				}
			}
		}
	}
	else if (strstr(fNameLowercase, "dokidoki.zip")) {
		if (loadFileInZip(dest, fName, "epr-7356.ic1", 0x4000) == 0) {
			if (loadFileInZip(dest+0x4000, fName, "epr-7357.ic2", 0x4000) == 0) {
				if (loadFileInZip(dest+0x8000, fName, "epr-7358.ic3", 0x4000) == 0) {
					size = 0x10000;
					strlcpy(currentFilename, "Doki Doki Penguin SG-AC", sizeof(currentFilename));
					gEmuFlags |= SGAC_MODE;
					gArcadeGameSet = AC_DOKI_DOKI_PENGUIN;
					dipSwitch0 = 0xC0;
				}
			}
		}
	}
	return size;
}

void strcasel(char *dest, const char *source) {
	int i;

	strncpy(dest, source, 15);
	for (i = 0; i < 16; i++) {
		if (dest[i] > 0x40 && dest[i] < 0x5B) dest[i] |= 0x20;
	}
}

//  Hang-On Jr., Sega System E
//   Game ID# 833-5911 HANG ON JR. REV.
//   ROM BD # 834-5910 REV.B
//
// Analog control board:  834-5805 (required for game to boot)
// ICs on this board are LS244 (IC1), ADC0804 (IC2), LS367 (IC3) and CD4051 (IC4).
static const ArcadeRom hangOnJrRoms[5] = {
	{"epr-7257b.ic7", 0x8000, 0xd63925a7},
	{"epr-7258.ic5",  0x8000, 0xee3caab3},
	{"epr-7259.ic4",  0x8000, 0xd2ba9bc9},
	{"epr-7260.ic3",  0x8000, 0xe14da070},
	{"epr-7261.ic2",  0x8000, 0x3810cbf5},
};

//  Slap Shooter, Sega System E
//   ROM BD # 834-5930
static const ArcadeRom slapShooterRoms[5] = {
	{"epr-7351.ic7", 0x8000, 0x894adb04},
	{"epr-7352.ic5", 0x8000, 0x61c938b6},
	{"epr-7353.ic4", 0x8000, 0x8ee2951a},
	{"epr-7354.ic3", 0x8000, 0x41482aa0},
	{"epr-7355.ic2", 0x8000, 0xc67e1aef},
};

//  Transformers, Sega System E
//   Game ID# 833-5927-01
//   ROM BD # 834-5929-01
static const ArcadeRom transformerRoms[5] = {
	{"epr-7605.ic7", 0x8000, 0xccf1d123},
	{"epr-7347.ic5", 0x8000, 0xdf0f639f},
	{"epr-7348.ic4", 0x8000, 0x0f38ea96},
	{"epr-7606.ic3", 0x8000, 0x9d485df6},
	{"epr-7350.ic2", 0x8000, 0x0052165d},
};

//  Astro Flash (Japan), Sega System E
static const ArcadeRom astroFlashRoms[5] = {
	{"epr-7723.ic7", 0x8000, 0x66061137},
	{"epr-7347.ic5", 0x8000, 0xdf0f639f},
	{"epr-7348.ic4", 0x8000, 0x0f38ea96},
	{"epr-7349.ic3", 0x8000, 0xf8c352d5},
	{"epr-7350.ic2", 0x8000, 0x0052165d},
};

//  Riddle of Pythagoras (Japan), Sega System E
//   Game ID# 833-6200 ピタゴラス ノ ナゾ
//   I/O board 834-6193 © SEGA 1986
static const ArcadeRom riddleOfPythagorasRoms[5] = {
	{"epr-10426.bin", 0x8000, 0x4404c7e7},
	{"epr-10425.bin", 0x8000, 0x35964109},
	{"epr-10424.bin", 0x8000, 0xfcda1dfa},
	{"epr-10423.bin", 0x8000, 0x0b87244f},
	{"epr-10422.bin", 0x8000, 0x14781e56},
};

//  Opa Opa (MC-8123, 317-0042), Sega System E
//   Game ID# 833-6407-01 OPA OPA
static const ArcadeRom opaOpaRoms[6] = {
	{"epr-11054.ic7", 0x8000, 0x024b1244},
	{"epr-11053.ic5", 0x8000, 0x6bc41d6e},
	{"epr-11052.ic4", 0x8000, 0x395c1d0a},
	{"epr-11051.ic3", 0x8000, 0x4ca132a2},
	{"epr-11050.ic2", 0x8000, 0xa165e2ef},
	{"317-0042.key",  0x2000, 0xd6312538},
};

//  Opa Opa, Sega System E
//   Game ID# 833-6407 OPA OPA
static const ArcadeRom opaOpaNRoms[5] = {
	{"epr-11023a.ic7", 0x8000, 0x101c5c6a},
	{"epr-11022.ic5",  0x8000, 0x15203a42},
	{"epr-11021.ic4",  0x8000, 0xb4e83340},
	{"epr-11020.ic3",  0x8000, 0xc51aad27},
	{"epr-11019.ic2",  0x8000, 0xbd0a6248},
};

//  Fantasy Zone II - The Tears of Opa-Opa (MC-8123, 317-0057), Sega System E
static const ArcadeRom fantasyZone2Roms[6] = {
	{"epr-11416.ic7", 0x8000,  0x76db7b7b},
	{"epr-11415.ic5", 0x10000, 0x57b45681},
	{"epr-11413.ic3", 0x10000, 0xa231dc85},
	{"epr-11414.ic4", 0x10000, 0x6f7a9f5f},
	{"epr-11412.ic2", 0x10000, 0xb14db5af},
	{"317-0057.key",  0x2000,  0xee43d0f0},
};

//  Tetris (Japan), Sega System E
static const ArcadeRom tetrisRoms[3] = {
	{"epr-12213.7", 0x8000, 0xef3c7a38},
	{"epr-12212.5", 0x8000, 0x28b550bf},
	{"epr-12211.4", 0x8000, 0x5aa114e9},
};

//  Megumi Rescue, Sega System E
static const ArcadeRom megumiRescueRoms[5] = {
	{"megumi_rescue_version_10.30_final_version_ic-7.ic7", 0x8000, 0x490d0059},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic5", 0x8000, 0x278caba8},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic4", 0x8000, 0xbda242d1},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic3", 0x8000, 0x56e36f85},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic2", 0x8000, 0x5b74c767},
};

const ArcadeGame systemeGames[10] = {
	AC_GAME("hangonjr", "Hang-On Jr. (Rev. B)", hangOnJrRoms)
	AC_GAME("slapshtr", "Slap Shooter", slapShooterRoms)
	AC_GAME("transfrm", "Transformer", transformerRoms)
	AC_GAME("astrofl",  "Astro Flash (Japan)", astroFlashRoms)
	AC_GAME("ridleofp", "Riddle of Pythagoras (Japan)", riddleOfPythagorasRoms)
	AC_GAME("opaopa",   "Opa Opa (MC-8123, 317-0042)", opaOpaRoms)
	AC_GAME("opaopan",  "Opa Opa (Rev A, unprotected)", opaOpaNRoms)
	AC_GAME("fantzn2",  "Fantasy Zone II - The Tears of Opa-Opa (MC-8123, 317-0057)", fantasyZone2Roms)
	AC_GAME("tetrisse", "Tetris (Japan, System E)", tetrisRoms)
	AC_GAME("megrescu", "Megumi Rescue", megumiRescueRoms)
};
