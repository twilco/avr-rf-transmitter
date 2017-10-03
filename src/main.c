#include "avr_config.h"

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRD |= (1 << GREEN_LED);
    while (1)
    {
        PORTD = (1 << GREEN_LED);
        _delay_ms(1000);
        PORTD = 0x0;
        _delay_ms(1000);
    }
}

