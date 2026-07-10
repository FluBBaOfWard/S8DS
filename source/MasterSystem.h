#ifndef MASTERSYSTEM_HEADER
#define MASTERSYSTEM_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/// This runs all save state functions for each chip.
int smsPackState(void *statePtr);

/// This runs all load state functions for each chip.
void smsUnpackState(const void *statePtr);

/// Gets the total state size in bytes.
int smsGetStateSize(void);

void nullUISG1000(int keyHit);
void nullUIOMV(int keyHit);
void nullUISC3000(int keyHit);
void nullUISG1000II(int keyHit);
void nullUIMark3(int keyHit);
void nullUISMS1(int keyHit);
void nullUISMS2(int keyHit);
void nullUIMD(int keyHit);

void setupSG1000Background(void);
void setupOMVBackground(void);
void setupSC3000Background(void);
void setupSG1000IIBackground(void);
void setupMARK3Background(void);
void setupSMS1Background(void);
void setupSMS2Background(void);
void setupGGBackground(void);
void setupMDBackground(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MASTERSYSTEM_HEADER
