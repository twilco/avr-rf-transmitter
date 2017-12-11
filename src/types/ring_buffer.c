#include "ring_buffer.h"

/*
    Reads the next byte in the buffer, stores it into the passed in byte address, and then advances the ring buffer's
    internal read index to the next spot.
    
    @param buffer - The buffer to work on
    @param byte - Pointer to where the read value should be written
    @return buffer_status - Status of the read operation - BUFFER_EMPTY if there's nothing in the buffer to get, BUFFER_OK otherwise
*/
enum Buffer_Status ring_buffer_read(volatile struct Ring_Buffer* buffer, volatile uint8_t* byte) {
    if (buffer->newest_index == buffer->oldest_index) {
        return BUFFER_EMPTY;
    }

    *byte = buffer->data[buffer->oldest_index];
    buffer->oldest_index = ((buffer->oldest_index + 1) % RING_BUFFER_SIZE);
    return BUFFER_OK;
}

/*
    Writes the input byte to the next spot in the ring buffer, assuming it isn't already full.  After writing, it advances the write index
    to the next spot.
    
    @param buffer - The buffer to work on
    @param byte - The byte to write
    @return buffer_status - The status of the write operation - BUFFER_FULL if there was no space in the buffer to perform the write, BUFFER_OK otherwise
*/
enum Buffer_Status ring_buffer_write(volatile struct Ring_Buffer* buffer, volatile uint8_t byte){
    uint8_t next_index = (((buffer->newest_index) + 1) % RING_BUFFER_SIZE);
    
    if (next_index == buffer->oldest_index) {
        return BUFFER_FULL;
    }
    
    buffer->data[buffer->newest_index] = byte;
    buffer->newest_index = next_index;
    return BUFFER_OK;
}
