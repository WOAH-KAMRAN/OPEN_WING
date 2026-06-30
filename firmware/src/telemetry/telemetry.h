#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>
#include "driver/uart.h"
#include "../ahrs/madgwick.h"
#include "../io/ibus.h"
#include "../control/flight_control.h"

#define TELEMETRY_PACKET_SIZE 64

#define TELEMETRY_FLAG_MAG_HEALTHY  0x01
#define TELEMETRY_FLAG_HOME_SET     0x02
#define TELEMETRY_FLAG_GPS_FIX      0x04

struct __attribute__((packed)) TelemetryPacket {
    uint8_t header[2];
    uint8_t version;
    uint8_t heartbeat;
    float roll, pitch, yaw;
    uint16_t rc_channels[6];
    uint8_t mode;
    uint8_t flags;
    uint16_t checksum;
    uint8_t reserved[32];
};

class Telemetry {
private:
    uart_port_t uart_num;
    TelemetryPacket packet;
    uint8_t heartbeat_count;
    
public:
    Telemetry(uart_port_t uart_num = TELEMETRY_UART_NUM);
    
    bool begin();
    void update(const AttitudeData &attitude, const IBUSData &rc_input,
                const ControlOutput &output, FlightMode mode, bool mag_healthy, bool home_set);
    void sendPacket();
    
private:
    uint16_t calculateChecksum();
};

#endif
