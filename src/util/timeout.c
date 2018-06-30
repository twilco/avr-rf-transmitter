#include "timeout.h"

volatile bool timer2_timeout_active = false;
volatile uint8_t timer2_overflows_before_timeout = 0;
volatile uint8_t timer2_timeout_ovf_counter = 0;

/**
 * Performs basic set up to initiate a timeout using the Timer2 peripheral using the input number of overflows.
 *
 * @param num_overflows Number of Timer2 overflows before a timeout is considered to be complete.
 */
void start_timer2_timeout(uint8_t num_overflows)
{
    timer2_timeout_active = true;
    timer2_timeout_ovf_counter = 0;
    timer2_overflows_before_timeout = num_overflows;
}

/**
 * Determines if the timeout configured for Timer2 has completed.  Should always be preceded by a call to
 * start_timer2_timeout(num_overflows).
 *
 * The Timer2 overflow ISR should be incrementing our timer2_timeout_ovf_counter - otherwise this will never
 * return true.
 *
 * @return true if timeout is complete, false otherwise
 */
bool timer2_timeout_complete()
{
    if(!timer2_timeout_active) {
        return true;
    }
    
    return timer2_timeout_ovf_counter >= timer2_overflows_before_timeout;
}