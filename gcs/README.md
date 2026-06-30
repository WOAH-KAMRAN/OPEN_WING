# OpenWing Ground Control Station

Lightweight, modular telemetry protocol and PWA configurator for ESP32-based flight controller.

## Project Structure

```
/gcs/
├── protocol/           # Telemetry protocol (C/C++)
│   ├── protocol.h
│   ├── protocol.c
│   └── protocol_spec.md
└── gcs/               # PWA Ground Control Station
    ├── communication/ # Transport layer
    │   ├── serial.js
    │   └── websocket.js
    ├── telems/        # Telemetry layer
    │   ├── protocol_parser.js
    │   ├── message_definitions.js
    │   └── state_manager.js
    ├── app/           # UI components
    │   ├── dashboard/
    │   ├── pid_panel/
    │   ├── compass/
    │   └── map/
    ├── widgets/       # Reusable widgets
    │   ├── horizon.js
    │   └── compass.js
    ├── app.js
    ├── index.html
    ├── styles.css
    └── manifest.json
```

## Data Flow

```
RAW BYTES (USB/WiFi)
→ communication/
→ telems/protocol_parser
→ telems/state_manager
→ app/ (React UI)
```

## Protocol

Binary protocol with CRC16 validation:
- Header: 0xAA55
- Version: 1 byte
- Message ID: 1 byte
- Length: 1 byte
- Payload: N bytes
- CRC16: 2 bytes

See `/protocol/protocol_spec.md` for full specification.

## Installation

```bash
npm install
```

## Development

```bash
npm run dev
```

## Build

```bash
npm run build
```

## Features

- USB Serial (WebSerial)
- WiFi (WebSocket)
- Live dashboard with artificial horizon
- PID tuning panel
- Compass view
- GPS map with path trail
- Real-time telemetry

## Usage

1. Connect flight controller via USB or WiFi
2. Click "Connect" button
3. View telemetry in dashboard
4. Adjust PID values in PID panel
5. Track position on map

## Firmware Integration

Include `/protocol/protocol.h` and `/protocol/protocol.c` in your ESP32 firmware.
