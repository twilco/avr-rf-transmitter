#include "avr_spi.h"

void master_spi_init()
{
    // Set MOSI and SCK as output pins, since the AVR will be operating as master in this case.  
    // Set the SS pin as an output because we never want this AVR to be a slave - it can be used as a GPIO.
    MOSI_DDR |= (1 << MOSI_PIN);
    SCK_DDR |= (1 << SCK_PIN);
    SS_DDR |= (1 << SS_PIN);
    
    // Set our slave line high to start, since slaves are generally active low.
    SS_PORT |= (1 << SS_PIN);
    
    /*
        Set SPE to enable SPI.
        Set MSTR to make this AVR act as a master.
        Set SPIE to enable interrupt when a SPI transmission has completed.
    */
    SPCR = (1 << SPE) | (1 << MSTR);
    //| (1 << SPIE);
    
    // Set SPI2X to double the SPI clock speed.
    SPSR = (1 << SPI2X);
}

void select_slave(uint8_t avr_port, uint8_t avr_pin)
{
    // SPI Slaves are almost always active low - bring the specified pin down to activate this slave
    BIT_CLEAR(avr_port, avr_pin);
}

/*
    Sends a byte of data over SPI, and returns the byte sent to us post-transmission.
    
    @return uint8_t - Byte sent back to us after we transmitted our data
*/
uint8_t spi_transceieve(uint8_t data)
{
    SPDR = data;
    // Wait until SPIF bit is set, which indicates transmission of our byte and reception of the response byte is complete
    while(!BIT_IS_SET(SPSR, SPIF));
    // SPDR now contains the byte we received after transmission of our data
    return SPDR;
}

void unselect_slave(uint8_t avr_port, uint8_t avr_pin)
{
    // SPI Slaves are almost always active low - bring the specified pin high to deactivate this slave
    BIT_SET(avr_port, avr_pin);
}