#ifndef GUI_HEADER
#define GUI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u8 g_gammaValue;

void setupGUI(void);
void enterGUI(void);
void exitGUI(void);
void nullUINormal(int key);
void nullUIDebug(int key);

void uiNullNormal(void);

void ejectGame(void);
void sdscHandler(const unsigned char sdscChar);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GUI_HEADER
