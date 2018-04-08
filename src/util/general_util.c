#include "general_util.h"

/*
   Sets a bit on the 'write_to' integer if the 'bit_to_check' is set on our 'val_to_check' - clears the bit otherwise.
   
   @param val_to_check - bytes to check
   @param bit_to_check - the bit to check on 'val_to_check' - zero indexed
   @param write_to - the place to either set or clear the bit
   @param bit_pos_to_write_to - the position of the bit to write to - zero indexed
*/
void check_set_or_clear(uint16_t val_to_check, uint8_t bit_to_check, volatile uint8_t *write_to, uint8_t bit_pos_to_write_to)
{
    if(BIT_IS_SET(val_to_check, bit_to_check)) {
        BIT_SET(*write_to, bit_pos_to_write_to);
    } else {
        BIT_CLEAR(*write_to, bit_pos_to_write_to);
    }
}

/*
   Sets or clears a bit determined by the passed in bool.
   
   @param set - true to set the bit, false to clear the bit
   @param write_to - memory address of the place to set/clear the bit in question
   @param bit_pos - position to write to
*/
void set_or_clear(bool set, volatile uint8_t *write_to, uint8_t bit_pos)
{
    if(set) {
        BIT_SET(*write_to, bit_pos);
    } else {
        BIT_CLEAR(*write_to, bit_pos);
    }
}