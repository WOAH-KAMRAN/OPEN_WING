#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include "driver/uart.h"

struct GPSData {
    double latitude;      // Degrees, positive for North
    double longitude;     // Degrees, positive for East
    float altitude;       // Meters above MSL
    float ground_speed;   // m/s
    float course;         // Degrees true
    uint8_t fix_quality;  // 0=no fix, 1=2D, 2=3D
    uint8_t satellites;   // Number of satellites in use
    uint32_t timestamp;   // Microseconds
    bool new_data;
};

class GPS {
private:
    uart_port_t uart_num;
    uint8_t buffer[256];
    uint16_t buffer_index;
    GPSData data;
    
    bool parseNMEA(const char *sentence);
    bool parseGPRMC(const char *sentence);
    bool parseGPGGA(const char *sentence);
    bool validateChecksum(const char *sentence);
    double parseDegrees(const char *str);
    
public:
    GPS(uart_port_t uart_num = UART_NUM_2);
    
    bool begin();
    void update();
    GPSData getData() { return data; }
    bool hasNewData() { return data.new_data; }
    bool has3DFix() { return data.fix_quality >= 2; }
};

#endif
