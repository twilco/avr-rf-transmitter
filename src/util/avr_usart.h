#ifndef AVR_USART_H_
#define AVR_USART_H_

#include "../avr_config.h"

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define BAUD_RATE 2400
#define CALCULATED_UBBR ((F_CPU / 16 / BAUD_RATE) - 1)

void usart_init();
bool usart_transmission_buffer_empty();
void usart_transmit(unsigned char data);
void usart_transmit_string(const char* string, const uint8_t string_length);

#endif /* AVR_USART_H_ */