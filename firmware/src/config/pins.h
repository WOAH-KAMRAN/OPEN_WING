#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

// I2C pins
#define PIN_I2C_SDA          21
#define PIN_I2C_SCL          22

// UART pins
#define PIN_IBUS_TX          17
#define PIN_IBUS_RX          16

#define PIN_TELEMETRY_TX     25
#define PIN_TELEMETRY_RX     26

// PWM servo pins
#define PIN_SERVO_AILERON    4
#define PIN_SERVO_ELEVATOR   12
#define PIN_SERVO_THROTTLE   15

// LED indicator
#define PIN_LED_STATUS       2

// Optional: GPS UART
#define PIN_GPS_TX           33
#define PIN_GPS_RX           32

#endif
