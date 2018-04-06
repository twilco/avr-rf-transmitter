#ifndef RFM69_H_
#define RFM69_H_

#include "rfm69_registers.h"

#include <stdint.h>

void rfm69_init();
void rfm69_set_power_level();
void rfm69_write_reg(uint8_t reg_addr, uint8_t new_value);
uint8_t rfm69_read_reg(uint8_t reg_addr);

#endif /* RFM69_H_ */