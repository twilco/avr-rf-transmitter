#include "avr_util.h"

void adc_init()
{
    /*
        Set REFS0 to use AVCC external reference with a capacitor from the AREF pin to ground.
    */
    ADMUX = (1 << REFS0);
    /*
        Set ADEN to turn on the ADC.
        Set ADIE to trigger interrupt upon completion of ADC conversion.
        Set ADPS0 and ADPS2 to select the 32x ADC prescaler.
            - The internal ADC circuitry requires a clock of 50-200khz to function correctly
            - We're running at a 4mhz clock / max speed of 200khz = 20
            - The next closest prescaler is 32.  4000000 / 32 = 125khz clock speed for the ADC
    */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS2);
}

/*
    Indicates whether or not an analog-to-digital conversion is in progress.
*/
bool adc_in_progress()
{
    return BIT_CHECK(ADCSRA, ADSC);
}

void disable_pcint(enum Pcint_Group group)
{
    switch(group) {
        case ALL_GROUPS:
            BIT_CLEAR(PCICR, PCIE0);
            BIT_CLEAR(PCICR, PCIE1);
            BIT_CLEAR(PCICR, PCIE2);
            break;
        case PCINT_0_7:
            BIT_CLEAR(PCICR, PCIE0);
            break;
        case PCINT_8_14:
            BIT_CLEAR(PCICR, PCIE1);
            break;
        case PCINT_16_23:
            BIT_CLEAR(PCICR, PCIE2);
            break;
    }
}

void enable_pcint(enum Pcint_Group group)
{
    switch(group) {
        case ALL_GROUPS:
            BIT_SET(PCICR, PCIE0);
            BIT_SET(PCICR, PCIE1);
            BIT_SET(PCICR, PCIE2);
            break;
        case PCINT_0_7:
            BIT_SET(PCICR, PCIE0);
            break;
        case PCINT_8_14:
            BIT_SET(PCICR, PCIE1);
            break;
        case PCINT_16_23:
            BIT_SET(PCICR, PCIE2);
            break;
    }
}

// Performs the necessary pre-sleep housekeeing items, and then puts the uC to sleep to preserver power.
void enter_sleep()
{
    sei();
    enable_pcint(ALL_GROUPS);
    power_adc_disable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    /* 
      The sleep_mode() function enables sleep mode and then executes a SLEEP instruction.  When the uC wakes
      up (likely due to an interrupt), the code in the interrupt will be run, and then the sleep_mode() function 
      will finish off by disabling sleep mode (clearing the SE, sleep enable, bit in the SMCR register).  Then control 
      continues as normal to the rest of this (enter_sleep()) function.
    */
    sleep_mode();
    disable_pcint(ALL_GROUPS);
}

// This function handles post-sleep house keeping items to get our uC back up and ready to go.
void exit_sleep()
{
    power_adc_enable();
}

void select_adc_channel(volatile enum Adc_Channel channel)
{
    switch(channel) {
        case ADC0_PIN:
            BIT_CLEAR(ADMUX, MUX0);
            BIT_CLEAR(ADMUX, MUX1);
            BIT_CLEAR(ADMUX, MUX2);
            BIT_CLEAR(ADMUX, MUX3);
            break;
        
        case ADC1_PIN:
            BIT_SET(ADMUX, MUX0);
        
            BIT_CLEAR(ADMUX, MUX1);
            BIT_CLEAR(ADMUX, MUX2);
            BIT_CLEAR(ADMUX, MUX3);
            break;
        
        case ADC2_PIN:
            BIT_SET(ADMUX, MUX1);
        
            BIT_CLEAR(ADMUX, MUX0);
            BIT_CLEAR(ADMUX, MUX2);
            BIT_CLEAR(ADMUX, MUX3);
            break;
        
        case ADC3_PIN:
            BIT_SET(ADMUX, MUX0);
            BIT_SET(ADMUX, MUX1);
            
            BIT_CLEAR(ADMUX, MUX2);
            BIT_CLEAR(ADMUX, MUX3);
            break;
        
        case ADC4_PIN:
            BIT_SET(ADMUX, MUX2);
            
            BIT_CLEAR(ADMUX, MUX0);
            BIT_CLEAR(ADMUX, MUX1);
            BIT_CLEAR(ADMUX, MUX3);
            break;
        
        case ADC5_PIN:
            BIT_SET(ADMUX, MUX0);
            BIT_SET(ADMUX, MUX2);
            
            BIT_CLEAR(ADMUX, MUX1);
            BIT_CLEAR(ADMUX, MUX3);
            break;
            
        case INTERAL_TEMP_SENSOR:
            BIT_SET(ADMUX, MUX3);
            
            BIT_CLEAR(ADMUX, MUX0);
            BIT_CLEAR(ADMUX, MUX1);
            BIT_CLEAR(ADMUX, MUX2);
            break;
            
        case NONE:
            /* This actually sets the input channel to 0V (GND).  */
            BIT_SET(ADMUX, MUX0);
            BIT_SET(ADMUX, MUX1);
            BIT_SET(ADMUX, MUX2);
            BIT_SET(ADMUX, MUX3);
    }
}

void start_adc(volatile enum Adc_Channel channel)
{
    select_adc_channel(channel);
    
    /*
        Set ADSC to start the conversion.
    */
    ADCSRA |= (1 << ADSC);
}

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