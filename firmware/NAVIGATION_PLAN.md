# Navigation Implementation Plan

This document outlines the complete plan for implementing autonomous navigation features including RTL (Return To Launch), mission planning, and waypoint following.

## Current Status

**Partially Implemented**: Basic waypoint data structures exist in `src/navigation/navigation.h/.cpp`
**Missing**: GPS driver, guidance algorithms, autonomous flight logic

## Implementation Phases

### Phase 1: GPS Integration (Foundation)

#### 1.1 GPS Driver (`src/sensors/gps.h/.cpp`)
- NMEA parser for UBlox/NEO-M8N modules
- Parse GPRMC/GPGGA sentences for position, velocity, time
- 5Hz update rate via UART
- GPS fix quality detection (3D fix required for navigation)
- Data structure:
  ```cpp
  struct GPSData {
      double latitude;      // Degrees
      double longitude;     // Degrees
      float altitude;       // Meters
      float ground_speed;  // m/s
      float course;         // Degrees
      uint8_t fix_quality;  // 0=no fix, 1=2D, 2=3D
      uint8_t satellites;   // Number of satellites
      uint32_t timestamp;
      bool new_data;
  };
  ```

#### 1.2 Geodetic Utilities (`src/navigation/geodetic.h/.cpp`)
- Lat/lon to local NED (North-East-Down) coordinate conversion
- Distance/bearing calculations between waypoints
- Haversine formula for great-circle distance
- Coordinate frame transformations (body to navigation)
- Functions:
  ```cpp
  float distanceBetween(double lat1, double lon1, double lat2, double lon2);
  float bearingTo(double lat1, double lon1, double lat2, double lon2);
  void latLonToNED(double lat, double lon, float alt, double home_lat, double home_lon, float home_alt, float &north, float &east, float &down);
  ```

### Phase 2: Guidance System

#### 2.1 L1 Guidance Algorithm (`src/navigation/l1_guidance.h/.cpp`)
- Pure pursuit path following
- Cross-track error calculation
- Lateral acceleration command generation
- Tunable L1 distance parameter for aggressiveness
- Input: Current position, target waypoint, current heading
- Output: Desired course (bearing to follow)

#### 2.2 Navigation Controller (`src/navigation/nav_controller.h/.cpp`)
- Cross-track PID controller (lateral guidance)
- Speed PID controller (throttle management)
- Altitude PID controller (vertical guidance)
- Output: Desired roll angle and throttle
- Integration with existing flight control system

### Phase 3: Flight Modes

#### 3.1 Home Position System
- Capture GPS position at startup when GPS has 3D fix
- Store in non-volatile memory (ESP32 preferences)
- Manual home reset via RC channel
- Home position validation (reasonable lat/lon range)

#### 3.2 RTL Mode Implementation
- Calculate bearing to home
- Climb to RTL altitude (configurable, e.g., 50m)
- Navigate to home using L1 guidance
- Loiter over home at configurable radius/altitude
- Auto-land or hold based on configuration
- RTL state machine:
  - RTL_CLIMB: Climb to RTL altitude
  - RTL_RETURN: Navigate to home
  - RTL_LOITER: Circle over home
  - RTL_LAND: Descent and land (optional)

#### 3.3 AUTO Mode Implementation
- Mission waypoint list management (load from storage)
- Sequential waypoint execution
- Waypoint acceptance radius (configurable)
- Mission completion handling (RTL or loiter)
- Mission file format (JSON or binary)
- AUTO state machine:
  - AUTO_TAKEOFF: Climb to mission altitude
  - AUTO_NAVIGATE: Follow waypoints
  - AUTO_LOITER: Wait at waypoint (if configured)
  - AUTO_COMPLETE: Mission done

#### 3.4 LOITER Mode Implementation
- Circle around current position or waypoint
- Configurable radius and altitude
- Coordinated turns using bank angle
- Loiter direction (clockwise/counter-clockwise)

### Phase 4: Integration

