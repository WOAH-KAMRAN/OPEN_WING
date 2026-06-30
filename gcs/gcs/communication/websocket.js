class WebSocketConnection {
  constructor(onData, onError) {
    this.ws = null;
    this.onData = onData;
    this.onError = onError;
    this.connected = false;
    this.reconnectInterval = null;
    this.reconnectDelay = 2000;
  }

  connect(url) {
    if (this.connected) return true;

    try {
      this.ws = new WebSocket(url);
      this.ws.binaryType = 'arraybuffer';

      this.ws.onopen = () => {
        this.connected = true;
        this.clearReconnect();
      };

      this.ws.onmessage = (event) => {
        const data = new Uint8Array(event.data);
        this.onData?.(data);
      };

      this.ws.onerror = (error) => {
        this.onError?.(error);
      };

      this.ws.onclose = () => {
        this.connected = false;
        this.scheduleReconnect(url);
      };

      return true;
    } catch (error) {
      this.onError?.(error);
      return false;
    }
  }

  disconnect() {
    this.clearReconnect();
    this.connected = false;

    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }

  write(data) {
    if (!this.connected || !this.ws) return false;

    try {
      this.ws.send(data);
      return true;
    } catch (error) {
      this.onError?.(error);
      return false;
    }
  }

  scheduleReconnect(url) {
    this.clearReconnect();
    this.reconnectInterval = setTimeout(() => {
      this.connect(url);
    }, this.reconnectDelay);
  }

  clearReconnect() {
    if (this.reconnectInterval) {
      clearTimeout(this.reconnectInterval);
      this.reconnectInterval = null;
    }
  }

  isConnected() {
    return this.connected;
  }
}

export default WebSocketConnection;
