#ifndef FILEHANDLING_HEADER
#define FILEHANDLING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define FILEEXTENSIONS ".sms.gg.sg.sc.col.rom.mx1.mx2"

void initSettings(void);
int loadSettings(void);
int saveSettings(void);
bool loadGame(const char *gameName);
void loadState(void);
void saveState(void);
int packState(void *statePtr);
void unpackState(const void *statePtr);
int getStateSize(void);
int loadNVRAM(void);
int loadSRAM(void);
void saveNVRAM(void);
void forceSaveNVRAM(void);
void selectGame(void);
void selectUSBios(void);
void selectJPBios(void);
void selectGGBios(void);
void selectCOLECOBios(void);
void selectMSXBios(void);
void selectSORDM5Bios(void);
int loadUSBIOS(void);
int loadJPBIOS(void);
int loadGGBIOS(void);
int loadCOLECOBIOS(void);
int loadMSXBIOS(void);
int loadSORDM5BIOS(void);
void selectIPS(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHANDLING_HEADER