#### 4.1 Mode Enum Extension (`src/config/config.h`)
```cpp
enum FlightMode {
    MANUAL,
    FBWA,
    STABILIZE,
    RTL,
    AUTO,
    LOITER
};
```

#### 4.2 Mode Switching Logic (`src/main.cpp`)
- Extend RC Channel 5 mapping:
  - < 1100: MANUAL
  - 1100-1300: FBWA
  - 1300-1500: STABILIZE
  - 1500-1700: RTL
  - 1700-1900: AUTO
  - > 1900: LOITER
- Add mode validation (GPS required for RTL/AUTO/LOITER)

#### 4.3 Navigation Task Integration
- Add GPS reading to medium-frequency task (50Hz)
- Navigation controller update at 50Hz
- Waypoint management and mode logic
- Integration with existing attitude control loop

#### 4.4 Telemetry Extension
- Add GPS position (lat, lon, alt)
- Add ground speed and heading
- Add current waypoint index
- Add cross-track error
- Add navigation mode state

### Phase 5: Safety

#### 5.1 GPS Loss Failsafe
- Detect GPS signal loss (> 5 seconds)
- If in AUTO/RTL/LOITER: switch to FBWA or land
- Alert via telemetry
- Configurable behavior

#### 5.2 RC Loss Failsafe
- Detect RC signal loss
- If GPS available: engage RTL
- If no GPS: circle/land in FBWA
- Configurable failsafe behavior
- Failsafe timer before action

#### 5.3 Geofence (Optional)
- Define maximum distance from home
- Return to home if exceeded
- Altitude limits

## Implementation Order (Priority)

1. **GPS driver** - Foundation for everything
2. **Geodetic utilities** - Coordinate conversions
3. **Home position capture** - Simple, enables RTL
4. **L1 guidance** - Core navigation algorithm
5. **Navigation controller** - PID integration
6. **RTL mode** - First autonomous mode
7. **AUTO mode** - Mission execution
8. **LOITER mode** - Position holding
9. **Telemetry update** - Debugging/monitoring
10. **Failsafes** - Safety features

## Key Design Decisions

- **L1 Guidance**: Industry-standard, proven robustness
- **50Hz navigation**: Matches medium-frequency task, sufficient for fixed-wing
- **Separate navigation controller**: Clean separation from attitude control
- **NED coordinate frame**: Standard in aerospace, simplifies math
- **3D GPS fix requirement**: Prevents navigation with poor accuracy
- **RTL before AUTO**: RTL is simpler, validates navigation stack
- **ESP32 Preferences**: Non-volatile storage for home position and mission

## Configuration Parameters

Add to `src/config/config.h`:
```cpp
// Navigation configuration
#define RTL_ALTITUDE 50.0f          // Meters
#define RTL_LOITER_RADIUS 50.0f     // Meters
#define WAYPOINT_ACCEPT_RADIUS 30.0f // Meters
#define LOITER_RADIUS 50.0f         // Meters
#define GPS_LOSS_TIMEOUT 5000       // Milliseconds
#define RC_LOSS_TIMEOUT 1000        // Milliseconds
#define L1_DISTANCE 25.0f           // Meters (guidance parameter)
```

## Hardware Requirements

- GPS module (UBlox NEO-M8N or similar)
- UART connection (pins defined in config)
- External antenna for GPS
- Optional: Compass for better heading reference

## Testing Strategy

1. **GPS Testing**: Verify NMEA parsing, fix quality, accuracy
2. **Geodetic Testing**: Validate coordinate conversions
3. **Guidance Testing**: L1 algorithm in simulation
4. **RTL Testing**: Ground test, then low-altitude flight
5. **AUTO Testing**: Simple 2-3 waypoint mission
6. **Failsafe Testing**: Simulate GPS/RC loss

## Notes

- All navigation code should be thread-safe (use mutexes)
- GPS data should be validated before use (fix quality, satellite count)
- Navigation should be disabled if GPS is unavailable
- Mode transitions should be smooth (no sudden control changes)
- Telemetry should include navigation state for debugging
