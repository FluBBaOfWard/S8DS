#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>

#include "RomLoading.h"
#include "FileHandling.h"
#include "Gui.h"
#include "Shared/unzip/unzipnds.h"
#include "Equates.h"
#include "Cart.h"
#include "io.h"

// Mega-Tech SMS games
//--------------------
// After Burner
// Alien Syndrome
// Astro Warrior
// Black Belt?
// Enduro Racer
// Fantasy Zone
// Fantasy Zone II: The Tears of Opa Opa
// Great Football
// Great Golf
// Great Soccer
// Kung Fu Kid?
// Out Run
// Parlour Games
// Power Strike II
// Shinobi
// Spellcaster


static char *const megatechNames[] ={"Great Golf MT",   "Great Soccer MT", "Out Run MT",      "Alien Syndrom MT","Shinobi MT",      "Fantasy Zone MT", "After Burner MT", "Astro Warrior MT","Great Football MT","Parlour Games MT"};
static char *const megatechFiles[] ={"mt_ggolf.zip",    "mt_gsocr.zip",    "mt_orun.zip",     "mt_asyn.zip",     "mt_shnbi.zip",    "mt_fz.zip",       "mt_aftrb.zip",    "mt_astro.zip",    "mt_gfoot.zip",     "mt_parlg.zip"};
static char *const megatechGames[] ={"mp11129f.ic1",    "mp10747f.ic1",    "mpr-11078.ic1",   "mpr-11194.ic1",   "mp11706.ic1",     "mpr-10118.ic1",   "mp11271.ic1",     "ep13817.ic2",     "mpr-10576f.ic1",   "mp11404.ic1"};
static char *const megatechGuides[]={"epr-12368-04.ic2","epr-12368-05.ic2","epr-12368-06.ic2","epr-12368-07.ic2","epr-12368-08.ic2","epr-12368-09.bin","epr-12368-10.ic2","epr-12368-13.ic1","epr-12368-19.ic2", "epr-12368-29.ic2"};
static const int   megatechSizes[] ={0x20000,           0x20000,           0x40000,           0x40000,           0x40000,           0x20000,           0x80000,           0x20000,           0x20000,            0x20000};
int loadArcadeROM(void *dest, const char *fName) {
	int i;
	int size = 0;
	char fNameLowercase[16];

//	drawtext("Checking for Arcade Roms", 23, 0);
	strcasel(fNameLowercase, fName);
	for (i = 0; i < 10; i++) {
		if (strstr(fNameLowercase, megatechFiles[i])) {
			if (loadFileInZip(dest, fName, megatechGames[i], megatechSizes[i]) == 0) {
				size = cenHead.ucSize;
				strlcpy(currentFilename, megatechNames[i], sizeof(currentFilename));
				g_emuFlags |=MT_MODE;
				return size;
			}
		}
	}

	if (strstr(fNameLowercase, "chboxing.zip")) {
		if (loadFileInZip(dest, fName, "cb6105.bin", 0x4000) == 0) {
			if (loadFileInZip(dest+0x4000, fName, "cb6106.bin", 0x4000) == 0) {
				if (loadFileInZip(dest+0x8000, fName, "cb6107.bin", 0x2000) == 0) {
					size = 0x10000;
					strlcpy(currentFilename, "Champion Boxing SG-AC", sizeof(currentFilename));
					g_emuFlags |=SGAC_MODE;
					gArcadeGameSet = AC_CHAMPION_BOXING;
					dipSwitch0 = 0x40;
				}
			}
		}
	} else if (strstr(fNameLowercase, "chwrestl.zip")) {
		if (loadFileInZip(dest, fName, "5732", 0x4000) == 0) {
			if (loadFileInZip(dest+0x4000, fName, "5733", 0x4000) == 0) {
				if (loadFileInZip(dest+0x8000, fName, "5734", 0x4000) == 0) {
					size = 0x10000;
					strlcpy(currentFilename, "Champion Wrestling SG-AC", sizeof(currentFilename));
					g_emuFlags |=SGAC_MODE;
					gArcadeGameSet = AC_CHAMPION_WRESTLING;
					dipSwitch0 = 0xC0;
				}
			}
		}
	} else if (strstr(fNameLowercase, "dokidoki.zip")) {
		if (loadFileInZip(dest, fName, "epr-7356.ic1", 0x4000) == 0) {
			if (loadFileInZip(dest+0x4000, fName, "epr-7357.ic2", 0x4000) == 0) {
				if (loadFileInZip(dest+0x8000, fName, "epr-7358.ic3", 0x4000) == 0) {
					size = 0x10000;
					strlcpy(currentFilename, "Doki Doki Penguin SG-AC", sizeof(currentFilename));
					g_emuFlags |=SGAC_MODE;
					gArcadeGameSet = AC_DOKI_DOKI_PENGUIN;
					dipSwitch0 = 0xC0;
				}
			}
		}
	} else if (strstr(fNameLowercase, "hangonjr.zip")) {
		if (loadFileInZip(dest, fName, "rom5.ic7", 0x8000) == 0) {
			if (loadFileInZip(dest+0x8000, fName, "rom4.ic5", 0x8000) == 0) {
				if (loadFileInZip(dest+0x10000, fName, "rom3.ic4", 0x8000) == 0) {
					if (loadFileInZip(dest+0x18000, fName, "rom2.ic3", 0x8000) == 0) {
						if (loadFileInZip(dest+0x20000, fName, "rom1.ic2", 0x8000) == 0) {
							size = 0x40000;
							strlcpy(currentFilename, "Hang On Jr System-E", sizeof(currentFilename));
							g_emuFlags |=SYSE_MODE;
							gArcadeGameSet = AC_HANG_ON_JR;
						}
					}
				}
			}
		}
	} else if (strstr(fNameLowercase, "ridleofp.zip")) {
		if (loadFileInZip(dest, fName, "epr10426.bin", 0x8000) == 0) {
			if (loadFileInZip(dest+0x8000, fName, "epr10425.bin", 0x8000) == 0) {
				if (loadFileInZip(dest+0x10000, fName, "epr10424.bin", 0x8000) == 0) {
					if (loadFileInZip(dest+0x18000, fName, "epr10423.bin", 0x8000) == 0) {
						if (loadFileInZip(dest+0x20000, fName, "epr10422.bin", 0x8000) == 0) {
							size = 0x40000;
							strlcpy(currentFilename, "Riddle Of Pythagoras System-E", sizeof(currentFilename));
							g_emuFlags |=SYSE_MODE;
							gArcadeGameSet = AC_RIDDLE_OF_PYTHAGORAS;
						}
					}
				}
			}
		}
	} else if (strstr(fNameLowercase, "tetrisse.zip")) {
		if (loadFileInZip(dest, fName, "epr12213.7", 0x8000) == 0) {
			if (loadFileInZip(dest+0x8000, fName, "epr12212.5", 0x8000) == 0) {
				if (loadFileInZip(dest+0x10000, fName, "epr12211.4", 0x8000) == 0) {
					size = 0x20000;
					strlcpy(currentFilename, "Tetris System-E", sizeof(currentFilename));
					g_emuFlags |=SYSE_MODE;
					gArcadeGameSet = AC_TETRIS;
				}
			}
		}
	} else if (strstr(fNameLowercase, "transfrm.zip")) {
		if (loadFileInZip(dest, fName, "ic7.top", 0x8000) == 0) {
			if (loadFileInZip(dest+0x8000, fName, "epr-7347.ic5", 0x8000) == 0) {
				if (loadFileInZip(dest+0x10000, fName, "epr-7348.ic4", 0x8000) == 0) {
					if (loadFileInZip(dest+0x18000, fName, "ic3.top", 0x8000) == 0) {
						if (loadFileInZip(dest+0x20000, fName, "epr-7350.ic2", 0x8000) == 0) {
							size = 0x40000;
							strlcpy(currentFilename, "Transformer System-E", sizeof(currentFilename));
							g_emuFlags |=SYSE_MODE;
							gArcadeGameSet = AC_TRANSFORMER;
						}
					}
				}
			}
		}
	} else if (strstr(fNameLowercase, "astrofl.zip")) {
		if (loadFileInZip(dest, fName, "epr-7723.ic7", 0x8000) == 0) {
			if (loadFileInZip(dest+0x8000, fName, "epr-7347.ic5", 0x8000) == 0) {
				if (loadFileInZip(dest+0x10000, fName, "epr-7348.ic4", 0x8000) == 0) {
					if (loadFileInZip(dest+0x18000, fName, "epr-7349.ic3", 0x8000) == 0) {
						if (loadFileInZip(dest+0x20000, fName, "epr-7350.ic2", 0x8000) == 0) {
							size = 0x40000;
							strlcpy(currentFilename, "Astro Flash System-E", sizeof(currentFilename));
							g_emuFlags |=SYSE_MODE;
							gArcadeGameSet = AC_ASTRO_FLASH;
						}
					}
				}
			}
		}
	} else if (strstr(fNameLowercase, "fantzn2.zip")) {
		if (loadFileInZip(dest, fName, "epr-11416.ic7", 0x8000) == 0) {
			if (loadFileInZip(dest+0x8000, fName, "epr-11415.ic5", 0x10000) == 0) {
				if (loadFileInZip(dest+0x18000, fName, "epr-11414.ic4", 0x10000) == 0) {
					if (loadFileInZip(dest+0x28000, fName, "epr-11413.ic3", 0x10000) == 0) {
						if (loadFileInZip(dest+0x38000, fName, "epr-11412.ic2", 0x10000) == 0) {
							size = 0x80000;
							strlcpy(currentFilename, "Fantasy Zone 2 System-E", sizeof(currentFilename));
							g_emuFlags |=SYSE_MODE;
							gArcadeGameSet = AC_FANTASY_ZONE_2;
						}
					}
				}
			}
		}
	} else if (strstr(fNameLowercase, "opaopa.zip")) {
		if (loadFileInZip(dest, fName, "epr11224.ic7", 0x8000) == 0) {
			if (loadFileInZip(dest+0x8000, fName, "epr11223.ic5", 0x8000) == 0) {
				if (loadFileInZip(dest+0x10000, fName, "epr11222.ic4", 0x8000) == 0) {
					if (loadFileInZip(dest+0x18000, fName, "epr11221.ic3", 0x8000) == 0) {
						if (loadFileInZip(dest+0x20000, fName, "epr11220.ic2", 0x8000) == 0) {
							size = 0x40000;
							strlcpy(currentFilename, "Opa Opa System-E", sizeof(currentFilename));
							g_emuFlags |=SYSE_MODE;
							gArcadeGameSet = AC_OPA_OPA;
						}
					}
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
