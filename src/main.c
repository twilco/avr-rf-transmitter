#include "avr_config.h"

#include "types/general_types.h"
#include "types/ring_buffer.h"

#include "util/avr_adc.h"
#include "util/avr_spi.h"
#include "util/avr_usart.h"
#include "util/avr_util.h"
#include "util/general_util.h"

#include "lib/rfm69/rfm69.h"

#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

enum Buffer_Status construct_and_store_packet(struct Ring_Buffer* buffer, const char* training_chars, const uint8_t start_char, const uint8_t num_training_chars, const char* data, const uint8_t num_data_chars, bool null_terminate);

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

/* Number of seconds of user inactivity before the AVR should go to sleep. */
#define SECONDS_BEFORE_SLEEP (uint16_t) 900

/* Number of times Timer 2 needs to overflow before the AVR should go to sleep. */
#define EIGHT_BIT_TIMER_MAX 255
#define TIMER2_OVERFLOWS_BEFORE_SLEEP (uint32_t) (SECONDS_BEFORE_SLEEP / (float) ((EIGHT_BIT_TIMER_MAX * TIMER2_PRESCALER) / (float) F_CPU))

/* Use UU for our preamble, or training chars.  I selected these characters because the binary value of
   the 'U' char is 01010101, which supposedly gives the receivers data slicer a nice square wave to sync up with */
const char TRAINING_CHARS[] = "U";

/* Number of training chars being used - must match the length of the above variable. */
const uint8_t NUM_TRAINING_CHARS = 1;

/* This is the char we'll use to tell the receiver that any bytes that follow are actual data bytes. */
const char START_CHAR = 0b10101010;

/* Number of data chars being sent in the packet.  This should NOT include the checksum char.
   Currently, we have 'misc_byte', 'button_byte', 'lsb_analog_stick_x_byte', and 'lsb_analog_stick_y_byte' */
const uint8_t NUM_DATA_CHARS = 4;

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

/* Timer2 overflow counter that is used when determining whether or not to put the microcontroller to sleep. */
volatile uint32_t timer2_inactivity_ovf_counter = 0;

/* Flag indicating whether or not a packet should be constructed and sent off. */
volatile bool should_construct_packet = false;

/* The circular buffer that will store our packets while they wait to be sent over USART. */
struct Ring_Buffer packet_buffer;

/* The ADC channel selected by the AVR's internal ADC multiplexer, determined by a set of registers. */
volatile enum Adc_Channel selected_adc_channel = NONE;

volatile struct Digital_Input_Status digital_input_status = {.analog_stick_btn_pressed = false, .purple1_btn_pressed = false, .purple2_btn_pressed = false,
                                                             .purple3_btn_pressed = false, .brown1_btn_pressed = false, .brown2_btn_pressed = false,
                                                             .brown3_btn_pressed = false, .blue1_btn_pressed = false, .blue2_btn_pressed = false,
                                                             .left_shoulder_btn_pressed = false, .right_shoulder_btn_pressed = false};

int main(void)
{
    /*
        Turn on internal pull-up resistors for all of our non-analog stick digital inputs.
        The push button on the analog stick is active high, so (unfortunately) an external 
        pull-down resistor is necessary.  Pull-ups are also enabled for the (currently) two
        unused pins, PIND2 and PIND3, to reduce power consumption in sleep modes and eliminate floating inputs.
        
        If you change the pin a button is plugged in to, you may need to update these pull-ups, 
        and will definitely need to update avr_config.h.
    */
    PORTB |= (1 << PINB0) | (1 << PINB1) | (1 << PINB2);
    PORTC |= (1 << PINC2) | (1 << PINC3) | (1 << PINC4) | (1 << PINC5);
    PORTD |= (1 << PIND2) | (1 << PIND3) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7);
    
    /*
       Set these bits to enable pin change interrupts for our inputs, including the two pins used for our analog 
       stick x and y values.  As with the previous group, these will likely need to be changed if you change the 
       pin that an input is plugged in to.
    */
    PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2);
    PCMSK1 = (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11) | (1 << PCINT12) | (1 << PCINT13);
    PCMSK2 = (1 << PCINT20) | (1 << PCINT21) | (1 << PCINT22) | (1 << PCINT23);
    
    /*
        Set TOEI2 to enable the Timer2 overflow interrupt.
    */
    TIMSK2 = (1 << TOIE2);
    
    adc_init();
    master_spi_init();
    usart_init();
    
    sei();
    
    /*
        Start Timer2 with a prescaler of 256.  This results in overflows every 16.32ms.
    */
    TCCR2B = (1 << CS22) | (1 << CS21);
    
    uint8_t temp_byte;
    char packet_data[NUM_DATA_CHARS];
    while (1)
    {
        if(should_construct_packet) {
            // Check to see if our packet data has changed this the last packet was sent.  If so, let's reset our inactivity counter, since the user has interacted with button(s) and/or the analog stick.
            if(packet_data[0] != button_byte) {
                timer2_inactivity_ovf_counter = 0;
                packet_data[0] = button_byte;
            }
            
            if(packet_data[1] != misc_byte) {
                timer2_inactivity_ovf_counter = 0;
                packet_data[1] = misc_byte;
            }

            packet_data[2] = lsb_analog_stick_x_byte;
            packet_data[3] = lsb_analog_stick_y_byte;
            
            construct_and_store_packet(&packet_buffer, TRAINING_CHARS, START_CHAR, NUM_TRAINING_CHARS, packet_data, NUM_DATA_CHARS, false);
            should_construct_packet = false;
        }
        
       if(usart_transmission_buffer_empty() && ring_buffer_read(&packet_buffer, &temp_byte) == BUFFER_OK) {
           UDR0 = temp_byte;
       }
    }
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

