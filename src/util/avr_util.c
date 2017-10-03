#include "avr_util.h"

void usart_init() 
{
    /*
     UBRR is a 16-bit register, but only the 4 least significant bits of UBBRH are valid.
     Let's put our calculated baud rate into these registers.
    */
    UBRRH = (CALCULATED_BAUD >> 8);
    UBRRL = CALCULATED_BAUD;
    
    /*
      Set the TXEN bit in the USART control register B to enable transmission off the TX pin.
    */
    UCSRB = (1 << TXEN);
    
    /*
      Set these bits to set 8-bits in the USART control register C to set a character size of 8.
    */
    UCSRC = (1 << UCSZ0) | (1 << UCSZ1);
}

void usart_transmit(unsigned char data) 
{
    // Wait for transmit buffer to be empty
    while ( !(UCSRA & (1 << UDRE)) );
    
    UDR = data;
}

void usart_transmit_string(char* string) 
{
    for(uint8_t i = 0; i < strlen(string); i++) {
        usart_transmit(string[i]);
    }
}