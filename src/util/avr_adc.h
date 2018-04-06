#ifndef AVR_ADC_H_
#define AVR_ADC_H_

#include "general_util.h"
#include "../types/general_types.h"

#include <avr/io.h>

void adc_init();
bool adc_in_progress();
void select_adc_channel(volatile enum Adc_Channel channel);
void start_adc(volatile enum Adc_Channel channel);

#endif /* AVR_ADC_H_ */