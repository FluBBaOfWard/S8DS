#ifndef ROMLOADING_HEADER
#define ROMLOADING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

int loadArcadeROM(void *dest, const char *fName);
void strcasel(char *dest, const char *source);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ROMLOADING_HEADER
