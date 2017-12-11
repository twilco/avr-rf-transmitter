#include "avr_config.h"

#include "types/general_types.h"
#include "types/ring_buffer.h"
#include "util/avr_util.h"

#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

void send_packet(const char* payload, const uint8_t packet_size, uint8_t num_times);
enum Buffer_Status construct_and_store_packet(struct Ring_Buffer* buffer, const char* training_chars, const uint8_t num_training_chars, const char start_char, const char* data, const uint8_t num_data_chars, bool null_terminate);
uint8_t get_packet_size(const char* training_chars, const char start_char, const char* data, bool null_terminate);
void reinit();

void print_byte(uint8_t byte);

/* Since the ADC in AVRs output 10 bits, and the center of our joystick is represented by 524,
   these 8 bits on their own are equivalent to 12 in decimal.  To save space versus transmitting
   a full 16 bits for this value (6 wasted bits), the other two MSB bits are packed in a different byte 
   and will need to properly initialized separately. */
#define DEFAULT_ANALOG_X_Y_BYTE_VAL 0b00001100

/* The misc byte is a mashup of bits that didn't fit into other bytes.  Currently, it contains the 
   bits detailing the pressed status of three of our buttons, and the two MSB bits for each the x and y
   axes of our analog stick (AVR's ADC has 10-bit resolution).  See the byte position variables for more clarity. 
   If you change the misc_byte position variables, *this #define must change, too.* */
#define DEFAULT_MISC_BYTE 0b01010000

/* Use UUU for our preamble, or training chars.  I selected these characters because the binary value of
   the 'U' char is 01010101, which supposedly gives the receivers data slicer a nice square wave to sync up with */
const char TRAINING_CHARS[] = "UU";

/* Number of training chars being used - must match the length of the above variable. */
const uint8_t NUM_TRAINING_CHARS = 2;

/* Number of data chars being sent in the packet.  This should NOT include the checksum char.
   Currently, we have 'misc_byte', 'button_byte', 'lsb_analog_stick_x_byte', and 'lsb_analog_stick_y_byte' */
const uint8_t NUM_DATA_CHARS = 4;

/* This is the char we'll use to tell the receiver that the next byte is the first of the data bytes. */
const char START_CHAR = '>';

/* The part of the data packet indicating whether a button is pressed or unpressed. */
volatile uint8_t button_byte = 0;

/* The positions in the button_byte for each button. */
volatile const uint8_t BLUE1_BTN_BYTE_POS = 0;
volatile const uint8_t BLUE2_BTN_BYTE_POS = 1;
volatile const uint8_t PURPLE1_BTN_BYTE_POS = 2;
volatile const uint8_t PURPLE2_BTN_BYTE_POS = 3;
volatile const uint8_t PURPLE3_BTN_BYTE_POS = 4;
volatile const uint8_t BROWN1_BTN_BYTE_POS = 5;
volatile const uint8_t BROWN2_BTN_BYTE_POS = 6;
volatile const uint8_t BROWN3_BTN_BYTE_POS = 7;

/* The part of the data packet containing miscellaneous information that either didn't fit into other bytes or is 
    a one-off indicator that doesn't fit into other byte groups. */
volatile uint8_t misc_byte = DEFAULT_MISC_BYTE;

/* Below list the byte positions for each part of the misc_byte. */
volatile const uint8_t LEFT_SHOULDER_BTN_BYTE_POS = 0;
volatile const uint8_t RIGHT_SHOULDER_BTN_BYTE_POS = 1;
volatile const uint8_t ANALOG_STICK_BTN_BYTE_POS = 2;
volatile const uint8_t ANALOG_STICK_X_BIT_8_POS = 3; // zero indexing in these variable names here - the analog stick value has a 10 bit resolution
volatile const uint8_t ANALOG_STICK_X_BIT_9_POS = 4;
volatile const uint8_t ANALOG_STICK_Y_BIT_8_POS = 5;
volatile const uint8_t ANALOG_STICK_Y_BIT_9_POS = 6;

/* The 8 least significant bits of the analog stick x-axis value. */
volatile uint8_t lsb_analog_stick_x_byte = DEFAULT_ANALOG_X_Y_BYTE_VAL;

/* The 8 least significant bits of the analog stick y-axis value. */
volatile uint8_t lsb_analog_stick_y_byte = DEFAULT_ANALOG_X_Y_BYTE_VAL;

/* The number of times timer2 has overflown. */
volatile uint8_t timer2_overflows = 0;

/* Flag indicating whether or not a packet should be constructed and sent off. */
volatile bool should_build_packet = false;

