#ifndef AVR_SPI_H_
#define AVR_SPI_H_

#include "general_util.h"

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define DDR_SPI DDRB
#define SS_PIN PINB2
#define MOSI_PIN PINB3
#define SCK_PIN PINB5

void master_spi_init();
uint8_t spi_transmit(uint8_t data);
bool spi_transmission_complete();

#endif /* AVR_SPI_H_ */