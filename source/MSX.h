#ifndef MSX_HEADER
#define MSX_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/// This runs all save state functions for each chip.
int msxPackState(void *statePtr);

/// This runs all load state functions for each chip.
void msxUnpackState(const void *statePtr);

/// Gets the total state size in bytes.
int msxGetStateSize(void);

void nullUIMSX(int keyHit);

void msxSetupBackground(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MSX_HEADER
