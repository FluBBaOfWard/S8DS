

//-----------------------------------------------------------machine
#define HW_AUTO			(0x00)
#define HW_SG1000		(0x01)
#define HW_SC3000		(0x02)
#define HW_OMV			(0x03)
#define HW_SG1000II		(0x04)
#define HW_MARK3		(0x05)
#define HW_SMS1			(0x06)
#define HW_SMS2			(0x07)
#define HW_GG			(0x08)
#define HW_MEGADRIVE	(0x09)
#define HW_COLECO		(0x0A)
#define HW_MSX			(0x0B)
#define HW_SORDM5		(0x0C)
#define HW_SYSE			(0x0D)
#define HW_SGAC			(0x0E)
#define HW_MEGATECH		(0x0F)

#define HW_SELECT_END	(0x0D)
//-----------------------------------------------------------region
#define REGION_AUTO		0
#define REGION_US		1
#define REGION_PAL		2
#define REGION_JAPAN	3
//-----------------------------------------------------------cartflags
#define SRAMFLAG		(0x02)		// Save SRAM
//-----------------------------------------------------------emuflags
#define PALTIMING		(0x0001)	// 0=NTSC 1=PAL timing
#define COUNTRY			(0x0002)	// 0=Export 1=JAP
#define GG_MODE			(0x0004)	// 0=SMS mode, 1=GG mode
#define SG_MODE			(0x0008)	// 0=SMS mode, 1=SG mode
#define SC_MODE			(0x0010)	// 0=SMS mode, 1=SC mode
#define OMV_MODE		(0x0020)	// 0=SMS mode, 1=OMV mode
#define MD_MODE			(0x0040)	// 0=SMS mode, 1=MegaDrive mode
#define SGAC_MODE		(0x0080)	// 0=SMS mode, 1=SG-Arcade mode
#define COL_MODE		(0x0100)	// 0=SMS mode, 1=Coleco mode
#define MSX_MODE		(0x0200)	// 0=SMS mode, 1=MSX mode
#define SORDM5_MODE		(0x0400)	// 0=SMS mode, 1=SordM5 mode
#define SYSE_MODE		(0x0800)	// 0=SMS mode, 1=System E mode
#define MT_MODE			(0x1000)	// 0=SMS mode, 1=MegaTech mode


#define SCALED_1_1		0		// Display types
#define SCALED_FIT		1
#define SCALED_ASPECT	2

//------------------------------------------------------- Arcade games
#define AC_NONE					(0x00)
#define AC_CHAMPION_BOXING		(0x01)
#define AC_CHAMPION_WRESTLING	(0x02)
#define AC_DOKI_DOKI_PENGUIN	(0x03)
#define AC_SUPER_DERBY			(0x04)
#define AC_SUPER_DERBY_II		(0x05)
#define AC_HANG_ON_JR			(0x10)
#define AC_SLAP_SHOOTER			(0x11)
#define AC_TRANSFORMER			(0x12)
#define AC_ASTRO_FLASH			(0x13)
#define AC_RIDDLE_OF_PYTHAGORAS	(0x14)
#define AC_OPA_OPA				(0x15)
#define AC_OPA_OPA_N			(0x16)
#define AC_FANTASY_ZONE_2		(0x17)
#define AC_TETRIS				(0x18)
#define AC_MEGUMI_RESCUE		(0x19)
#define AC_MT_GREAT_GOLF		(0x20)
#define AC_MT_GREAT_SOCCER		(0x21)
#define AC_MT_OUT_RUN			(0x22)
#define AC_MT_ALIEN_SYNDROME	(0x23)
#define AC_MT_SHINOBI			(0x24)
#define AC_MT_FANTASY_ZONE		(0x25)
#define AC_MT_AFTER_BURNER		(0x26)
#define AC_MT_ASTRO_WARRIOR		(0x27)
#define AC_MT_KUNG_FU_KID		(0x28)
#define AC_MT_GREAT_FOOTBALL	(0x29)
#define AC_MT_PARLOUR_GAMES		(0x2A)

