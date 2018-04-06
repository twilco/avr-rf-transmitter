#ifndef AVR_SPI_H_
#define AVR_SPI_H_

#include <stdint.h>
#include <avr/io.h>

#define DDR_SPI DDRB
#define SS_PIN PINB2
#define MOSI_PIN PINB3
#define SCK_PIN PINB5

void master_spi_init();
void spi_transmit(uint8_t data);

#endif /* AVR_SPI_H_ */