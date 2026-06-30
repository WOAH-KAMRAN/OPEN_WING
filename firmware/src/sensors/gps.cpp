#include "gps.h"
#include "../config/config.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// NMEA sentence prefixes
#define NMEA_GPRMC "$GPRMC"
#define NMEA_GPGGA "$GPGGA"

GPS::GPS(uart_port_t uart_num_param) : uart_num(uart_num_param),
                                         buffer_index(0) {
    // Initialize data structure
    data.latitude = 0.0;
    data.longitude = 0.0;
    data.altitude = 0.0f;
    data.ground_speed = 0.0f;
    data.course = 0.0f;
    data.fix_quality = 0;
    data.satellites = 0;
    data.timestamp = 0;
    data.new_data = false;
}

bool GPS::begin() {
    uart_config_t uart_config = {
        .baud_rate = 9600,  // Standard GPS baud rate
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
    
    // Use GPS pins (default: TX=33, RX=32)
    err = uart_set_pin(uart_num, 33, 32, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        return false;
    }
    
    err = uart_driver_install(uart_num, 1024, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        return false;
    }
    
    return true;
}

bool GPS::validateChecksum(const char *sentence) {
    // Find checksum delimiter
    const char *asterisk = strchr(sentence, '*');
    if (asterisk == NULL) {
        return false;
    }
    
    // Calculate checksum of sentence (excluding $ and *)
    uint8_t calculated_checksum = 0;
    for (const char *p = sentence + 1; p < asterisk; p++) {
        calculated_checksum ^= *p;
    }
    
    // Parse received checksum
    char checksum_str[3];
    checksum_str[0] = asterisk[1];
    checksum_str[1] = asterisk[2];
    checksum_str[2] = '\0';
    
    uint8_t received_checksum = (uint8_t)strtol(checksum_str, NULL, 16);
    
    return calculated_checksum == received_checksum;
}

double GPS::parseDegrees(const char *str) {
    // NMEA format: DDMM.MMMM or DDDMM.MMMM
    // Convert to decimal degrees: DD + MM.MMMM/60
    
    double value = atof(str);
    double degrees = floor(value / 100.0);
    double minutes = value - (degrees * 100.0);
    
    return degrees + (minutes / 60.0);
}

bool GPS::parseGPRMC(const char *sentence) {
    // GPRMC format: $GPRMC,HHMMSS,A,DDMM.MMMM,N,DDDMM.MMMM,E,SSS.S,TTT.T,DDMMYY,DDD.D,E*CS
    // Example: $GPRMC,123519,A,4851.5151,N,00215.3552,E,022.4,084.4,230394,003.1,W*6A
    
    char *token;
    char sentence_copy[128];
    strncpy(sentence_copy, sentence, sizeof(sentence_copy));
    sentence_copy[sizeof(sentence_copy) - 1] = '\0';
    
    // Skip prefix
    token = strtok(sentence_copy, ",");
    if (token == NULL || strcmp(token, NMEA_GPRMC) != 0) {
        return false;
    }
    
    // Time (HHMMSS) - skip
    token = strtok(NULL, ",");
    
    // Status (A=active, V=void)
    token = strtok(NULL, ",");
    if (token == NULL || (token[0] != 'A' && token[0] != 'V')) {
        return false;
    }
    
    // If status is V (void), no valid data
    if (token[0] == 'V') {
        return false;
    }
    
    // Latitude (DDMM.MMMM)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    double lat = parseDegrees(token);
    
    // Latitude direction (N/S)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    if (token[0] == 'S') lat = -lat;
    
    // Longitude (DDDMM.MMMM)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    double lon = parseDegrees(token);
    
    // Longitude direction (E/W)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    if (token[0] == 'W') lon = -lon;
    
    // Ground speed (knots)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    float speed_knots = atof(token);
    data.ground_speed = speed_knots * 0.514444f;  // Convert to m/s
    
    // Course (degrees true)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    data.course = atof(token);
    
    // Date (DDMMYY) - skip
    token = strtok(NULL, ",");
    
    // Magnetic variation - skip
    token = strtok(NULL, ",");
    token = strtok(NULL, ",");
    
    // Update position data
    data.latitude = lat;
    data.longitude = lon;
    
    return true;
}

bool GPS::parseGPGGA(const char *sentence) {
    // GPGGA format: $GPGGA,HHMMSS,DDMM.MMMM,N,DDDMM.MMMM,E,1,08,0.9,545.4,M,46.9,M,,*47
    // Example: $GPGGA,123519,4851.5151,N,00215.3552,E,1,08,0.9,545.4,M,46.9,M,,*47
    
    char *token;
    char sentence_copy[128];
    strncpy(sentence_copy, sentence, sizeof(sentence_copy));
    sentence_copy[sizeof(sentence_copy) - 1] = '\0';
    
    // Skip prefix
    token = strtok(sentence_copy, ",");
    if (token == NULL || strcmp(token, NMEA_GPGGA) != 0) {
        return false;
    }
    
    // Time (HHMMSS) - skip
    token = strtok(NULL, ",");
    
    // Latitude (DDMM.MMMM)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    double lat = parseDegrees(token);
    
    // Latitude direction (N/S)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    if (token[0] == 'S') lat = -lat;
    
    // Longitude (DDDMM.MMMM)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    double lon = parseDegrees(token);
    
    // Longitude direction (E/W)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    if (token[0] == 'W') lon = -lon;
    
    // Fix quality (0=no fix, 1=2D, 2=3D)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    data.fix_quality = atoi(token);
    
    // Number of satellites
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    data.satellites = atoi(token);
    
    // HDOP - skip
    token = strtok(NULL, ",");
    
    // Altitude (meters)
    token = strtok(NULL, ",");
    if (token == NULL) return false;
    data.altitude = atof(token);
    
    // Altitude units (M) - skip
    token = strtok(NULL, ",");
    
    // Geoid height - skip
    token = strtok(NULL, ",");
    token = strtok(NULL, ",");
    
    // Update position data
    data.latitude = lat;
    data.longitude = lon;
    
    return true;
}

bool GPS::parseNMEA(const char *sentence) {
    // Validate checksum
    if (!validateChecksum(sentence)) {
        return false;
    }
    
    // Dispatch to specific parser based on sentence type
    if (strncmp(sentence, NMEA_GPRMC, 6) == 0) {
        return parseGPRMC(sentence);
    } else if (strncmp(sentence, NMEA_GPGGA, 6) == 0) {
        return parseGPGGA(sentence);
    }
    
    return false;
}

void GPS::update() {
    data.new_data = false;
    
    // Read available bytes from UART
    size_t bytes_available;
    uart_get_buffered_data_len(uart_num, &bytes_available);
    
    if (bytes_available == 0) {
        return;
    }
    
    uint8_t temp_buffer[256];
    int len = uart_read_bytes(uart_num, temp_buffer, bytes_available, 0);
    
    if (len <= 0) {
        return;
    }
    
    // Process received bytes
    for (int i = 0; i < len; i++) {
        uint8_t byte = temp_buffer[i];
        
        // Look for sentence start
        if (buffer_index == 0 && byte != '$') {
            continue;
        }
        
        buffer[buffer_index] = byte;
        buffer_index++;
        
        // Check for end of sentence (newline)
        if (byte == '\n' || byte == '\r') {
            buffer[buffer_index - 1] = '\0';  // Null-terminate
            
            // Parse the sentence
            if (parseNMEA((char *)buffer)) {
                data.new_data = true;
                data.timestamp = micros();
            }
            
            buffer_index = 0;
        }
        
        // Prevent buffer overflow
        if (buffer_index >= sizeof(buffer) - 1) {
            buffer_index = 0;
        }
    }
}