/* The circular buffer that will store our packets while they wait to be sent over USART. */
struct Ring_Buffer packet_buffer;

/* The ADC channel selected by the AVR's internal ADC multiplexer, determined by a set of registers. */
volatile enum Adc_Channel selected_adc_channel = NONE;

volatile struct Digital_Input_Status digital_input_status = {.analog_stick_btn_pressed = false, .purple1_btn_pressed = false, .purple2_btn_pressed = false,
                                                             .purple3_btn_pressed = false, .brown1_btn_pressed = false, .brown2_btn_pressed = false,
                                                             .brown3_btn_pressed = false, .blue1_btn_pressed = false, .blue2_btn_pressed = false,
                                                             .left_shoulder_btn_pressed = false, .right_shoulder_btn_pressed = false};

/* TODO: Blink LED and zero out sleep counter on packet change */

int main(void)
{
    /*
        Turn on internal pull-up resistors for all of our non-analog stick digital inputs.
        The push button on the analog stick is active high, so (unfortunately) an external 
        pull-down resistor is necessary.
        
        If you change the pin a button is plugged in to, you may need to update these pull-ups, 
        and will definitely need to update avr_config.h.
    */
    PORTB |= (1 << PINB0) | (1 << PINB1) | (1 << PINB2);
    PORTC |= (1 << PINC2) | (1 << PINC3) | (1 << PINC4) | (1 << PINC5);
    PORTD |= (1 << PIND5) | (1 << PIND6) | (1 << PIND7);
    
    /*
       Set these bits to enable pin change interrupts for our digital buttons.
       
       As with the previous group, these will likely need to be changed if you change the pin
       that an input is plugged in to.  This group should also mostly match up the the previous group,
       excluding the analog stick digital switch, which is active high and thus does not get its pull-up
       enabled in the previous group.
    */
    PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2);
    PCMSK1 = (1 << PCINT10) | (1 << PCINT11) | (1 << PCINT12) | (1 << PCINT13);
    PCMSK2 = (1 << PCINT20) | (1 << PCINT21) | (1 << PCINT22) | (1 << PCINT23);
    
    
    //PCMSK1 = (1 << PCINT8) | (1 << PCINT9);
    
    /*
        Set these three bits to enable interrupts for all three pin change groups.
    */
    //PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2);
    
    /*
        Set TOEI2 to enable the Timer2 overflow interrupt.
    */
    TIMSK2 = (1 << TOIE2);
    
    adc_init();
    usart_init();
    
    sei();
    
    /*
        Start Timer2 with a prescaler of 256.  This result in overflows every 16.32ms.
    */
    TCCR2B = (1 << CS22) | (1 << CS21);
    
    uint8_t temp_byte;
    enum Buffer_Status buffer_status; //todo: remove
    while (1)
    {
        // If an ADC is in progress, let's wait until it's done to send our packet
        if(should_build_packet) {
            char data[NUM_DATA_CHARS];
            data[0] = button_byte;
            data[1] = misc_byte;
            data[2] = lsb_analog_stick_x_byte;
            data[3] = lsb_analog_stick_y_byte;
            
            //const uint8_t packet_size = NUM_TRAINING_CHARS + sizeof START_CHAR + NUM_DATA_CHARS + 1; // + 1 for checksum byte, which will be calculated in construct_packet()
            buffer_status = construct_and_store_packet(&packet_buffer, TRAINING_CHARS, NUM_TRAINING_CHARS, START_CHAR, data, NUM_DATA_CHARS, false);
            if(buffer_status == BUFFER_FULL) usart_transmit_string("uhoh\n", 5);
            //send_packet(packet, packet_size, 1);
            should_build_packet = false;
        }
        
       if(usart_transmission_buffer_empty() && ring_buffer_read(&packet_buffer, &temp_byte) == BUFFER_OK) {
           UDR0 = temp_byte;
       }
    }
}

void print_byte(uint8_t byte)
{
    char string[16];
    itoa(byte, string, 10);
    //usart_transmit_string(string, 16);
    usart_transmit('\n');
}

// Interrupt fired upon completion of an analog-to-digital conversion.
ISR(ADC_vect)
{
    if(selected_adc_channel == ANALOG_STICK_Y) {
        lsb_analog_stick_y_byte = ADC & 0xFF;
        check_set_or_clear(ADC, 8, &misc_byte, ANALOG_STICK_Y_BIT_8_POS);
        check_set_or_clear(ADC, 9, &misc_byte, ANALOG_STICK_Y_BIT_9_POS);
    } else if (selected_adc_channel == ANALOG_STICK_X) {
        lsb_analog_stick_x_byte = ADC & 0xFF;
        check_set_or_clear(ADC, 8, &misc_byte, ANALOG_STICK_X_BIT_8_POS);
        check_set_or_clear(ADC, 9, &misc_byte, ANALOG_STICK_X_BIT_9_POS);
    }
}

ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));
ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));

