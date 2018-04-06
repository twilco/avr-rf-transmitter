#include "avr_util.h"

void disable_pcint(enum Pcint_Group group)
{
    switch(group) {
        case ALL_GROUPS:
            BIT_CLEAR(PCICR, PCIE0);
            BIT_CLEAR(PCICR, PCIE1);
            BIT_CLEAR(PCICR, PCIE2);
            break;
        case PCINT_0_7:
            BIT_CLEAR(PCICR, PCIE0);
            break;
        case PCINT_8_14:
            BIT_CLEAR(PCICR, PCIE1);
            break;
        case PCINT_16_23:
            BIT_CLEAR(PCICR, PCIE2);
            break;
    }
}

void enable_pcint(enum Pcint_Group group)
{
    switch(group) {
        case ALL_GROUPS:
            BIT_SET(PCICR, PCIE0);
            BIT_SET(PCICR, PCIE1);
            BIT_SET(PCICR, PCIE2);
            break;
        case PCINT_0_7:
            BIT_SET(PCICR, PCIE0);
            break;
        case PCINT_8_14:
            BIT_SET(PCICR, PCIE1);
            break;
        case PCINT_16_23:
            BIT_SET(PCICR, PCIE2);
            break;
    }
}

// Performs the necessary pre-sleep housekeeing items, and then puts the uC to sleep to preserver power.
void enter_sleep()
{
    sei();
    enable_pcint(ALL_GROUPS);
    power_adc_disable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    /* 
      The sleep_mode() function enables sleep mode and then executes a SLEEP instruction.  When the uC wakes
      up (likely due to an interrupt), the code in the interrupt will be run, and then the sleep_mode() function 
      will finish off by disabling sleep mode (clearing the SE, sleep enable, bit in the SMCR register).  Then control 
      continues as normal to the rest of this (enter_sleep()) function.
    */
    sleep_mode();
    disable_pcint(ALL_GROUPS);
}

// This function handles post-sleep house keeping items to get our uC back up and ready to go.
void exit_sleep()
{
    power_adc_enable();
}