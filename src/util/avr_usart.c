#include "avr_usart.h"

void usart_init() 
{
    /*
     UBRR is a 16-bit register, but only the 4 least significant bits of UBBRH are valid.
     Let's put our calculated baud rate into these registers.
    */
    UBRR0H = (CALCULATED_UBBR >> 8);
    UBRR0L = (CALCULATED_UBBR & 0xff);
    
    /*
      Set the TXEN bit in the USART control register B to enable transmission off the TX pin.
    */
    UCSR0B = (1 << TXEN0) | (1 << TXCIE0);
    
    /*
      Set these bits to set 8-bits in the USART control register C to set a character size of 8.
    */
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

bool usart_transmission_buffer_empty()
{
    return (UCSR0A & (1 << UDRE0));
}

void usart_transmit(unsigned char data) 
{
    // Wait for transmit buffer to be empty
    while ( !(UCSR0A & (1 << UDRE0)) );
    
    UDR0 = data;
}

void usart_transmit_string(const char* string, const uint8_t string_length) 
{
    for(uint8_t i = 0; i < string_length; i++) {
        usart_transmit(string[i]);
    }
}