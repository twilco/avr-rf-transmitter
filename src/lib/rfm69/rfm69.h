#ifndef RFM69_H_
#define RFM69_H_

#include "../../util/avr_spi.h"
#include "rfm69_registers.h"

#include <stdint.h>

// Zero-indexed - so in a BIT_SET(some_uint8_t, RFM69_REG_READ_WRITE_BIT_LOCATION), we are attempting to set the eighth bit
#define RFM69_REG_READ_WRITE_BIT_LOCATION 7

enum Rfm69_Mode {
    RFM69_MODE_LISTEN,
    RFM69_MODE_SLEEP,
    RFM69_MODE_STANDBY,
    RFM69_MODE_SYNTH,
    RFM69_MODE_RX,
    RFM69_MODE_TX
} Rfm69_Mode;

void rfm69_init();
void rfm69_set_mode();
void rfm69_set_power_level();
void rfm69_write_reg(uint8_t reg_addr, uint8_t value);
uint8_t rfm69_read_reg(uint8_t reg_addr);

#endif /* RFM69_H_ */