ISR(USART_TX_vect)
{
    uint8_t byte;
    
    if(usart_transmission_buffer_empty() && ring_buffer_read(&packet_buffer, &byte) == BUFFER_OK) {
        UDR0 = byte;
    }
}

ISR(PCINT0_vect)
{ 
    // One of our inputs changed (and/or bounced) - let's set/reset the timer until all inputs settle down.
    //TCNT0 = 0;
    //timer0_overflow_count = 0;
    // Start/restart Timer0 with a prescaler of 256.  With a 4MHz clock, this will run for 16.32ms before overflowing.
    //TCCR0B = (1 << CS02);
}

ISR(TIMER0_OVF_vect)
{  
    // If we have overflown once already, we know our inputs have settled down and we can actually use them
    /*if(timer0_overflow_count == 1) {
        // Stop the timer - we don't need it anymore
        TCCR0B = 0x0;
        timer0_overflow_count = 0;
        usart_transmit('c');
    } else {
        timer0_overflow_count++;
    }            
    
        // Analog stick button is active high, so no inverse operator needed for our BIT_CHECK
        set_or_clear(BIT_CHECK(ANALOG_STICK_BTN_PIN_REG, ANALOG_STICK_BTN_PIN), &misc_byte, ANALOG_STICK_BTN_BYTE_POS);
        // The rest of our buttons are active low, so an inverse operator is necessary
        set_or_clear(!BIT_CHECK(LEFT_SHOULDER_BTN_PIN_REG, LEFT_SHOULDER_BTN_PIN), &misc_byte, LEFT_SHOULDER_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(RIGHT_SHOULDER_BTN_PIN_REG, RIGHT_SHOULDER_BTN_PIN), &misc_byte, RIGHT_SHOULDER_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(PURPLE1_BTN_PIN_REG, PURPLE1_BTN_PIN), &button_byte, PURPLE1_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(PURPLE2_BTN_PIN_REG, PURPLE2_BTN_PIN), &button_byte, PURPLE2_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(PURPLE3_BTN_PIN_REG, PURPLE3_BTN_PIN), &button_byte, PURPLE3_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(BROWN1_BTN_PIN_REG, BROWN1_BTN_PIN), &button_byte, BROWN1_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(BROWN2_BTN_PIN_REG, BROWN2_BTN_PIN), &button_byte, BROWN2_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(BROWN3_BTN_PIN_REG, BROWN3_BTN_PIN), &button_byte, BROWN3_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(BLUE1_BTN_PIN_REG, BLUE1_BTN_PIN), &button_byte, BLUE1_BTN_BYTE_POS);
        set_or_clear(!BIT_CHECK(BLUE2_BTN_PIN_REG, BLUE2_BTN_PIN), &button_byte, BLUE2_BTN_BYTE_POS);
        should_send_packet = true;
        // also need to poll for analog input, ignore buttons bit
    } else if(timer0_overflow_count == 0) {
        // we only need to kick off the ADC for either the x or the y analog stick value - the ADC Complete ISR should start the conversion for the other
        start_adc(ANALOG_STICK_Y);
        selected_adc_channel = ANALOG_STICK_Y;
        timer0_overflow_count++;
    } else {
        // this probably shouldn't happen - let's reset
    }*/
}

