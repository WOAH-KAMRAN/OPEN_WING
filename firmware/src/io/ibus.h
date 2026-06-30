#ifndef IBUS_H
#define IBUS_H

#include <Arduino.h>
#include "driver/uart.h"

#define IBUS_CHANNELS  14
#define IBUS_PACKET_SIZE 32

struct IBUSData {
    uint16_t channels[IBUS_CHANNELS];
    bool failsafe;
    bool new_data;
    uint32_t timestamp;
    uint16_t checksum;
};

class IBUS {
private:
    uart_port_t uart_num;
    uint8_t buffer[IBUS_PACKET_SIZE];
    uint8_t buffer_index;
    IBUSData data;
    
public:
    IBUS(uart_port_t uart_num = UART_NUM_1);
    
    bool begin();
    void update();
    IBUSData getData() { return data; }
    uint16_t getChannel(uint8_t channel);
    bool isFailsafe() { return data.failsafe; }
    bool hasNewData() { return data.new_data; }
    
private:
    bool parsePacket();
    uint16_t calculateChecksum(const uint8_t *packet);
};

#endif
