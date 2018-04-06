#ifndef AVR_UTIL_H_
#define AVR_UTIL_H_

#include "general_util.h"
#include "../avr_config.h"
#include "../types/general_types.h"

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

void disable_pcint(enum Pcint_Group group);
void enable_pcint(enum Pcint_Group group);
void enter_sleep();
void exit_sleep();

#endif /* AVR_UTIL_H_ */