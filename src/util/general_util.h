#ifndef GENERAL_UTIL_H_
#define GENERAL_UTIL_H_

#include "avr_util.h"

#include <stdbool.h>
#include <stdint.h>

#define BIT_SET(a,b) ((a) |= (1UL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1UL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1UL<<(b)))
#define BIT_CHECK(a,b) (((a) & (1UL<<(b))) ? 1 : 0)

void check_set_or_clear(uint16_t val_to_check, uint8_t bit_to_check, volatile uint8_t *write_to, uint8_t bit_pos_to_write_to);
void set_or_clear(bool set, volatile uint8_t *write_to, uint8_t bit_pos_to_write_to);

#endif /* GENERAL_UTIL_H_ */