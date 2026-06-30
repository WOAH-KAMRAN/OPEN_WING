#include "protocol.h"
#include <string.h>

uint16_t crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = CRC16_INIT;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

bool encode_packet(uint8_t *buffer, uint16_t *length, const packet_t *packet) {
    if (packet->length > PROTOCOL_MAX_PAYLOAD) {
        return false;
    }
    
    uint16_t idx = 0;
    
    buffer[idx++] = (PROTOCOL_HEADER >> 8) & 0xFF;
    buffer[idx++] = PROTOCOL_HEADER & 0xFF;
    buffer[idx++] = packet->version;
    buffer[idx++] = packet->msg_id;
    buffer[idx++] = packet->length;
    
    memcpy(&buffer[idx], packet->payload, packet->length);
    idx += packet->length;
    
    uint16_t crc = crc16(&buffer[2], idx - 2);
    buffer[idx++] = (crc >> 8) & 0xFF;
    buffer[idx++] = crc & 0xFF;
    
    *length = idx;
    return true;
}

bool decode_packet(const uint8_t *buffer, uint16_t length, packet_t *packet, uint16_t *consumed) {
    *consumed = 0;
    
    if (length < 7) {
        return false;
    }
    
    uint16_t header = (buffer[0] << 8) | buffer[1];
    if (header != PROTOCOL_HEADER) {
        *consumed = 1;
        return false;
    }
    
    uint8_t version = buffer[2];
    uint8_t msg_id = buffer[3];
    uint8_t payload_len = buffer[4];
    
    uint16_t total_len = 6 + payload_len + 2;
    if (length < total_len) {
        return false;
    }
    
    uint16_t crc_calc = crc16(&buffer[2], 3 + payload_len);
    uint16_t crc_recv = (buffer[6 + payload_len] << 8) | buffer[6 + payload_len + 1];
    
    if (crc_calc != crc_recv) {
        *consumed = 1;
        return false;
    }
    
    packet->version = version;
    packet->msg_id = msg_id;
    packet->length = payload_len;
    memcpy(packet->payload, &buffer[5], payload_len);
    
    *consumed = total_len;
    return true;
}
