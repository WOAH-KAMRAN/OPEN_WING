# Fixed-Wing Flight Controller Firmware
<p align="center">
  <img src="assets/banner.png" alt="Banner" width="100%">
</p>
A modular, real-time flight controller firmware for ESP32 designed for fixed-wing aircraft.

## Hardware Target

- **MCU**: ESP32 (dual core, FreeRTOS)
- **IMU**: MPU6050 (I2C)
- **Magnetometer**: QMC5883L or HMC5883L (I2C)
- **RC Input**: IBUS (UART)
- **Servo Output**: PWM via ESP32 LEDC
- **Telemetry**: UART (custom protocol)

## Architecture

The firmware follows a modular architecture with clear separation of concerns:

```
src/
├── config/          # Configuration and pin definitions
├── sensors/         # Sensor drivers (MPU6050, Magnetometer)
├── ahrs/            # Attitude estimation (Madgwick filter)
├── control/         # Flight control (PID, mode logic)
├── navigation/      # Navigation functions
├── storage/         # Flash-backed persistent configuration
├── io/              # I/O systems (IBUS, PWM, WebServer)
└── telemetry/       # Telemetry protocol
```

## FreeRTOS Task Architecture

- **High-frequency task (400 Hz)**: IMU + AHRS + control loop (Core 0)
- **Medium-frequency task (50 Hz)**: Navigation + RC processing (Core 1)
- **Low-frequency task (10 Hz)**: Telemetry (Core 1)

## Flight Modes

- **MANUAL**: Direct RC passthrough to servos
- **FBWA** (Fly By Wire A): Stabilized flight with attitude control
- **STABILIZE**: Attitude stabilization (same as FBWA in current implementation)
- **RTL**: Return to Launch (stub)
- **AUTO**: Autonomous waypoint following (stub)
- **LOITER**: Hold position (stub)

## Control System

### Dual-Loop Control

- **Outer loop**: Angle PID (roll/pitch angle control)
- **Inner loop**: Rate PID (roll/pitch rate control)

### PID Tuning

Default PID gains are defined in `flight_control.cpp`. These should be tuned for your specific airframe:

```cpp
// Rate PID gains
ROLL_RATE_KP  = 0.15
ROLL_RATE_KI  = 0.0
ROLL_RATE_KD  = 0.005

// Angle PID gains
ROLL_ANGLE_KP = 4.5
ROLL_ANGLE_KI = 0.0
ROLL_ANGLE_KD = 0.0
```

### Dynamic PID via Serial CLI

PID gains can be adjusted at runtime through the Serial CLI (no flight mode restriction):

```
SET <AXIS> <KP> <KI> <KD>    Set PID gains for axis
  Axes: ROLL_RATE, PITCH_RATE, ROLL_ANGLE, PITCH_ANGLE
  (Omit axis name to set ALL four axes)
GET                           Print current PID gains
SAVE                          Save gains to flash (persistent across reboots)
LOAD                          Load gains from flash
RESET                         Reset to firmware defaults
HELP                          Show this help
```

### Flash Storage

PID gains, gyro bias, magnetometer calibration, and home position are stored in ESP32 non-volatile memory via the `Preferences` library:

- **`config/pins.h`** — Hardware pin definitions
- **`config/config.h`** — System configuration and `FlightMode` enum
- **`storage/flash_manager.h`** — `PersistentConfig` struct and `FlashManager` class
- **`control/flight_control.cpp`** — `loadFromFlash()` / `saveToFlash()` methods

## Pin Configuration

Default pin assignments (configurable in `config/config.h`):

- **I2C SDA**: GPIO 21
- **I2C SCL**: GPIO 22
- **IBUS TX**: GPIO 17
- **IBUS RX**: GPIO 16
- **Telemetry TX**: GPIO 25
- **Telemetry RX**: GPIO 26
- **Servo Aileron**: GPIO 4
- **Servo Elevator**: GPIO 12
- **Servo Throttle**: GPIO 15
- **Built-in LED**: GPIO 2 (status indication)

## Building and Flashing

### Requirements

- PlatformIO
- ESP32 development board
- Arduino framework

### Build

```bash
pio run
```

### Upload

```bash
pio run --target upload
```

### Monitor Serial Output

```bash
pio device monitor
```

## Calibration

### Gyro Calibration

Gyro bias calibration is performed automatically at startup. Keep the sensor still during initialization. Bias values are stored in flash after calibration but recalibrated every boot (bias drifts with temperature).

### Magnetometer Calibration

To calibrate the magnetometer:

1. Uncomment the calibration code in `main.cpp`
2. Flash the firmware
3. Move the sensor in a figure-8 pattern
4. The calibration values will be printed to serial
5. Store the values via the Serial CLI `SAVE` command

## Telemetry Protocol

Custom binary protocol over UART at 115200 baud (v2, 64-byte fixed packet):