ISR(TIMER2_OVF_vect)
{
    if(timer2_overflows == 0) {
         selected_adc_channel = ANALOG_STICK_Y;
         timer2_overflows++;
    } else if(timer2_overflows == 1) {
        selected_adc_channel = ANALOG_STICK_X;
        timer2_overflows = 0;
        should_build_packet = true;
    }
    start_adc(selected_adc_channel);
    // active high button first (analog stick button), then all active low buttons
    if(digital_input_status.analog_stick_btn_pressed == BIT_CHECK(ANALOG_STICK_BTN_PIN_REG, ANALOG_STICK_BTN_PIN)) {
        set_or_clear(BIT_CHECK(ANALOG_STICK_BTN_PIN_REG, ANALOG_STICK_BTN_PIN), &misc_byte, ANALOG_STICK_BTN_BYTE_POS);
    }
    
    if(digital_input_status.left_shoulder_btn_pressed == !BIT_CHECK(LEFT_SHOULDER_BTN_PIN_REG, LEFT_SHOULDER_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(LEFT_SHOULDER_BTN_PIN_REG, LEFT_SHOULDER_BTN_PIN), &misc_byte, LEFT_SHOULDER_BTN_BYTE_POS);
    }
    
    if(digital_input_status.right_shoulder_btn_pressed == !BIT_CHECK(RIGHT_SHOULDER_BTN_PIN_REG, RIGHT_SHOULDER_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(RIGHT_SHOULDER_BTN_PIN_REG, RIGHT_SHOULDER_BTN_PIN), &misc_byte, RIGHT_SHOULDER_BTN_BYTE_POS);
    }
    
    if(digital_input_status.purple1_btn_pressed == !BIT_CHECK(PURPLE1_BTN_PIN_REG, PURPLE1_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(PURPLE1_BTN_PIN_REG, PURPLE1_BTN_PIN), &button_byte, PURPLE1_BTN_BYTE_POS);
    }
    
    if(digital_input_status.purple2_btn_pressed == !BIT_CHECK(PURPLE2_BTN_PIN_REG, PURPLE2_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(PURPLE2_BTN_PIN_REG, PURPLE2_BTN_PIN), &button_byte, PURPLE2_BTN_BYTE_POS);
    }

    if(digital_input_status.purple3_btn_pressed == !BIT_CHECK(PURPLE3_BTN_PIN_REG, PURPLE3_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(PURPLE3_BTN_PIN_REG, PURPLE3_BTN_PIN), &button_byte, PURPLE3_BTN_BYTE_POS);
    }

    if(digital_input_status.brown1_btn_pressed == !BIT_CHECK(BROWN1_BTN_PIN_REG, BROWN1_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(BROWN1_BTN_PIN_REG, BROWN1_BTN_PIN), &button_byte, BROWN1_BTN_BYTE_POS);
    }

    if(digital_input_status.brown2_btn_pressed == !BIT_CHECK(BROWN2_BTN_PIN_REG, BROWN2_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(BROWN2_BTN_PIN_REG, BROWN2_BTN_PIN), &button_byte, BROWN2_BTN_BYTE_POS);
    }

    if(digital_input_status.brown3_btn_pressed == !BIT_CHECK(BROWN3_BTN_PIN_REG, BROWN3_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(BROWN3_BTN_PIN_REG, BROWN3_BTN_PIN), &button_byte, BROWN3_BTN_BYTE_POS);
    }

    if(digital_input_status.blue1_btn_pressed == !BIT_CHECK(BLUE1_BTN_PIN_REG, BLUE1_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(BLUE1_BTN_PIN_REG, BLUE1_BTN_PIN), &button_byte, BLUE1_BTN_BYTE_POS);
    }

    if(digital_input_status.blue2_btn_pressed == !BIT_CHECK(BLUE2_BTN_PIN_REG, BLUE2_BTN_PIN)) {
        set_or_clear(!BIT_CHECK(BLUE2_BTN_PIN_REG, BLUE2_BTN_PIN), &button_byte, BLUE2_BTN_BYTE_POS);
    }

    digital_input_status.purple1_btn_pressed = !BIT_CHECK(PURPLE1_BTN_PIN_REG, PURPLE1_BTN_PIN);
    digital_input_status.purple2_btn_pressed = !BIT_CHECK(PURPLE2_BTN_PIN_REG, PURPLE2_BTN_PIN);
    digital_input_status.purple3_btn_pressed = !BIT_CHECK(PURPLE3_BTN_PIN_REG, PURPLE3_BTN_PIN);
    
    digital_input_status.brown1_btn_pressed = !BIT_CHECK(BROWN1_BTN_PIN_REG, BROWN1_BTN_PIN);
    digital_input_status.brown2_btn_pressed = !BIT_CHECK(BROWN2_BTN_PIN_REG, BROWN2_BTN_PIN);
    digital_input_status.brown3_btn_pressed = !BIT_CHECK(BROWN3_BTN_PIN_REG, BROWN3_BTN_PIN);
    
    digital_input_status.blue1_btn_pressed = !BIT_CHECK(BLUE1_BTN_PIN_REG, BLUE1_BTN_PIN);
    digital_input_status.blue2_btn_pressed = !BIT_CHECK(BLUE2_BTN_PIN_REG, BLUE2_BTN_PIN);
    
    digital_input_status.left_shoulder_btn_pressed = !BIT_CHECK(LEFT_SHOULDER_BTN_PIN_REG, LEFT_SHOULDER_BTN_PIN);
    digital_input_status.right_shoulder_btn_pressed = !BIT_CHECK(RIGHT_SHOULDER_BTN_PIN_REG, RIGHT_SHOULDER_BTN_PIN);
    
    // All of our buttons are active low except for this one.  No inverse operator (!) needed here.
    digital_input_status.analog_stick_btn_pressed = BIT_CHECK(ANALOG_STICK_BTN_PIN_REG, ANALOG_STICK_BTN_PIN);
}

