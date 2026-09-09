/**
 * Luna Display Studio - Web Serial Communication Bridge
 * Connects directly to ESP32 on COM3 via browser Web Serial API to preview layouts live!
 */

class WebSerialBridge {
  constructor(statusCallback) {
    this.port = null;
    this.writer = null;
    this.isConnected = false;
    this.statusCallback = statusCallback;
  }

  isSupported() {
    return "serial" in navigator;
  }

  async connect() {
    if (!this.isSupported()) {
      alert("Web Serial is supported in Google Chrome, Microsoft Edge, and Opera.\nPlease open this studio in Chrome or Edge to connect to COM3.");
      return false;
    }

    try {
      this.port = await navigator.serial.requestPort();
      await this.port.open({ baudRate: 115200 });

      const textEncoder = new TextEncoderStream();
      textEncoder.readable.pipeTo(this.port.writable);
      this.writer = textEncoder.writable.getWriter();

      this.isConnected = true;
      if (this.statusCallback) this.statusCallback(true, "Connected to ESP32");
      return true;
    } catch (err) {
      console.error("Web Serial connection failed:", err);
      if (this.statusCallback) this.statusCallback(false, "Connection Failed");
      return false;
    }
  }

  async disconnect() {
    if (this.writer) {
      await this.writer.close();
      this.writer = null;
    }
    if (this.port) {
      await this.port.close();
      this.port = null;
    }
    this.isConnected = false;
    if (this.statusCallback) this.statusCallback(false, "Disconnected");
  }

  async sendLayoutCommand(command) {
    if (!this.isConnected || !this.writer) return false;
    try {
      await this.writer.write(command + "\n");
      return true;
    } catch (err) {
      console.error("Error writing to serial:", err);
      return false;
    }
  }
}
