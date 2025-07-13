#ifndef PIT_H
#define PIT_H

#include <stdint.h>

extern double time_since_boot;
extern const uint64_t base_frequency;

void sleep(uint64_t ms); // in milliseconds

uint64_t pit_get_frequency();
void pit_set_frequency(uint64_t frequency);
void pit_timer_tick();
void pit_set_divisor(uint16_t divisor);

#endif