#include "ibus.h"
#include "../config/config.h"

// IBUS protocol constants
#define IBUS_HEADER_LENGTH  2
#define IBUS_HEADER_0      0x20
#define IBUS_HEADER_1      0x40

IBUS::IBUS(uart_port_t uart_num_param) : uart_num(uart_num_param),
                                          buffer_index(0) {
    // Initialize data structure
    for (int i = 0; i < IBUS_CHANNELS; i++) {
        data.channels[i] = 1500;  // Center position
    }
    data.failsafe = true;
    data.new_data = false;
    data.timestamp = 0;
    data.checksum = 0;
}

bool IBUS::begin() {
    uart_config_t uart_config = {
        .baud_rate = IBUS_BAUD_RATE,
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
    
    err = uart_set_pin(uart_num, IBUS_TX_PIN, IBUS_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        return false;
    }
    
    err = uart_driver_install(uart_num, 1024, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        return false;
    }
    
    return true;
}

uint16_t IBUS::calculateChecksum(const uint8_t *packet) {
    uint16_t checksum = 0;
    for (int i = 0; i < IBUS_PACKET_SIZE - 2; i++) {
        checksum += packet[i];
    }
    return checksum;
}

bool IBUS::parsePacket() {
    // Check header
    if (buffer[0] != IBUS_HEADER_0 || buffer[1] != IBUS_HEADER_1) {
        return false;
    }
    
    // Calculate and verify checksum
    uint16_t calculated_checksum = calculateChecksum(buffer);
    uint16_t received_checksum = (buffer[IBUS_PACKET_SIZE - 2] << 8) | buffer[IBUS_PACKET_SIZE - 1];
    
    if (calculated_checksum != received_checksum) {
        return false;
    }
    
    // Extract channel data (little-endian, 2 bytes per channel)
    int channel_index = 0;
    for (int i = 2; i < IBUS_PACKET_SIZE - 2; i += 2) {
        if (channel_index < IBUS_CHANNELS) {
            data.channels[channel_index] = (buffer[i + 1] << 8) | buffer[i];
            channel_index++;
        }
    }
    
    // Check for failsafe (all channels at 0 or max)
    bool all_zero = true;
    bool all_max = true;
    for (int i = 0; i < IBUS_CHANNELS; i++) {
        if (data.channels[i] != 0) all_zero = false;
        if (data.channels[i] != 0xFFFF) all_max = false;
    }
    data.failsafe = all_zero || all_max;
    
    data.new_data = true;
    data.timestamp = micros();
    data.checksum = received_checksum;
    
    return true;
}

void IBUS::update() {
    data.new_data = false;
    
    // Read available bytes from UART
    size_t bytes_available;
    uart_get_buffered_data_len(uart_num, &bytes_available);
    
    if (bytes_available == 0) {
        return;
    }
    
    uint8_t temp_buffer[128];
    int len = uart_read_bytes(uart_num, temp_buffer, bytes_available, 0);
    
    if (len <= 0) {
        return;
    }
    
    // Process received bytes
    for (int i = 0; i < len; i++) {
        uint8_t byte = temp_buffer[i];
        
        // Look for packet start
        if (buffer_index == 0 && byte != IBUS_HEADER_0) {
            continue;
        }
        
        if (buffer_index == 1 && byte != IBUS_HEADER_1) {
            buffer_index = 0;
            continue;
        }
        
        buffer[buffer_index] = byte;
        buffer_index++;
        
        // Check if we have a complete packet
        if (buffer_index >= IBUS_PACKET_SIZE) {
            if (parsePacket()) {
                // Valid packet received
            }
            buffer_index = 0;
        }
    }
}

uint16_t IBUS::getChannel(uint8_t channel) {
    if (channel >= IBUS_CHANNELS) {
        return 1500;  // Return center if invalid channel
    }
    return data.channels[channel];
}
