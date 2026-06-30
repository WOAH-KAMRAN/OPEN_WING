# OpenWing Telemetry Protocol Specification

## Version
1.0.0

## Overview
Lightweight binary telemetry protocol for ESP32-based flight controller.
Optimized for embedded systems with minimal overhead.

## Packet Structure

```
[HEADER (2)][VERSION (1)][MSG_ID (1)][LENGTH (1)][PAYLOAD (N)][CRC16 (2)]
```

- **HEADER**: `0xAA55` (2 bytes, big-endian)
- **VERSION**: Protocol version (1 byte)
- **MSG_ID**: Message identifier (1 byte)
- **LENGTH**: Payload length in bytes (1 byte, max 255)
- **PAYLOAD**: Message-specific data (N bytes)
- **CRC16**: CRC-16-CCITT of [VERSION][MSG_ID][LENGTH][PAYLOAD] (2 bytes, big-endian)

## CRC Calculation
- Polynomial: 0x1021 (CRC-16-CCITT)
- Initial value: 0xFFFF
- Final XOR: 0x0000
- Input reflected: No
- Output reflected: No

## Message IDs

### Telemetry Messages (0x00 - 0x7F)

| ID  | Name          | Direction | Description |
|-----|---------------|-----------|-------------|
| 0x01| ATTITUDE      | FC→GCS    | Roll, pitch, yaw |
| 0x02| HEADING       | FC→GCS    | Magnetic heading |
| 0x03| GPS           | FC→GCS    | Position, altitude, speed |
| 0x04| RC_CHANNELS   | FC→GCS    | RC input values |
| 0x05| FLIGHT_MODE   | FC→GCS    | Current flight mode |
| 0x06| PID_VALUES    | FC→GCS    | Current PID parameters |
| 0x07| STATUS        | FC→GCS    | System status flags |
| 0x08| BATTERY       | FC→GCS    | Voltage, current |

### Command Messages (0x80 - 0xFF)

| ID  | Name          | Direction | Description |
|-----|---------------|-----------|-------------|
| 0x81| SET_PID       | GCS→FC    | Set PID parameters |
| 0x82| GET_PID       | GCS→FC    | Request PID parameters |
| 0x83| SET_MODE      | GCS→FC    | Set flight mode |
| 0x84| ARM_DISARM    | GCS→FC    | Arm/disarm motors |
| 0x85| REBOOT        | GCS→FC    | Reboot flight controller |

## Message Definitions

### ATTITUDE (0x01)
```
struct __attribute__((packed)) {
    float roll;    // radians, -PI to PI
    float pitch;   // radians, -PI/2 to PI/2
    float yaw;     // radians, 0 to 2PI
};
```
Length: 12 bytes

### HEADING (0x02)
```
struct __attribute__((packed)) {
    float heading; // degrees, 0 to 360
};
```
Length: 4 bytes

### GPS (0x03)
```
struct __attribute__((packed)) {
    int32_t latitude;  // degrees * 1e7
    int32_t longitude; // degrees * 1e7
    int32_t altitude;  // millimeters
    uint16_t speed;    // cm/s
    uint8_t satellites;
    uint8_t fix_type;  // 0=none, 1=2D, 2=3D, 3=DGPS
};
```
Length: 16 bytes

### RC_CHANNELS (0x04)
```
struct __attribute__((packed)) {
    uint16_t channels[8]; // microseconds, 1000-2000
};
```
Length: 16 bytes

### FLIGHT_MODE (0x05)
```
struct __attribute__((packed)) {
    uint8_t mode; // 0=MANUAL, 1=STABILIZE, 2=AUTO, 3=RTL
    uint8_t armed; // 0=disarmed, 1=armed
};
```
Length: 2 bytes

### PID_VALUES (0x06)
```
struct __attribute__((packed)) {
    float roll_p;
    float roll_i;
    float roll_d;
    float pitch_p;
    float pitch_i;
    float pitch_d;
    float yaw_p;
    float yaw_i;
    float yaw_d;
};
```
Length: 36 bytes

### STATUS (0x07)
```
struct __attribute__((packed)) {
    uint16_t errors;
    uint8_t cpu_load;
    uint8_t loop_time; // microseconds
};
```
Length: 4 bytes

### BATTERY (0x08)
```
struct __attribute__((packed)) {
    uint16_t voltage; // millivolts
    int16_t current;  // milliamps
    uint8_t percentage;
};
```
Length: 5 bytes

### SET_PID (0x81)
```
struct __attribute__((packed)) {
    uint8_t axis; // 0=roll, 1=pitch, 2=yaw
    float p;
    float i;
    float d;
};
```
Length: 13 bytes

### GET_PID (0x82)
```
struct __attribute__((packed)) {
    uint8_t axis; // 0=roll, 1=pitch, 2=yaw
};
```
Length: 1 byte

### SET_MODE (0x83)
```
struct __attribute__((packed)) {
    uint8_t mode;
};
```
Length: 1 byte

### ARM_DISARM (0x84)
```
struct __attribute__((packed)) {
    uint8_t arm; // 0=disarm, 1=arm
};
```
Length: 1 byte

### REBOOT (0x85)
No payload. Length: 0 bytes.

## Stream Safety

- Packets may be fragmented across stream boundaries
- Parser must handle partial packets
- Resynchronize on header detection
- Validate CRC before processing payload
- Discard invalid packets and continue scanning

## Example Packets

### ATTITUDE Message
Roll: 0.1 rad, Pitch: -0.05 rad, Yaw: 1.57 rad
```
AA55 01 01 0C [3D CC CC CC][BD CC CC CD][3F C9 0F DB] CRC
```

### SET_PID Command
Set roll axis: P=0.5, I=0.1, D=0.02
```
AA55 01 81 0D 00 [3F 00 00 00][3D CC CC CD][3C A3 D7 0A] CRC
```

## LoRa Constraints (Future)

- Bandwidth: ~1-10 kbps
- Latency: 100-1000ms
- Optimization strategy:
  - Reduce telemetry rate to 1-5 Hz
  - Use differential encoding for GPS
  - Compress attitude to 16-bit fixed-point
  - Prioritize critical messages (mode, arm status)
