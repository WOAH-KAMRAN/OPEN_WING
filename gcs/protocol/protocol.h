#ifndef OPENWING_PROTOCOL_H
#define OPENWING_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_VERSION 1
#define PROTOCOL_HEADER 0xAA55
#define PROTOCOL_MAX_PAYLOAD 255

#define CRC16_POLY 0x1021
#define CRC16_INIT 0xFFFF

typedef enum {
    MSG_ATTITUDE = 0x01,
    MSG_HEADING = 0x02,
    MSG_GPS = 0x03,
    MSG_RC_CHANNELS = 0x04,
    MSG_FLIGHT_MODE = 0x05,
    MSG_PID_VALUES = 0x06,
    MSG_STATUS = 0x07,
    MSG_BATTERY = 0x08,
    MSG_SET_PID = 0x81,
    MSG_GET_PID = 0x82,
    MSG_SET_MODE = 0x83,
    MSG_ARM_DISARM = 0x84,
    MSG_REBOOT = 0x85
} message_id_t;

typedef enum {
    FLIGHT_MODE_MANUAL = 0,
    FLIGHT_MODE_STABILIZE = 1,
    FLIGHT_MODE_AUTO = 2,
    FLIGHT_MODE_RTL = 3
} flight_mode_t;

typedef enum {
    GPS_FIX_NONE = 0,
    GPS_FIX_2D = 1,
    GPS_FIX_3D = 2,
    GPS_FIX_DGPS = 3
} gps_fix_type_t;

typedef struct __attribute__((packed)) {
    float roll;
    float pitch;
    float yaw;
} msg_attitude_t;

typedef struct __attribute__((packed)) {
    float heading;
} msg_heading_t;

typedef struct __attribute__((packed)) {
    int32_t latitude;
    int32_t longitude;
    int32_t altitude;
    uint16_t speed;
    uint8_t satellites;
    uint8_t fix_type;
} msg_gps_t;

typedef struct __attribute__((packed)) {
    uint16_t channels[8];
} msg_rc_channels_t;

typedef struct __attribute__((packed)) {
    uint8_t mode;
    uint8_t armed;
} msg_flight_mode_t;

typedef struct __attribute__((packed)) {
    float roll_p;
    float roll_i;
    float roll_d;
    float pitch_p;
    float pitch_i;
    float pitch_d;
    float yaw_p;
    float yaw_i;
    float yaw_d;
} msg_pid_values_t;

typedef struct __attribute__((packed)) {
    uint16_t errors;
    uint8_t cpu_load;
    uint8_t loop_time;
} msg_status_t;

typedef struct __attribute__((packed)) {
    uint16_t voltage;
    int16_t current;
    uint8_t percentage;
} msg_battery_t;

typedef struct __attribute__((packed)) {
    uint8_t axis;
    float p;
    float i;
    float d;
} msg_set_pid_t;

typedef struct __attribute__((packed)) {
    uint8_t axis;
} msg_get_pid_t;

typedef struct __attribute__((packed)) {
    uint8_t mode;
} msg_set_mode_t;

typedef struct __attribute__((packed)) {
    uint8_t arm;
} msg_arm_disarm_t;

typedef struct {
    uint8_t version;
    uint8_t msg_id;
    uint8_t length;
    uint8_t payload[PROTOCOL_MAX_PAYLOAD];
} packet_t;

uint16_t crc16(const uint8_t *data, uint16_t length);
bool encode_packet(uint8_t *buffer, uint16_t *length, const packet_t *packet);
bool decode_packet(const uint8_t *buffer, uint16_t length, packet_t *packet, uint16_t *consumed);

#ifdef __cplusplus
}
#endif

#endif
