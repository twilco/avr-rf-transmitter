#include "avr_spi.h"

void master_spi_init()
{
    // Set MOSI and SCK as output pins, since the AVR will be operating as master in this case.  
    // Set the SS pin as an output because we never want this AVR to be a slave - it can be used as a GPIO.
    DDR_SPI |= (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << SS_PIN);
    
    /*
        Set SPE to enable SPI.
        Set MSTR to make this AVR act as a master.
        Set SPIE to enable interrupt when a SPI transmission has completed.
    */
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPIE);
    
    // Set SPI2X to double the SPI clock speed.
    SPSR = (1 << SPI2X);
}

/*
    Sends a byte of data over SPI, and returns the byte sent to us post-transmission.
    
    @return uint8_t - Byte sent back to us after we transmitted our data
*/
uint8_t spi_transmit(uint8_t data)
{
    SPDR = data;
    // Wait for transmission of data and reception of incoming byte to complete
    while(!spi_transmission_complete());
    // SPDR now contains the byte we received after transmission of our data
    return SPDR;
}

/*
    Returns true if a SPI transmission is in progress, false otherwise
    
    @return bool
*/
bool spi_transmission_complete()
{
    return BIT_CHECK(SPSR, SPIF);
}