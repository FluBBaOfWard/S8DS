#ifndef ARM_TIMING_HEADER
#define ARM_TIMING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

void detectArm9Timing(void);
const char *getArm9ModeText(void);
const char *getArm9ClockText(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ARM_TIMING_HEADER
