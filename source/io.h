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

#ifdef __cplusplus
} // extern "C"
#endif

#endif // IO_HEADER
