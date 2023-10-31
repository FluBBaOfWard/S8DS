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

const ArcadeGame megaTechGames[10];
const ArcadeGame systemeGames[10];
const ArcadeGame sgacGames[5];

static const char syseDefines[] = {AC_HANG_ON_JR, AC_SLAP_SHOOTER, AC_TRANSFORMER, AC_ASTRO_FLASH, AC_RIDDLE_OF_PYTHAGORAS, AC_OPA_OPA, AC_OPA_OPA_N, AC_FANTASY_ZONE_2, AC_TETRIS, AC_MEGUMI_RESCUE};
static const char sgacDefines[] = {AC_CHAMPION_BOXING, AC_CHAMPION_WRESTLING, AC_DOKI_DOKI_PENGUIN, AC_SUPER_DERBY, AC_SUPER_DERBY_II};

int loadArcadeROM(void *dest, const char *fName) {
	int i;
	char fNameLowercase[16];
	char zipSubName[32];

	gArcadeGameSet = AC_NONE;
	dipSwitch0 = 0;
	dipSwitch1 = 0;

//	drawtext("Checking for Arcade Roms", 23, 0);
	strcasel(fNameLowercase, fName);
	for (i = 0; i < 10; i++) {
		strlMerge(zipSubName, megaTechGames[i].gameName, ".zip", sizeof(zipSubName));
		if (strstr(fNameLowercase, zipSubName)) {
			if (!loadACRoms(dest, megaTechGames, i, ARRSIZE(megaTechGames), true)) {
				strlcpy(currentFilename, megaTechGames[i].fullName, sizeof(currentFilename));
				gEmuFlags |= MT_MODE;
				return megaTechGames[i].roms[0].romSize;
			}
		}
	}

	for (i = 0; i < 10; i++) {
		strlMerge(zipSubName, systemeGames[i].gameName, ".zip", sizeof(zipSubName));
		if (strstr(fNameLowercase, zipSubName)) {
			if (!loadACRoms(dest, systemeGames, i, ARRSIZE(systemeGames), true)) {
				strlcpy(currentFilename, systemeGames[i].fullName, sizeof(currentFilename));
				gEmuFlags |= SYSE_MODE;
				gArcadeGameSet = syseDefines[i];
				return 0x80000;
			}
		}
	}

	for (i = 0; i < 5; i++) {
		strlMerge(zipSubName, sgacGames[i].gameName, ".zip", sizeof(zipSubName));
		if (strstr(fNameLowercase, zipSubName)) {
			if (!loadACRoms(dest, sgacGames, i, ARRSIZE(sgacGames), true)) {
				strlcpy(currentFilename, sgacGames[i].fullName, sizeof(currentFilename));
				gEmuFlags |= SGAC_MODE;
				dipSwitch0 = 0xC0;
				gArcadeGameSet = sgacDefines[i];
				return 0x10000;
			}
		}
	}
	return 0;
}

void strcasel(char *dest, const char *source) {
	int i;

	strlcpy(dest, source, 16);
	for (i = 0; i < 16; i++) {
		if (dest[i] > 0x40 && dest[i] < 0x5B) dest[i] |= 0x20;
	}
}

// Game 04 - Great Golf (SMS)
// Also known to have the ID# MPR-11128 instead of MPR-11129F, same contents
static const ArcadeRom mt_ggolfRoms[2] = {
	{"mp11129f.ic1",    0x20000, 0xc6611c84},
	{"epr-12368-04.ic2", 0x8000, 0x62e5579b},
};

// Game 05 - Great Soccer (SMS) - bad dump
static const ArcadeRom mt_gsocrRoms[2] = {
	{"mp10747f.ic1",    0x20000, 0x9cf53703},
	{"epr-12368-05.ic2", 0x8000, 0xbab91fcc},
};

// Game 06 - Out Run (SMS)
static const ArcadeRom mt_orunRoms[2] = {
	{"mpr-11078.ic1",   0x40000, 0x5589d8d2},
	{"epr-12368-06.ic2", 0x8000, 0xc7c74429},
};

