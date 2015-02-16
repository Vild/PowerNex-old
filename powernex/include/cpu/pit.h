#ifndef TIMER_H_
#define TIMER_H_

#include <powernex/powernex.h>

void pit_init(uint32_t frequency);

extern uint32_t pit_tick;

#endif
