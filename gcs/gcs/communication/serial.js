class SerialConnection {
  constructor(onData, onError) {
    this.port = null;
    this.reader = null;
    this.writer = null;
    this.onData = onData;
    this.onError = onError;
    this.connected = false;
    this.keepReading = false;
  }

  async connect() {
    if (this.connected) return true;

    if (!navigator.serial) {
      const error = new Error('WebSerial not supported. Enable Experimental Web Platform features in brave://flags or use Chrome/Edge.');
      console.error('SerialConnection: WebSerial not supported');
      this.onError?.(error);
      return false;
    }

    console.log('SerialConnection: Requesting port...');
    try {
      this.port = await navigator.serial.requestPort();
      console.log('SerialConnection: Port selected');
      await this.port.open({ baudRate: 115200 });
      console.log('SerialConnection: Port opened');
      this.connected = true;
      this.keepReading = true;
      this.readLoop();
      return true;
    } catch (error) {
      console.error('SerialConnection: Error', error);
      this.onError?.(error);
      return false;
    }
  }

  async disconnect() {
    this.keepReading = false;
    this.connected = false;

    if (this.reader) {
      try {
        await this.reader.cancel();
      } catch (e) {}
    }

    if (this.port) {
      try {
        await this.port.close();
      } catch (e) {}
    }

    this.port = null;
    this.reader = null;
    this.writer = null;
  }

  async readLoop() {
    while (this.port.readable && this.keepReading) {
      this.reader = this.port.readable.getReader();
      try {
        while (true) {
          const { value, done } = await this.reader.read();
          if (done) break;
          if (value) this.onData?.(value);
        }
      } catch (error) {
        this.onError?.(error);
        break;
      } finally {
        this.reader.releaseLock();
      }
    }
  }

  async write(data) {
    if (!this.connected || !this.port.writable) return false;

    if (!this.writer) {
      this.writer = this.port.writable.getWriter();
    }

    try {
      await this.writer.write(data);
      return true;
    } catch (error) {
      this.onError?.(error);
      return false;
    }
  }

  isConnected() {
    return this.connected;
  }
}

export default SerialConnection;
