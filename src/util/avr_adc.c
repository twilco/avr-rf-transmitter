#include "avr_adc.h"

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