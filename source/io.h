#ifndef IO_HEADER
#define IO_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 joyCfg;
extern u8 colecoKey;
extern u8 sc3Keyboard;
extern u8 keyboardRows[8];
extern u8 inputHW;
extern u8 ym2413Enabled;
extern int coinCounter0;
extern int coinCounter1;
extern u8 dipSwitch0;
extern u8 dipSwitch1;
extern u32 EMUinput;

/**
 * Convert device input keys to target keys.
 * @param input NDS/GBA keys
 * @return The converted input.
 */
int convertInput(int input);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // IO_HEADER
