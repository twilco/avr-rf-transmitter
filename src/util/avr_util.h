#ifndef AVR_UTIL_H_
#define AVR_UTIL_H_

#include "../avr_config.h"
#include <avr/io.h>
#include <string.h>

#define BAUD_RATE 250000
#define CALCULATED_BAUD ((F_CPU / 16 / BAUD_RATE) - 1)

void usart_init();
void usart_transmit(unsigned char data);
void usart_transmit_string(char* string);

#endif /* AVR_UTIL_H_ */