#ifndef CART_HEADER
#define CART_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 g_ROM_Size;
extern u32 g_emuFlags;
extern u8 g_cartFlags;
extern u8 g_configSet;
extern u8 g_scalingSet;
extern u8 g_machineSet;
extern u8 g_machine;
extern u8 g_region;
extern u8 gArcadeGameSet;

extern u8 EMU_SRAM[0x8000];
extern u8 ROM_Space[0x100000];
extern u8 BIOS_US_Space[0x40000];
extern u8 BIOS_JP_Space[0x2000];
extern u8 BIOS_GG_Space[0x400];
extern u8 BIOS_COLECO_Space[0x2000];
extern u8 BIOS_MSX_Space[0x8000];
extern u8 BIOS_SORDM5_Space[0x2000];
extern void *g_BIOSBASE_US;
extern void *g_BIOSBASE_JP;
extern void *g_BIOSBASE_GG;
extern void *g_BIOSBASE_COLECO;
extern void *g_BIOSBASE_MSX;
extern void *g_BIOSBASE_SORDM5;

void loadCart(int);
void ejectCart(void);

/**
* Saves the state of cart to the destination.
* @param  *destination: Where to save the state.
* @return The size of the state.
*/
int cartSaveState(void *destination);

/**
* Loads the state of cart from the source.
* @param  *source: Where to load the state from.
* @return The size of the state.
*/
int cartLoadState(const void *source);

/**
 * Gets the state size of cart state.
 * @return The size of the state.
 */
int cartGetStateSize(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CART_HEADER
