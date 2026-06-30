import { MessageIds, MessageSizes } from './message_definitions.js';

const PROTOCOL_VERSION = 1;
const PROTOCOL_HEADER = 0xAA55;
const CRC16_POLY = 0x1021;
const CRC16_INIT = 0xFFFF;

const FIRMWARE_HEADER_0 = 0xAA;
const FIRMWARE_HEADER_1 = 0x55;
const FIRMWARE_PACKET_SIZE = 29;

class ProtocolParser {
  constructor(protocol = 'new') {
    this.buffer = new Uint8Array(4096);
    this.bufferLength = 0;
    this.protocol = protocol;
  }

  setProtocol(protocol) {
    this.protocol = protocol;
    this.bufferLength = 0;
  }

  crc16(data) {
    let crc = CRC16_INIT;
    for (let i = 0; i < data.length; i++) {
      crc ^= (data[i] << 8);
      for (let j = 0; j < 8; j++) {
        if (crc & 0x8000) {
          crc = (crc << 1) ^ CRC16_POLY;
        } else {
          crc <<= 1;
        }
        crc &= 0xFFFF;
      }
    }
    return crc;
  }

  calculateFirmwareChecksum(data) {
    let checksum = 0;
    for (let i = 0; i < data.length; i++) {
      checksum += data[i];
    }
    return checksum & 0xFFFF;
  }

  feed(data) {
    if (this.bufferLength + data.length > this.buffer.length) {
      const newBuffer = new Uint8Array(this.buffer.length * 2);
      newBuffer.set(this.buffer);
      this.buffer = newBuffer;
    }
    this.buffer.set(data, this.bufferLength);
    this.bufferLength += data.length;
  }

  parse() {
    if (this.protocol === 'firmware') {
      return this.parseFirmwareProtocol();
    }
    return this.parseNewProtocol();
  }

  parseFirmwareProtocol() {
    const packets = [];

    while (this.bufferLength >= FIRMWARE_PACKET_SIZE) {
      if (this.buffer[0] !== FIRMWARE_HEADER_0 || this.buffer[1] !== FIRMWARE_HEADER_1) {
        this.bufferLength--;
        this.buffer.copyWithin(0, 1, this.bufferLength);
        continue;
      }

      const packetData = this.buffer.subarray(0, FIRMWARE_PACKET_SIZE);
      const checksumData = packetData.subarray(0, FIRMWARE_PACKET_SIZE - 2);
      const checksumCalc = this.calculateFirmwareChecksum(checksumData);
      const checksumRecv = (packetData[FIRMWARE_PACKET_SIZE - 2] << 8) | packetData[FIRMWARE_PACKET_SIZE - 1];

      if (checksumCalc !== checksumRecv) {
        this.bufferLength--;
        this.buffer.copyWithin(0, 1, this.bufferLength);
        continue;
      }

      const view = new DataView(packetData.buffer, packetData.byteOffset, packetData.byteLength);
      const payload = {
        roll: view.getFloat32(2, true),
        pitch: view.getFloat32(6, true),
        yaw: view.getFloat32(10, true),
        rcChannels: [],
        mode: view.getUint8(26)
      };

      for (let i = 0; i < 6; i++) {
        payload.rcChannels.push(view.getUint16(14 + i * 2, true));
      }

      packets.push({
        msgId: 0x01,
        payload: new Uint8Array([0]),
        firmwareData: payload
      });

      this.bufferLength -= FIRMWARE_PACKET_SIZE;
      this.buffer.copyWithin(0, FIRMWARE_PACKET_SIZE, this.bufferLength + FIRMWARE_PACKET_SIZE);
    }

    return packets;
  }

  parseNewProtocol() {
    const packets = [];

    while (this.bufferLength >= 7) {
      const header = (this.buffer[0] << 8) | this.buffer[1];
      if (header !== PROTOCOL_HEADER) {
        this.bufferLength--;
        this.buffer.copyWithin(0, 1, this.bufferLength);
        continue;
      }

      const version = this.buffer[2];
      const msgId = this.buffer[3];
      const payloadLen = this.buffer[4];
      const totalLen = 6 + payloadLen + 2;

      if (this.bufferLength < totalLen) {
        break;
      }

      const crcData = this.buffer.subarray(2, 5 + payloadLen);
      const crcCalc = this.crc16(crcData);
      const crcRecv = (this.buffer[5 + payloadLen] << 8) | this.buffer[6 + payloadLen];

      if (crcCalc !== crcRecv) {
        this.bufferLength--;
        this.buffer.copyWithin(0, 1, this.bufferLength);
        continue;
      }

      const payload = this.buffer.subarray(5, 5 + payloadLen);
      packets.push({
        version,
        msgId,
        payload
      });

      this.bufferLength -= totalLen;
      this.buffer.copyWithin(0, totalLen, this.bufferLength + totalLen);
    }

    return packets;
  }

  encode(msgId, payload) {
    const totalLen = 6 + payload.length + 2;
    const buffer = new Uint8Array(totalLen);

    buffer[0] = (PROTOCOL_HEADER >> 8) & 0xFF;
    buffer[1] = PROTOCOL_HEADER & 0xFF;
    buffer[2] = PROTOCOL_VERSION;
    buffer[3] = msgId;
    buffer[4] = payload.length;

    buffer.set(payload, 5);

    const crcData = buffer.subarray(2, 5 + payload.length);
    const crc = this.crc16(crcData);
    buffer[5 + payload.length] = (crc >> 8) & 0xFF;
    buffer[6 + payload.length] = crc & 0xFF;

    return buffer;
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

  encodeSetPid(axis, p, i, d) {
    const buffer = new Uint8Array(13);
    const view = new DataView(buffer.buffer);
    view.setUint8(0, axis);
    view.setFloat32(1, p, true);
    view.setFloat32(5, i, true);
    view.setFloat32(9, d, true);
    return this.encode(MessageIds.SET_PID, buffer);
  }

  encodeGetPid(axis) {
    const buffer = new Uint8Array([axis]);
    return this.encode(MessageIds.GET_PID, buffer);
  }

  encodeSetMode(mode) {
    const buffer = new Uint8Array([mode]);
    return this.encode(MessageIds.SET_MODE, buffer);
  }

  encodeArmDisarm(arm) {
    const buffer = new Uint8Array([arm ? 1 : 0]);
    return this.encode(MessageIds.ARM_DISARM, buffer);
  }

  encodeReboot() {
    return this.encode(MessageIds.REBOOT, new Uint8Array(0));
  }
}

export default ProtocolParser;