// Game 07 - Alien Syndrome (SMS)
static const ArcadeRom mt_asynRoms[2] = {
	{"mpr-11194.ic1",   0x40000, 0x4cc11df9},
	{"epr-12368-07.ic2", 0x8000, 0x14f4a17b},
};

// Game 08 - Shinobi (SMS)
static const ArcadeRom mt_shnbiRoms[2] = {
	{"mp11706.ic1",     0x40000, 0x0c6fac4e},
	{"epr-12368-08.ic2", 0x8000, 0x103a0459},
};

// Game 09 - Fantasy Zone (SMS)
// note, dump was bad, but the good (uniquely identifiable) parts matched the 'fantasy zone (world) (v1.2).bin' SMS rom
// so I'm using that until it gets verified.
static const ArcadeRom mt_fzRoms[2] = {
	{"mpr-10118.ic1",   0x20000, 0x65d7e4e0},
	{"epr-12368-09.bin", 0x8000, 0x373d2a70},
};

// Game 10 - After Burner (SMS)
static const ArcadeRom mt_aftrbRoms[2] = {
	{"mp11271.ic1",     0x80000, 0x1c951f8e},
	{"epr-12368-10.ic2", 0x8000, 0x2a7cb590},
};

// Game 13 - Astro Warrior (SMS)
static const ArcadeRom mt_astroRoms[2] = {
	{"ep13817.ic2",     0x20000, 0x299cbb74},
	{"epr-12368-13.ic1", 0x8000, 0x4038cbd1},
};

// Game 19 - Great Football (SMS)
static const ArcadeRom mt_gfootRoms[2] = {
	{"mpr-10576f.ic1",  0x20000, 0x2055825f},
	{"epr-12368-19.ic2", 0x8000, 0xe27cb37a},
};

// Game 29 - Parlour Games (SMS)
static const ArcadeRom mt_parlgRoms[2] = {
	{"mp11404.ic1",     0x20000, 0xe030e66c},
	{"epr-12368-29.ic2", 0x8000, 0x534151e8},
};


//  Hang-On Jr., Sega System E
//   Game ID# 833-5911 HANG ON JR. REV.
//   ROM BD # 834-5910 REV.B
//
// Analog control board:  834-5805 (required for game to boot)
// ICs on this board are LS244 (IC1), ADC0804 (IC2), LS367 (IC3) and CD4051 (IC4).
static const ArcadeRom hangonjrRoms[5] = {
	{"epr-7257b.ic7", 0x8000, 0xd63925a7},
	{"epr-7258.ic5",  0x8000, 0xee3caab3},
	{"epr-7259.ic4",  0x8000, 0xd2ba9bc9},
	{"epr-7260.ic3",  0x8000, 0xe14da070},
	{"epr-7261.ic2",  0x8000, 0x3810cbf5},
};

//  Slap Shooter, Sega System E
//   ROM BD # 834-5930
static const ArcadeRom slapshtrRoms[5] = {
	{"epr-7351.ic7", 0x8000, 0x894adb04},
	{"epr-7352.ic5", 0x8000, 0x61c938b6},
	{"epr-7353.ic4", 0x8000, 0x8ee2951a},
	{"epr-7354.ic3", 0x8000, 0x41482aa0},
	{"epr-7355.ic2", 0x8000, 0xc67e1aef},
};

//  Transformers, Sega System E
//   Game ID# 833-5927-01
//   ROM BD # 834-5929-01
static const ArcadeRom transfrmRoms[5] = {
	{"epr-7605.ic7", 0x8000, 0xccf1d123},
	{"epr-7347.ic5", 0x8000, 0xdf0f639f},
	{"epr-7348.ic4", 0x8000, 0x0f38ea96},
	{"epr-7606.ic3", 0x8000, 0x9d485df6},
	{"epr-7350.ic2", 0x8000, 0x0052165d},
};

