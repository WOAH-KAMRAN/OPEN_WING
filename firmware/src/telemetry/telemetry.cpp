#include "telemetry.h"
#include <string.h>
#include "../config/config.h"

Telemetry::Telemetry(uart_port_t uart_num_param) : uart_num(uart_num_param), heartbeat_count(0) {
    packet.header[0] = 0xAA;
    packet.header[1] = 0x55;
    packet.version = 0x02;
    memset(packet.reserved, 0, sizeof(packet.reserved));
}

bool Telemetry::begin() {
    uart_config_t uart_config = {
        .baud_rate = TELEMETRY_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    esp_err_t err = uart_param_config(uart_num, &uart_config);
    if (err != ESP_OK) {
        return false;
    }
    
    err = uart_set_pin(uart_num, TELEMETRY_TX_PIN, TELEMETRY_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        return false;
    }
    
    err = uart_driver_install(uart_num, 1024, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        return false;
    }
    
    return true;
}

uint16_t Telemetry::calculateChecksum() {
    uint16_t checksum = 0;
    uint8_t *bytes = (uint8_t *)&packet;
    uint16_t len = sizeof(TelemetryPacket) - sizeof(packet.checksum) - sizeof(packet.reserved);
    
    for (uint16_t i = 0; i < len; i++) {
        checksum += bytes[i];
    }
    
    return checksum;
}

void Telemetry::update(const AttitudeData &attitude, const IBUSData &rc_input,
                       const ControlOutput &output, FlightMode mode, bool mag_healthy, bool home_set) {
    packet.heartbeat = ++heartbeat_count;
    
    packet.roll = attitude.roll * (180.0f / M_PI);
    packet.pitch = attitude.pitch * (180.0f / M_PI);
    packet.yaw = attitude.yaw * (180.0f / M_PI);
    
    for (int i = 0; i < 6; i++) {
        packet.rc_channels[i] = rc_input.channels[i];
    }
    
    packet.mode = (uint8_t)mode;
    
    packet.flags = 0;
    if (mag_healthy) packet.flags |= TELEMETRY_FLAG_MAG_HEALTHY;
    if (home_set) packet.flags |= TELEMETRY_FLAG_HOME_SET;
    
    packet.checksum = calculateChecksum();
}

void Telemetry::sendPacket() {
    uart_write_bytes(uart_num, (const char *)&packet, sizeof(TelemetryPacket));
}
