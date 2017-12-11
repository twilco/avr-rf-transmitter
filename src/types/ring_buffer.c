#include "ring_buffer.h"

enum Buffer_Status ring_buffer_read(volatile struct Ring_Buffer* buffer, volatile uint8_t* byte) {
    if (buffer->newest_index == buffer->oldest_index) {
        return BUFFER_EMPTY;
    }

    *byte = buffer->data[buffer->oldest_index];
    //usart_transmit(*byte);
    buffer->oldest_index = ((buffer->oldest_index + 1) % BUFFER_SIZE);
    return BUFFER_OK;
}

enum Buffer_Status ring_buffer_write(volatile struct Ring_Buffer* buffer, volatile uint8_t byte){
    uint8_t next_index = (((buffer->newest_index) + 1) % BUFFER_SIZE);
    
    if (next_index == buffer->oldest_index) {
        return BUFFER_FULL;
    }
    
    buffer->data[buffer->newest_index] = byte;
    buffer->newest_index = next_index;
    return BUFFER_OK;
}