// every 10ms using timer interrupt, trigger full input poll - analog stick, analog stick btn, shoulder buttons, and PISO buttons
// transmit, and then store value from previous poll for all inputs.  if all same, increment 'unchanged_poll_cycles' - else if different from last poll, reset the counter.  
// when this counter gets to certain number (5 mins?), go to sleep and enable external wake up interrupt. 
// wake up from sleep with external interrupt from any input - disable interrupt, start polling and transmitting

// when checking for changed analog stick values, keep in mind they change without even touching the stick.  need to count a change as anything greater than 30 (?) change in any direction, since it shouldn't vary by that much just sitting still

void send_packet(const char* packet, const uint8_t packet_size, uint8_t num_times)
{
    for(uint8_t i = 0; i < num_times; i++) {
        usart_transmit_string(packet, packet_size);
    }
}

/*
    Constructs a RF packet with the necessary preamble training bytes, start byte, and checksum byte, and then stores that byte-by-byte
    in the buffer that you pass in.
    
    - The preamble, or training, bytes are used to sync up the sender and receiver, training the receiver
    to more accurately accept the actual data.
    - The start byte tells the receiver that actual data is about to follow.
    - The checksum byte adds up the data bytes, discarding any carryover, so that the receiver can do the same
    and determine if the packet was valid.
    
    An example packet might look like this, where '_' are the training bytes, '>' is the start byte, and 'X' is the checksum byte.
    
    _ _ _ > A B C D X
    
    @param buffer - The buffer to fill as you construct the packet.
    @param training_chars - The chars used for training the receiver to sync up with this transmitter before we start sending actual data.
    @param num_training_chars - The number of training chars being passed in
    @param start_char - The char used to indicate the start of the data portion of the packet
    @param data - The chars representing the data portion of the packet.
    @param num_data_chars - The number of data chars being passed in.
    @param null_terminated - Whether or not to null terminate this packet.
    @return Buffer_Status - Returns the Buffer_Status returned by the most recent write, which allows the caller to handle buffer-related issues, such as an attempted write to a full buffer.
*/enum Buffer_Status construct_and_store_packet(struct Ring_Buffer* buffer, const char* training_chars, const uint8_t num_training_chars, const char start_char, const char* data, const uint8_t num_data_chars, bool null_terminate)
{
    enum Buffer_Status status;
    
    //uint8_t open_idx = 0;
    for(uint8_t i = 0; i < num_training_chars; i++) {
        status = ring_buffer_write(buffer, training_chars[i]);
        //packet[open_idx++] = training_chars[i];
    }
    
    //status = ring_buffer_write(buffer, start_char);
    //packet[open_idx++] = start_char;
    
    uint8_t checksum = 0;
    for(uint8_t j = 0; j < num_data_chars; j++) {
        status = ring_buffer_write(buffer, data[j]);
        //packet[open_idx++] = data[j];
        checksum += data[j];
    }

    status = ring_buffer_write(buffer, checksum);
    //packet[open_idx++] = (char) checksum;
    
    if(null_terminate) {
        status = ring_buffer_write(buffer, '\0');
        //packet[open_idx++] = '\0';
    }
    
    return status;
}

/*
    Calculates the number of bytes necessary for our data packet.
    
    @param training_chars the chars used for training the receiver to sync up with this transmitter before we start sending
    actual data.  MUST BE NULL TERMINATED.
    @param start_char The char used to indicate the start of the data portion of the packet
    @param data The chars representing the data portion of the packet.  MUST BE NULL TERMINATED.
    @param null_terminated Whether or not to allocate space for a null character.
*/
uint8_t get_packet_size(const char* training_chars, const char start_char, const char* data, bool null_terminate)
{
    uint8_t size = ((uint8_t) strlen(training_chars)) + sizeof start_char + ((uint8_t) strlen(data)) + 1; // + 1 to include space for our checksum byte
    //char string[32];
    //itoa(strlen(data), string, 10);
    //usart_transmit_string(string);
    if(null_terminate) {
        return (size + 1);
    }
    return size;
}

/*
   Reinitializes application state.
*/
void reinit()
{
    
}