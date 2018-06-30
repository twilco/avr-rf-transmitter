#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <stdbool.h>
#include <stdint.h>

/* Flag indicating whether or not we should be counting up time towards some timeout value for Timer2. */
extern volatile bool timer2_timeout_active;

/* Number of Timer2 overflows before timeout is considered complete. */
extern volatile uint8_t timer2_overflows_before_timeout;

/* Timer2 overflow counter used to track time before a timeout occurs. */
extern volatile uint8_t timer2_timeout_ovf_counter;

// Performs set up necessary to start a timeout using the Timer2 peripheral.
void start_timer2_timeout(uint8_t num_overflows);

// Determines if a started timeout is complete.  Should always be preceded by a call to start_timer2_timeout().
bool timer2_timeout_complete();

#endif /* TIMEOUT_H_ */