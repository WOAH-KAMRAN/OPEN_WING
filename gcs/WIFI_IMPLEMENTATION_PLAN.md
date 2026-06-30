# WiFi Implementation Plan for ESP32 Firmware

## Overview
Plan to add WiFi support to the ESP32 firmware for wireless telemetry using the new OpenWing protocol.

## Current State
- Firmware uses custom protocol over UART (USB) at 115200 baud
- Protocol: Header (0xAA55) + attitude (floats) + RC channels + mode + simple checksum
- GCS now supports both protocols: firmware protocol (USB) and new protocol (WiFi)

## WiFi Implementation Strategy

### Phase 1: WiFi Infrastructure
1. **Add WiFi AP Mode**
   - ESP32 creates Access Point (e.g., "OpenWing_FC")
   - Default IP: 192.168.4.1
   - No password for development (add later for security)

2. **WebSocket Server**
   - Use ESPAsyncWebServer or arduinoWebSockets library
   - Listen on port 8080
   - Support single client connection
   - Handle connection/disconnection events

### Phase 2: Protocol Switching
1. **Dual Protocol Support**
   - Keep existing UART telemetry (firmware protocol)
   - Add WebSocket telemetry (new protocol)
   - Use compile-time flag or runtime config to select

2. **Protocol Selection**
   ```cpp
   #define USE_WIFI_TELEMETRY 0  // Default: USB
   // or runtime via config
   ```

### Phase 3: New Protocol Implementation
1. **Port Protocol Definitions**
   - Copy `/protocol/protocol.h` to firmware
   - Implement encode/decode functions
   - Add CRC16-CCITT calculation

2. **Message Encoding**
   - ATTITUDE (0x01): Send roll/pitch/yaw (radians)
   - RC_CHANNELS (0x04): Send all 8 channels
   - FLIGHT_MODE (0x05): Send mode and armed status
   - STATUS (0x07): Send CPU load, loop time, errors

3. **Command Handling**
   - SET_PID (0x81): Update PID parameters
   - GET_PID (0x82): Send current PID values
   - SET_MODE (0x83): Change flight mode
   - ARM_DISARM (0x84): Arm/disarm motors

### Phase 4: Integration
1. **Telemetry Task Modification**
   - Check if WiFi client connected
   - If connected, send new protocol via WebSocket
   - Always send old protocol via UART (for USB fallback)

2. **Command Reception**
   - WebSocket message callback
   - Parse incoming packets
   - Execute commands (PID, mode, arm)
   - Send responses if needed

## Firmware Changes Required

### New Files
```
src/
├── wifi/
│   ├── wifi_manager.h
│   └── wifi_manager.cpp
└── protocol/
    ├── openwing_protocol.h    (from /protocol/protocol.h)
    └── openwing_protocol.cpp  (from /protocol/protocol.c)
```

### Modified Files
1. `src/main.cpp`
   - Initialize WiFi manager
   - Start WebSocket server
   - Pass connection status to telemetry

2. `src/telemetry/telemetry.cpp`
   - Add WebSocket send method
   - Check connection before sending
   - Encode new protocol packets

3. `src/control/flight_control.cpp`
   - Add methods to update PID from commands
   - Add arm/disarm logic

### platformio.ini Dependencies
```ini
lib_deps =
    Wire
    ESPAsyncWebServer
    arduinoWebSockets
```

## Protocol Mapping

### Firmware Protocol (USB) → New Protocol (WiFi)

| Firmware Field | New Protocol Message |
|---------------|---------------------|
| Roll/Pitch/Yaw | ATTITUDE (0x01) |
| RC Channels | RC_CHANNELS (0x04) |
| Mode | FLIGHT_MODE (0x05) |
| (none) | STATUS (0x07) - new |
| (none) | BATTERY (0x08) - requires ADC |
| (none) | GPS (0x03) - requires GPS module |

## Command Flow

### GCS → ESP32 (WiFi)
1. GCS encodes command using new protocol
2. Sends via WebSocket
3. ESP32 receives and validates CRC
4. Executes command (e.g., SET_PID)
5. Updates internal state
6. Optional: sends response

### ESP32 → GCS (WiFi)
1. Telemetry task collects data
2. Encodes using new protocol
3. Sends via WebSocket if connected
4. GCS parses and updates UI

## Testing Plan

1. **WiFi Connection**
   - Verify ESP32 creates AP
   - GCS connects to WebSocket
   - Test reconnection on disconnect

2. **Protocol Validation**
   - Send ATTITUDE messages
   - Verify GCS parses correctly
   - Test CRC validation

3. **Commands**
   - Send SET_PID command
   - Verify firmware updates PID
   - Send GET_PID command
   - Verify response

4. **Dual Mode**
   - Test USB and WiFi simultaneously
   - Verify both protocols work independently

## Security Considerations (Future)
- Add WPA2 password to AP
- Implement authentication for WebSocket
- Add encryption layer if needed
- Rate limiting for commands

## Performance Impact
- WiFi adds ~10-20ms latency vs USB
- WebSocket overhead minimal
- CPU load increase: ~5-10%
- Memory usage: +20-30KB for WiFi stack

## Rollout Plan
1. Implement WiFi infrastructure (no protocol change)
2. Add new protocol encoding
3. Test with GCS
4. Add command handling
5. Full integration testing
6. Documentation update
