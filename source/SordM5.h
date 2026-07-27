#ifndef SORDM5_HEADER
#define SORDM5_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/// This runs all save state functions for each chip.
int sordM5PackState(void *statePtr);

/// This runs all load state functions for each chip.
void sordM5UnpackState(const void *statePtr);

/// Gets the total state size in bytes.
int sordM5GetStateSize(void);

void nullUISordM5(int keyHit);

void sordM5SetupBackground(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !SORDM5_HEADER
