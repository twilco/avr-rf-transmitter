#ifndef TYPES_H_
#define TYPES_H_

#include <stdbool.h>
#include <stdint.h>

enum Adc_Channel {
    ADC0_PIN,
    ADC1_PIN,
    ADC2_PIN,
    ADC3_PIN,
    ADC4_PIN,
    ADC5_PIN,
    INTERAL_TEMP_SENSOR,
    NONE
} Adc_Channel;

/* 
   Type definition for PCINT pin groups.  PCINT_0_7 corresponds to the
   PCINT group that triggers an interrupt for PCINT pins 0 through 7. 
*/
enum Pcint_Group {
    PCINT_0_7,
    PCINT_8_14,
    PCINT_16_23,
    ALL_GROUPS
} Pcint_Group;

struct Digital_Input_Status {
    bool analog_stick_btn_pressed;
    
    bool purple1_btn_pressed;
    bool purple2_btn_pressed;
    bool purple3_btn_pressed;
    
    bool brown1_btn_pressed;
    bool brown2_btn_pressed;
    bool brown3_btn_pressed;
    
    bool blue1_btn_pressed;
    bool blue2_btn_pressed;
    
    bool left_shoulder_btn_pressed;
    bool right_shoulder_btn_pressed;
};

#endif /* TYPES_H_ */