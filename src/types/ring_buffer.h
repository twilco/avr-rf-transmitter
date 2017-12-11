#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "../util/avr_util.h"

#include <stdint.h>

/* Use a power of two for your buffer size so when we % it later, the compiler can optimize to something that isn't division. */
#define BUFFER_SIZE 512

struct Ring_Buffer {
    uint8_t data[BUFFER_SIZE];
    uint8_t newest_index;
    uint8_t oldest_index;
};

enum Buffer_Status {
    BUFFER_FULL,
    BUFFER_EMPTY,
    BUFFER_OK
};

enum Buffer_Status ring_buffer_read(volatile struct Ring_Buffer* buffer, volatile uint8_t* byte);
enum Buffer_Status ring_buffer_write(volatile struct Ring_Buffer* buffer, volatile uint8_t byte);

#endif /* RING_BUFFER_H */