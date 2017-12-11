#ifndef AVR_UTIL_H_
#define AVR_UTIL_H_

#include "general_util.h"
#include "../avr_config.h"
#include "../types/general_types.h"
#include <avr/io.h>
#include <string.h>

#define BAUD_RATE 2400
#define CALCULATED_UBBR ((F_CPU / 16 / BAUD_RATE) - 1)

void adc_init();
bool adc_in_progress();
void select_adc_channel(volatile enum Adc_Channel channel);
void start_adc(volatile enum Adc_Channel channel);
void start_x_or_y_analog_stick_adc(volatile enum Adc_Channel *selected_channel);
void usart_init();
bool usart_transmission_buffer_empty();
void usart_transmit(unsigned char data);
void usart_transmit_string(const char* string, const uint8_t string_length);

#endif /* AVR_UTIL_H_ */