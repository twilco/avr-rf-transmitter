#include "rfm69.h"

void rfm69_init()
{
    
}

void rfm69_set_power_level()
{
    
}

void rfm69_write_reg(uint8_t reg_addr, uint8_t value)
{
    select_slave(SS_PORT, SS_PIN);
    
    // Set read/write bit to indicate we're writing, not reading the register
    spi_transceieve(BIT_SET(reg_addr, RFM69_REG_READ_WRITE_BIT_LOCATION));
    spi_transceieve(value);
    
    unselect_slave(SS_PORT, SS_PIN);
}

uint8_t rfm69_read_reg(uint8_t reg_addr)
{
    select_slave(SS_PORT, SS_PIN);
        
    // Clear read/write bit to indicate we're reading, not writing to the register
    spi_transceieve(BIT_CLEAR(reg_addr, RFM69_REG_READ_WRITE_BIT_LOCATION));
    // Send some dummy data to clock out the register value from the RFM69
    uint8_t reg_val = spi_transceieve(0);
        
    unselect_slave(SS_PORT, SS_PIN);
    return reg_val;
}

