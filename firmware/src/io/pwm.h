#ifndef PWM_H
#define PWM_H

#include <Arduino.h>
#include "driver/ledc.h"

#define PWM_FREQ_DEFAULT 50
#define PWM_RESOLUTION_DEFAULT 16

struct ServoConfig {
    int pin;
    ledc_channel_t channel;
    float min_us;
    float max_us;
    float min_value;
    float max_value;
};

#define MAX_SERVO_CHANNELS 8

class PWMOutput {
private:
    ledc_mode_t speed_mode;
    ledc_timer_bit_t timer_resolution;
    uint32_t freq_hz;
    ledc_timer_t timer_num;
    ServoConfig servo_configs[MAX_SERVO_CHANNELS];
    bool config_set[MAX_SERVO_CHANNELS];
    
public:
    PWMOutput(ledc_mode_t mode = LEDC_HIGH_SPEED_MODE);
    
    bool begin(uint32_t freq = PWM_FREQ_DEFAULT, ledc_timer_bit_t resolution = (ledc_timer_bit_t)PWM_RESOLUTION_DEFAULT);
    void configureServo(const ServoConfig &config);
    void writeServo(ledc_channel_t channel, float value);
    void writeMicroseconds(ledc_channel_t channel, uint32_t us);
};

#endif
