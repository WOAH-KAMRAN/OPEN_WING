#include "pwm.h"
#include "../config/config.h"

PWMOutput::PWMOutput(ledc_mode_t mode) : speed_mode(mode),
                                          timer_resolution((ledc_timer_bit_t)PWM_RESOLUTION_DEFAULT),
                                          freq_hz(PWM_FREQ_DEFAULT),
                                          timer_num(LEDC_TIMER_0) {
    for (int i = 0; i < MAX_SERVO_CHANNELS; i++) {
        config_set[i] = false;
    }
}

bool PWMOutput::begin(uint32_t freq, ledc_timer_bit_t resolution) {
    freq_hz = freq;
    timer_resolution = resolution;
    
    ledc_timer_config_t timer_conf = {
        .speed_mode = speed_mode,
        .duty_resolution = timer_resolution,
        .timer_num = timer_num,
        .freq_hz = freq_hz,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    
    esp_err_t err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK) {
        return false;
    }
    
    return true;
}

void PWMOutput::configureServo(const ServoConfig &config) {
    gpio_set_direction((gpio_num_t)config.pin, GPIO_MODE_OUTPUT);
    
    ledc_channel_config_t ledc_conf = {
        .gpio_num = config.pin,
        .speed_mode = speed_mode,
        .channel = config.channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer_num,
        .duty = 0,
        .hpoint = 0,
    };
    
    ledc_channel_config(&ledc_conf);
    
    if (config.channel < MAX_SERVO_CHANNELS) {
        servo_configs[config.channel] = config;
        config_set[config.channel] = true;
    }
}

void PWMOutput::writeMicroseconds(ledc_channel_t channel, uint32_t us) {
    uint32_t duty = (us * freq_hz * ((1 << timer_resolution) - 1)) / 1000000;
    
    if (duty > ((1 << timer_resolution) - 1)) {
        duty = (1 << timer_resolution) - 1;
    }
    
    ledc_set_duty(speed_mode, channel, duty);
    ledc_update_duty(speed_mode, channel);
}

void PWMOutput::writeServo(ledc_channel_t channel, float value) {
    if (channel >= MAX_SERVO_CHANNELS || !config_set[channel]) {
        return;
    }
    
    ServoConfig &cfg = servo_configs[channel];
    
    float us = cfg.min_us + (value - cfg.min_value) * (cfg.max_us - cfg.min_us) / (cfg.max_value - cfg.min_value);
    
    if (us < cfg.min_us) us = cfg.min_us;
    if (us > cfg.max_us) us = cfg.max_us;
    
    writeMicroseconds(channel, (uint32_t)us);
}
