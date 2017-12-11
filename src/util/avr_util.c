#include "avr_util.h"

void adc_init()
{
    /*
        Set REFS0 to use AVCC external reference with a capacitor from AREF to ground.
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
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS2); // ADEN after waking up from sleep - we should disable upon going to sleep mode to reduce power consumption
    /*
       Set these bits to disable the digital buffering layer for specified pin.  We don't need the digital buffer for these pins, as we are either
       using them as analog inputs (pins ADC0 and ADC1 in this case) or not using them at all.
    */
    DIDR0 = (1 << ADC0D) | (1 << ADC1D);
}

/*
    Indicates whether or not an analog-to-digital conversion is in progress.
*/
bool adc_in_progress()
{
    return BIT_CHECK(ADCSRA, ADSC);
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

/*
    Starts the ADC for either the x-axis analog stick value or the y-value, depending on which is already currently selected.
    
    @param *selected_channel - The channel that was selected prior to this function call
*/
void start_x_or_y_analog_stick_adc(volatile enum Adc_Channel *selected_channel)
{
    if(*selected_channel == ANALOG_STICK_Y) {
        *selected_channel = ANALOG_STICK_X;
    } else if(*selected_channel == ANALOG_STICK_X) {
        *selected_channel = ANALOG_STICK_Y;
    }
    start_adc(*selected_channel);
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