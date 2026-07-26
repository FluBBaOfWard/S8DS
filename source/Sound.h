#ifndef SOUND_HEADER
#define SOUND_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <maxmod9.h>
#include "SN76496/SN76496.h"
#include "AY38910/AY38910.h"
#include "SCC/SCC.h"

#define sample_rate  55920
#define buffer_size  (512+10)

extern SN76496 SN76496_0;
extern SN76496 SN76496_1;
extern AY38910 AY38910_0;
extern SCC SCC_0;
void soundInit(void);
void soundSetFrequency(void);
void soundSetMuteGUI(void);
mm_word soundRender(mm_word length, mm_addr dest, mm_stream_formats format);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !SOUND_HEADER