```
Offset  Size  Field
─────────────────────────────────
 0      2     Header (0xAA 0x55)
 2      1     Version (0x02)
 3      1     Heartbeat (incremented each packet)
 4      4     Roll (float, degrees)
 8      4     Pitch (float, degrees)
12      4     Yaw (float, degrees)
16     12     6x RC channels (uint16_t each)
28      1     Flight mode (uint8_t)
29      1     Flags (bitmask)
30      2     Checksum (16-bit sum, excludes reserved + checksum)
32     32     Reserved (future use: GPS, battery, nav)
─────────────────────────────────
Total:  64 bytes
```

### Flags byte

| Bit | Mask | Meaning       |
|-----|------|---------------|
| 0   | 0x01 | Mag healthy   |
| 1   | 0x02 | Home set      |
| 2   | 0x04 | GPS fix       |
| 3-7| —    | Reserved      |

## RC Channel Mapping

- **CH0**: Aileron
- **CH1**: Elevator
- **CH2**: Throttle
- **CH3**: Rudder (not used in current implementation)
- **CH4**: Aux
- **CH5**: Mode selection

### Mode Selection

- **< 1300**: MANUAL
- **1300-1700**: FBWA
- **> 1700**: STABILIZE

## Safety Features

- **Failsafe detection**: Detects loss of RC signal
- **Output limiting**: Prevents servo saturation
- **Anti-windup**: Integral term management in PID controllers
- **Mutex protection**: Thread-safe data sharing between tasks
- **Magnetometer failure fallback**: After 3 consecutive read failures, AHRS falls back to IMU-only mode (`updateIMU()`); background retry attempts re-init every ~1s

## Onboard Ground Control Station (Web UI)

The firmware includes a lightweight web server that runs directly on the ESP32, providing a real-time flight dashboard accessible from any phone or laptop browser — no app installation needed.

### Connecting

1. Power on the flight controller
2. Connect your phone or laptop to the **OpenWing_FC** WiFi network (no password)
3. Open a browser and navigate to **http://192.168.4.1**
4. The dashboard displays:
   - **Attitude**: Roll, Pitch, Yaw in real-time (updated every 300ms)
   - **Flight Mode**: Current operating mode
   - **PID Values**: Gains for all four controllers (Roll Rate, Pitch Rate, Roll Angle, Pitch Angle)

The web server is always active at boot and runs on the background idle task, with minimal impact on flight performance.

### Technical Details

- The web server reads flight data directly from shared memory (mutex-protected)
- Serves a minimal (~3KB) HTML page with dark theme, no external dependencies
- Data endpoint: `GET /data` returns JSON with attitude, PID values, and flight mode
- The ESP32 operates as a WiFi Access Point — no router needed

## LED Status Indicators

The built-in LED (GPIO 2, shared with `SERVO_ELEVATOR_PIN` in earlier revisions — now moved to GPIO 12) provides visual feedback during initialization and operation:

### Initialization Sequence

- **3 blinks (100ms)**: Firmware initialization started
- **5 blinks (200ms)**: MPU6050 initialization failed (error state)
- **3 blinks (200ms)**: IBUS RC initialization failed (error state)
- **5 blinks (50ms)**: Flight controller ready for operation

### Error States

- **5 blinks (200ms) repeating**: MPU6050 sensor not detected
- **3 blinks (200ms) repeating**: IBUS RC receiver not detected

### Normal Operation

- **LED off**: Normal operation (no continuous indication)
- **Future**: Consider adding heartbeat blink or mode-specific patterns

## Key Design Decisions

1. **Modular Architecture**: Clear separation of concerns for maintainability
2. **Deterministic Timing**: Fixed-frequency loops with `vTaskDelayUntil`
3. **Core Assignment**: High-frequency tasks on core 0, others on core 1
4. **Quaternion-based AHRS**: Madgwick filter for robust attitude estimation
5. **Dual-loop Control**: Inner rate loop for stability, outer angle loop for attitude

## Extending the Firmware

### Adding New Flight Modes

1. Add mode to `FlightMode` enum in `config.h`
2. Implement mode logic in `flight_control.cpp`
3. Add mode selection logic in `main.cpp`

### Adding New Sensors

1. Create driver in `sensors/` directory
2. Integrate sensor reading in high-frequency task
3. Update AHRS filter if needed

### Adding New Telemetry Fields

1. Update `TelemetryPacket` struct in `telemetry.h`
2. Update packet construction in `telemetry.cpp`
3. Update checksum calculation

## Troubleshooting

### MPU6050 Not Detected

- Check I2C wiring (SDA/SCL)
- Verify I2C address (0x68 or 0x69)
- Check pull-up resistors on I2C bus

### Magnetometer Not Detected

- Try both QMC5883L (0x0D) and HMC5883L (0x1E) addresses
- Check I2C wiring
- Verify sensor is powered

### Servos Not Responding

- Check PWM pin assignments
- Verify servo power supply
- Check servo signal connections

### RC Signal Lost

- Verify IBUS receiver connection
- Check baud rate (115200)
- Verify RC transmitter is bound

## License

This firmware is provided as-is for educational and hobby use. Use at your own risk.

## Contributing

Contributions are welcome! Please ensure code follows the existing style and includes appropriate documentation.
