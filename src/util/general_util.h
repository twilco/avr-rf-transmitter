#ifndef GENERAL_UTIL_H_
#define GENERAL_UTIL_H_

#include "avr_util.h"

#include <stdbool.h>
#include <stdint.h>

#define BIT_SET(target, bit_pos) ((target) |= (1UL << (bit_pos)))
#define BIT_CLEAR(target, bit_pos) ((target) &= ~(1UL << (bit_pos)))
#define BIT_FLIP(target, bit_pos) ((target) ^= (1UL << (bit_pos)))
#define BIT_IS_SET(target, bit_pos) (((target) & (1UL << (bit_pos))) ? 1 : 0)

void check_set_or_clear(uint16_t val_to_check, uint8_t bit_to_check, volatile uint8_t *write_to, uint8_t bit_pos_to_write_to);
void set_or_clear(bool set, volatile uint8_t *write_to, uint8_t bit_pos_to_write_to);

#endif /* GENERAL_UTIL_H_ */