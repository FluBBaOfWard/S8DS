#include <nds.h>

#include "ArmTiming.h"

// Eight dependent additions plus loop control give the ARM9 enough work per
// timer tick to leave a wide gap between the 67 MHz and 134 MHz clock modes.
// Timer 2 runs from the unchanged 33.5 MHz bus clock and timer 3 extends it to
// 32 bits. The dynamic Smooth renderer restarts these same timers later.
#define ARM9_TIMING_ITERATIONS       (1U << 15)
#define ARM9_TIMING_TRIALS           4
#define ARM9_FAST_THRESHOLD_TICKS    (ARM9_TIMING_ITERATIONS * 4)

static bool detectedDSiMode;
static bool measuredFastClock;
static bool configuredFastClock;
static bool clockResultsDiffer;

static void ITCM_CODE __attribute__((noinline)) arm9TimingLoop(u32 iterations) {
	__asm__ volatile(
		"mov r1, #0\n"
		"1:\n"
		"add r1, r1, #1\n"
		"add r1, r1, #1\n"
		"add r1, r1, #1\n"
		"add r1, r1, #1\n"
		"add r1, r1, #1\n"
		"add r1, r1, #1\n"
		"add r1, r1, #1\n"
		"add r1, r1, #1\n"
		"subs %[iterations], %[iterations], #1\n"
		"bne 1b\n"
		: [iterations] "+r" (iterations)
		:
		: "r1", "cc"
	);
}

void detectArm9Timing(void) {
	// Prime the ITCM path before measuring. Take the shortest trial so an IRQ in
	// one sample cannot make a fast CPU look slow.
	arm9TimingLoop(64);
	u32 bestTicks = UINT32_MAX;
	for (int trial = 0; trial < ARM9_TIMING_TRIALS; trial++) {
		cpuStartTiming(2);
		arm9TimingLoop(ARM9_TIMING_ITERATIONS);
		u32 ticks = cpuEndTiming();
		if (ticks < bestTicks) {
			bestTicks = ticks;
		}
	}

	detectedDSiMode = isDSiMode();
	measuredFastClock = bestTicks < ARM9_FAST_THRESHOLD_TICKS;
	configuredFastClock = measuredFastClock;
	clockResultsDiffer = false;
	if (detectedDSiMode) {
		configuredFastClock = (REG_SCFG_CLK & SCFG_CLK_ARM9_TWL) != 0;
		clockResultsDiffer = configuredFastClock != measuredFastClock;
	}
}

const char *getArm9ModeText(void) {
	return detectedDSiMode ? "Mode: DSi/TWL" : "Mode: DS/NTR";
}

const char *getArm9ClockText(void) {
	if (clockResultsDiffer) {
		return configuredFastClock
			? "ARM9: 134 MHz (timer: 67)"
			: "ARM9: 67 MHz (timer: 134)";
	}
	if (measuredFastClock) {
		return detectedDSiMode
			? "ARM9: 134 MHz"
			: "ARM9: 134 MHz (boosted)";
	}
	return "ARM9: 67 MHz";
}
