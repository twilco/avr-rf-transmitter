#ifndef RFM69_H_
#define RFM69_H_

#include "rfm69_registers.h"
#include "../../avr_config.h"
#include "../../util/avr_spi.h"
#include "../../util/timeout.h"

#include <stdint.h>
#include <stdbool.h>

// Zero-indexed - so in a BIT_SET(some_uint8_t, RFM69_REG_READ_WRITE_BIT_LOCATION), we are attempting to set the eighth bit
// Set the bit to write to the register, clear the bit to read the register
#define RFM69_REG_READ_WRITE_BIT_LOCATION 7

// Approximate milliseconds to wait before triggering a timeout during RFM69 init procedures
#define RFM69_INIT_TIMEOUT_MS 50

/* Number of times Timer 2 needs to overflow to timeout during RFM69 init operations.  Add .99 at the end to 
   almost always round up to the next overflow, so we never wait less than the specified ms timeout (and usually more, but this shouldn't be as big of a deal). */
#define TIMER2_OVERFLOWS_BEFORE_RFM_INIT_TIMEOUT (uint8_t) ((RFM69_INIT_TIMEOUT_MS / TIMER2_MS_TO_OVERFLOW) + .99)

// RFM69 won't send data if it detects activity on the current RF channel - this is how long to wait before giving up on the transmission.
#define RFM69_COLLISION_AVOIDANCE_LIMIT_MS 1000

// Value used in rfm69_set_encryption() to indicate we don't want any encryption.
#define RFM69_NO_ENCRYPTION_VAL 0

/* Number of times Timer 2 needs to overflow to timeout during RFM69 collision avoidance in transmission operations.  Add .99 at the end to 
   almost always round up to the next overflow, so we never wait less than the specified ms timeout (and usually more, but this shouldn't be as big of a deal). */
#define TIMER2_OVERFLOWS_BEFORE_COLLISION_AVOIDANCE_TIMEOUT (uint8_t) ((RFM69_COLLISION_AVOIDANCE_LIMIT_MS / TIMER2_MS_TO_OVERFLOW) + .99)

enum Rfm69_Mode {
    RFM69_MODE_LISTEN,
    RFM69_MODE_SLEEP,
    RFM69_MODE_STANDBY,
    RFM69_MODE_SYNTH,
    RFM69_MODE_RX,
    RFM69_MODE_TX
} Rfm69_Mode;

extern volatile enum Rfm69_Mode rfm69_current_mode;
extern volatile bool is_rfm69hw;
extern volatile uint8_t rfm69_power_level;

void rfm69_disable_high_power_regs();
void rfm69_enable_high_power_regs();
void rfm69_init(uint16_t module_freq, uint8_t network_id);
void rfm69_init_high_power(bool is_rfm69hw);
// Must be 16 bytes - e.g. rfm69_set_encryption("ABCDEFGHIJKLMNOP");
void rfm69_set_encryption(const char* key);
void rfm69_set_mode(enum Rfm69_Mode);
void rfm69_set_power_level(uint8_t power_level);
void rfm69_write_reg(uint8_t reg_addr, uint8_t value);
uint8_t rfm69_read_reg(uint8_t reg_addr);

#endif /* RFM69_H_ */