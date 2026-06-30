#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// System frequencies
#define HIGH_FREQ_RATE_HZ    400    // IMU + AHRS + control loop
#define MED_FREQ_RATE_HZ     50     // Navigation + RC processing
#define LOW_FREQ_RATE_HZ     10     // Telemetry

#define HIGH_FREQ_PERIOD_MS  (1000 / HIGH_FREQ_RATE_HZ)
#define MED_FREQ_PERIOD_MS   (1000 / MED_FREQ_RATE_HZ)
#define LOW_FREQ_PERIOD_MS   (1000 / LOW_FREQ_RATE_HZ)

// I2C configuration
#define I2C_SDA_PIN          21
#define I2C_SCL_PIN          22
#define I2C_FREQ             400000

// UART configuration
#define IBUS_UART_NUM        UART_NUM_1
#define IBUS_TX_PIN          17
#define IBUS_RX_PIN          16
#define IBUS_BAUD_RATE       115200

#define TELEMETRY_UART_NUM   UART_NUM_2
#define TELEMETRY_TX_PIN     25
#define TELEMETRY_RX_PIN     26
#define TELEMETRY_BAUD_RATE  115200

// PWM configuration
#define SERVO_AILERON_PIN    4
#define SERVO_ELEVATOR_PIN   12
#define SERVO_THROTTLE_PIN   15
#define PWM_FREQ             50
#define PWM_RESOLUTION       16
#define PWM_MIN_US           1000
#define PWM_MAX_US           2000

// Flight modes
enum FlightMode {
    MANUAL,
    FBWA,
    STABILIZE,
    RTL,
    AUTO,
    LOITER
};

// Mag failure threshold (consecutive read failures before fallback)
#define MAG_FAIL_THRESHOLD    3

// WiFi config (for webserver)
#define WEBSERVER_PORT        80
#define WIFI_AP_SSID          "OpenWing_FC"
#define WIFI_AP_PASSWORD      ""
#define WIFI_AP_CHANNEL       1

// Control parameters
struct ControlConfig {
    // Rate PID gains
    float roll_rate_kp, roll_rate_ki, roll_rate_kd;
    float pitch_rate_kp, pitch_rate_ki, pitch_rate_kd;
    
    // Angle PID gains
    float roll_angle_kp, roll_angle_ki, roll_angle_kd;
    float pitch_angle_kp, pitch_angle_ki, pitch_angle_kd;
    
    // Limits
    float max_roll_angle, max_pitch_angle;
    float max_roll_rate, max_pitch_rate;
};

extern ControlConfig control_config;

#endif
