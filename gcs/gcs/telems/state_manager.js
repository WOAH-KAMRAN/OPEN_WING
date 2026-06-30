class StateManager {
  constructor() {
    this.state = {
      attitude: { roll: 0, pitch: 0, yaw: 0 },
      heading: { heading: 0 },
      gps: { latitude: 0, longitude: 0, altitude: 0, speed: 0, satellites: 0, fixType: 0 },
      rcChannels: { channels: [1500, 1500, 1500, 1500, 1000, 1000, 1000, 1000] },
      flightMode: { mode: 0, armed: false },
      pidValues: {
        roll: { p: 0, i: 0, d: 0 },
        pitch: { p: 0, i: 0, d: 0 },
        yaw: { p: 0, i: 0, d: 0 }
      },
      status: { errors: 0, cpuLoad: 0, loopTime: 0 },
      battery: { voltage: 0, current: 0, percentage: 0 }
    };
    this.listeners = [];
    this.lastUpdate = 0;
  }

  update(packet) {
    if (packet.firmwareData) {
      this.updateFirmwareData(packet.firmwareData);
      return;
    }

    const { msgId, payload } = packet;

    switch (msgId) {
      case 0x01:
        this.state.attitude = this.parseAttitude(payload);
        break;
      case 0x02:
        this.state.heading = this.parseHeading(payload);
        break;
      case 0x03:
        this.state.gps = this.parseGps(payload);
        break;
      case 0x04:
        this.state.rcChannels = this.parseRcChannels(payload);
        break;
      case 0x05:
        this.state.flightMode = this.parseFlightMode(payload);
        break;
      case 0x06:
        this.state.pidValues = this.parsePidValues(payload);
        break;
      case 0x07:
        this.state.status = this.parseStatus(payload);
        break;
      case 0x08:
        this.state.battery = this.parseBattery(payload);
        break;
      default:
        return;
    }

    this.lastUpdate = Date.now();
    this.notifyListeners(msgId);
  }

  updateFirmwareData(data) {
    this.state.attitude = {
      roll: data.roll * Math.PI / 180,
      pitch: data.pitch * Math.PI / 180,
      yaw: data.yaw * Math.PI / 180
    };
    this.state.heading = { heading: data.yaw };
    this.state.rcChannels = { channels: data.rcChannels };
    this.state.flightMode = { mode: data.mode, armed: false };

    this.lastUpdate = Date.now();
    this.notifyListeners(0x01);
  }

  parseAttitude(payload) {
    const view = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
    return {
      roll: view.getFloat32(0, true),
      pitch: view.getFloat32(4, true),
      yaw: view.getFloat32(8, true)
    };
  }

  parseHeading(payload) {
    const view = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
    return {
      heading: view.getFloat32(0, true)
    };
  }

  parseGps(payload) {
    const view = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
    return {
      latitude: view.getInt32(0, true) / 1e7,
      longitude: view.getInt32(4, true) / 1e7,
      altitude: view.getInt32(8, true) / 1000,
      speed: view.getUint16(12, true) / 100,
      satellites: view.getUint8(14),
      fixType: view.getUint8(15)
    };
  }

  parseRcChannels(payload) {
    const view = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
    const channels = [];
    for (let i = 0; i < 8; i++) {
      channels.push(view.getUint16(i * 2, true));
    }
    return { channels };
  }

  parseFlightMode(payload) {
    return {
      mode: payload[0],
      armed: payload[1] === 1
    };
  }

  parsePidValues(payload) {
    const view = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
    return {
      roll: { p: view.getFloat32(0, true), i: view.getFloat32(4, true), d: view.getFloat32(8, true) },
      pitch: { p: view.getFloat32(12, true), i: view.getFloat32(16, true), d: view.getFloat32(20, true) },
      yaw: { p: view.getFloat32(24, true), i: view.getFloat32(28, true), d: view.getFloat32(32, true) }
    };
  }

  parseStatus(payload) {
    const view = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
    return {
      errors: view.getUint16(0, true),
      cpuLoad: view.getUint8(2),
      loopTime: view.getUint8(3)
    };
  }

  parseBattery(payload) {
    const view = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
    return {
      voltage: view.getUint16(0, true) / 1000,
      current: view.getInt16(2, true) / 1000,
      percentage: view.getUint8(4)
    };
  }

  subscribe(callback) {
    this.listeners.push(callback);
    return () => {
      this.listeners = this.listeners.filter(cb => cb !== callback);
    };
  }

  notifyListeners(msgId) {
    this.listeners.forEach(callback => callback(this.state, msgId));
  }

  getState() {
    return this.state;
  }

  getLastUpdate() {
    return this.lastUpdate;
  }

  isStale(timeoutMs = 1000) {
    return Date.now() - this.lastUpdate > timeoutMs;
  }
}

export default StateManager;
