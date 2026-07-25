#ifndef COLECO_HEADER
#define COLECO_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/// This runs all save state functions for each chip.
int colPackState(void *statePtr);

/// This runs all load state functions for each chip.
void colUnpackState(const void *statePtr);

/// Gets the total state size in bytes.
int colGetStateSize(void);

void nullUIColeco(int keyHit);

void colecoSetupBackground(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !COLECO_HEADER