ISR(PCINT0_vect, ISR_ALIASOF(PCINT2_vect));
ISR(PCINT1_vect, ISR_ALIASOF(PCINT2_vect));

// Interrupt fired whenever any of the pins configured by PCMSK change levels.
ISR(PCINT2_vect)
{
    exit_sleep();
}

ISR(TIMER2_OVF_vect)
{
    // Send a packet every other timer overflow, and alternate analog-to-digital conversions between our two analog stick axes.
    if(timer2_overflows == 0) {
        selected_adc_channel = ANALOG_STICK_Y;
        timer2_overflows++;
    } else if(timer2_overflows == 1) {
        selected_adc_channel = ANALOG_STICK_X;
        timer2_overflows = 0;
        should_construct_packet = true;
    }
    start_adc(selected_adc_channel);
   
    // If the button value matches that of the last value from the last overflow, we know (almost certainly) that
    // the value we're seeing is not a button bounce.  Let's set it in our data bytes.
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
    
    timer2_inactivity_ovf_counter++;
    
    if(timer2_inactivity_ovf_counter == TIMER2_OVERFLOWS_BEFORE_SLEEP) {
        timer2_inactivity_ovf_counter = 0;
        enter_sleep();
    }
}

// Interrupt fired once and automatically cleared by hardware upon completion of a USART transmission.
ISR(USART_TX_vect)
{
    uint8_t byte;
    if(usart_transmission_buffer_empty() && ring_buffer_read(&packet_buffer, &byte) == BUFFER_OK) {
        UDR0 = byte;
    }
}

/*
    Constructs a RF packet with the necessary preamble training bytes, data byte(s), and checksum byte, and then stores that byte-by-byte
    in the buffer that you pass in.
    
    - The preamble, or training, bytes are used to sync up the sender and receiver, training the receiver
    to more accurately accept the actual data.
    - The data byte(s) is the actual payload of your packet.
    - The checksum byte adds up the data bytes, discarding any carryover, so that the receiver can do the same
    and determine if the packet was valid.
    
    An example packet might look like this, where '_' are the training bytes, 'A', 'B', 'C', and 'D' are the data bytes, and 'X' is the checksum byte.
    
    _ _ _ > A B C D X
    
    @param buffer - The buffer to fill as you construct the packet.
    @param training_chars - The chars used for training the receiver to sync up with this transmitter before we start sending actual data.
    @param num_training_chars - The number of training chars being passed in.
    @param start_char - The char used to indicate the start of the data portion of the packet
    @param data - The chars representing the data portion of the packet.
    @param num_data_chars - The number of data chars being passed in.
    @param null_terminated - Whether or not to null terminate this packet.
    @return Buffer_Status - Returns the Buffer_Status returned by the most recent write, which allows the caller to handle buffer-related issues, such as an attempted write to a full buffer.
*/
enum Buffer_Status construct_and_store_packet(struct Ring_Buffer* buffer, const char* training_chars, const uint8_t start_char, const uint8_t num_training_chars, const char* data, const uint8_t num_data_chars, bool null_terminate)
{
    enum Buffer_Status status;
    
    for(uint8_t i = 0; i < num_training_chars; i++) {
        status = ring_buffer_write(buffer, training_chars[i]);
    }
    
    status = ring_buffer_write(buffer, start_char);
    
    uint8_t checksum = 0;
    for(uint8_t j = 0; j < num_data_chars; j++) {
        status = ring_buffer_write(buffer, data[j]);
        checksum += data[j];
    }

    status = ring_buffer_write(buffer, checksum);
    
    if(null_terminate) {
        status = ring_buffer_write(buffer, '\0');
    }
    
    return status;
}