#ifndef __DSP_SYSTEM_UTILS_H_
#define __DSP_SYSTEM_UTILS_H_
#include <stddef.h>

void sleep_5_cycles(void);
void sleep_10_cycles(void);
void sleep_25_cycles(void);
void sleep_50_cycles(void);
void sleep_100_cycles(void);
void sleep_200_cycles(void);
void sleep_250_cycles(void);
unsigned int atomic_read(unsigned int *addr);

#endif  /* __DSP_SYSTEM_UTILS_H_ */
