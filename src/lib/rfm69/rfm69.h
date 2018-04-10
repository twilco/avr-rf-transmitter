#ifndef RFM69_H_
#define RFM69_H_

#include "../../util/avr_spi.h"
#include "rfm69_registers.h"

#include <stdint.h>

#define RFM69_REG_READ_WRITE_BIT_LOCATION 7

void rfm69_init();
void rfm69_set_power_level();
void rfm69_write_reg(uint8_t reg_addr, uint8_t value);
uint8_t rfm69_read_reg(uint8_t reg_addr);

#endif /* RFM69_H_ */