//  Astro Flash (Japan), Sega System E
static const ArcadeRom astroflRoms[5] = {
	{"epr-7723.ic7", 0x8000, 0x66061137},
	{"epr-7347.ic5", 0x8000, 0xdf0f639f},
	{"epr-7348.ic4", 0x8000, 0x0f38ea96},
	{"epr-7349.ic3", 0x8000, 0xf8c352d5},
	{"epr-7350.ic2", 0x8000, 0x0052165d},
};

//  Riddle of Pythagoras (Japan), Sega System E
//   Game ID# 833-6200 ピタゴラス ノ ナゾ
//   I/O board 834-6193 © SEGA 1986
static const ArcadeRom ridleofpRoms[5] = {
	{"epr-10426.bin", 0x8000, 0x4404c7e7},
	{"epr-10425.bin", 0x8000, 0x35964109},
	{"epr-10424.bin", 0x8000, 0xfcda1dfa},
	{"epr-10423.bin", 0x8000, 0x0b87244f},
	{"epr-10422.bin", 0x8000, 0x14781e56},
};

//  Opa Opa (MC-8123, 317-0042), Sega System E
//   Game ID# 833-6407-01 OPA OPA
static const ArcadeRom opaopaRoms[6] = {
	{"epr-11054.ic7", 0x8000, 0x024b1244},
	{"epr-11053.ic5", 0x8000, 0x6bc41d6e},
	{"epr-11052.ic4", 0x8000, 0x395c1d0a},
	{"epr-11051.ic3", 0x8000, 0x4ca132a2},
	{"epr-11050.ic2", 0x8000, 0xa165e2ef},
	{"317-0042.key",  0x2000, 0xd6312538},
};

//  Opa Opa, Sega System E
//   Game ID# 833-6407 OPA OPA
static const ArcadeRom opaopanRoms[5] = {
	{"epr-11023a.ic7", 0x8000, 0x101c5c6a},
	{"epr-11022.ic5",  0x8000, 0x15203a42},
	{"epr-11021.ic4",  0x8000, 0xb4e83340},
	{"epr-11020.ic3",  0x8000, 0xc51aad27},
	{"epr-11019.ic2",  0x8000, 0xbd0a6248},
};

//  Fantasy Zone II - The Tears of Opa-Opa (MC-8123, 317-0057), Sega System E
static const ArcadeRom fantzn2Roms[6] = {
	{"epr-11416.ic7", 0x8000,  0x76db7b7b},
	{"epr-11415.ic5", 0x10000, 0x57b45681},
	{"epr-11413.ic3", 0x10000, 0xa231dc85},
	{"epr-11414.ic4", 0x10000, 0x6f7a9f5f},
	{"epr-11412.ic2", 0x10000, 0xb14db5af},
	{"317-0057.key",  0x2000,  0xee43d0f0},
};

//  Tetris (Japan), Sega System E
static const ArcadeRom tetrisseRoms[3] = {
	{"epr-12213.7", 0x8000, 0xef3c7a38},
	{"epr-12212.5", 0x8000, 0x28b550bf},
	{"epr-12211.4", 0x8000, 0x5aa114e9},
};

//  Megumi Rescue, Sega System E
static const ArcadeRom megrescuRoms[5] = {
	{"megumi_rescue_version_10.30_final_version_ic-7.ic7", 0x8000, 0x490d0059},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic5", 0x8000, 0x278caba8},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic4", 0x8000, 0xbda242d1},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic3", 0x8000, 0x56e36f85},
	{"megumi_rescue_version_10.30_final_version_ic-7.ic2", 0x8000, 0x5b74c767},
};

// Champion Boxing SGAC
static const ArcadeRom chboxingRoms[3] = {
	{"cb6105.bin", 0x4000, 0x43516f2e},
	{"cb6106.bin", 0x4000, 0x65e2c750},
	{"cb6107.bin", 0x2000, 0xc2f8e522},
};

