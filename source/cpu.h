#ifndef CPU_HEADER
#define CPU_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u8 waitMaskIn;
extern u8 waitMaskOut;

void cpuReset(void);
void run(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CPU_HEADER
