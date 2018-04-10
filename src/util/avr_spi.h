#ifndef AVR_SPI_H_
#define AVR_SPI_H_

#include "general_util.h"

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define SS_DDR DDRB
#define SS_PORT PORTB
#define SS_PIN PINB2

#define MOSI_DDR DDRB
#define MOSI_PIN PINB3

#define SCK_DDR DDRB
#define SCK_PIN PINB5

void master_spi_init();
void select_slave(uint8_t avr_port, uint8_t avr_pin);
uint8_t spi_transceieve(uint8_t data);
void unselect_slave(uint8_t avr_port, uint8_t avr_pin);

#endif /* AVR_SPI_H_ */