// Champion Wrestling SGAC (encrypted)
static const ArcadeRom chwrestlRoms[3] = {
	{"5732", 0x4000, 0xa4e44370},
	{"5733", 0x4000, 0x4f493538},
	{"5734", 0x4000, 0xd99b6301},
};

// Doki Doki Penguin Land SGAC
static const ArcadeRom dokidokiRoms[3] = {
	{"epr-7356.ic1", 0x4000, 0x95658c31},
	{"epr-7357.ic2", 0x4000, 0xe8dbad85},
	{"epr-7358.ic3", 0x4000, 0xc6f26b0b},
};

// Super Derby (satellite board) SGAC
static const ArcadeRom sderbysRoms[1] = {
	{"v1.2.ic10", 0x4000, 0xcf29b579},
};

// Super Derby II (satellite board) SGAC
static const ArcadeRom sderby2sRoms[2] = {
	{"epr-6450d.ic10", 0x4000, 0xe56986d3},
	{"epr-6504d.ic11", 0x4000, 0x7bb364b9},
};

const ArcadeGame megaTechGames[10] = {
	AC_GAME("mt_ggolf", "Great Golf (Mega-Tech, SMS based)", mt_ggolfRoms)
	AC_GAME("mt_gsocr", "Great Soccer (Mega-Tech, SMS based)", mt_gsocrRoms)
	AC_GAME("mt_orun",  "Out Run (Mega-Tech, SMS based)", mt_orunRoms)
	AC_GAME("mt_asyn",  "Alien Syndrome (Mega-Tech, SMS based)", mt_asynRoms)
	AC_GAME("mt_shnbi", "Shinobi (Mega-Tech, SMS based)", mt_shnbiRoms)
	AC_GAME("mt_fz",    "Fantasy Zone (Mega-Tech, SMS based)", mt_fzRoms)
	AC_GAME("mt_aftrb", "After Burner (Mega-Tech, SMS based)", mt_aftrbRoms)
	AC_GAME("mt_astro", "Astro Warrior (Mega-Tech, SMS based)", mt_astroRoms)
	AC_GAME("mt_gfoot", "Great Football (Mega-Tech, SMS based)", mt_gfootRoms)
	AC_GAME("mt_parlg", "Parlour Games (Mega-Tech, SMS based)", mt_parlgRoms)
};

const ArcadeGame systemeGames[10] = {
	AC_GAME("hangonjr", "Hang-On Jr. (Rev. B)", hangonjrRoms)
	AC_GAME("slapshtr", "Slap Shooter", slapshtrRoms)
	AC_GAME("transfrm", "Transformer", transfrmRoms)
	AC_GAME("astrofl",  "Astro Flash (Japan)", astroflRoms)
	AC_GAME("ridleofp", "Riddle of Pythagoras (Japan)", ridleofpRoms)
	AC_GAME("opaopa",   "Opa Opa (MC-8123, 317-0042)", opaopaRoms)
	AC_GAME("opaopan",  "Opa Opa (Rev A, unprotected)", opaopanRoms)
	AC_GAME("fantzn2",  "Fantasy Zone II - The Tears of Opa-Opa (MC-8123, 317-0057)", fantzn2Roms)
	AC_GAME("tetrisse", "Tetris (Japan, System E)", tetrisseRoms)
	AC_GAME("megrescu", "Megumi Rescue", megrescuRoms)
};

const ArcadeGame sgacGames[5] = {
	AC_GAME("chboxing", "Champion Boxing", chboxingRoms)
	AC_GAME("chwrestl", "Champion Pro Wrestling", chwrestlRoms)
	AC_GAME("dokidoki", "Doki Doki Penguin Land", dokidokiRoms)
	AC_GAME("sderbys",  "Super Derby (satellite board)", sderbysRoms)
	AC_GAME("sderby2s", "Super Derby II (satellite board)", sderby2sRoms)
};
