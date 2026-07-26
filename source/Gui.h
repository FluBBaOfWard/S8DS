#ifndef GUI_HEADER
#define GUI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

void setupGUI(void);
void enterGUI(void);
void exitGUI(void);
void nullUINormal(int key);
void nullUIDebug(int key);

void uiNullNormal(void);

void ejectGame(void);
void sdscHandler(const unsigned char sdscChar);
void debugIOUnimplR(u16 port);
void debugIOUnimplW(u8 val, u16 port);
void cartridgePortTouched(int keyHit);
void powerOnOff(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !GUI_HEADER
