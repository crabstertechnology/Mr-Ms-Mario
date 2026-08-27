// =============================================================================
// imu.h  —  Direct I2C interface for QMI8658 Accelerometer & Gyroscope
// =============================================================================
#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>

class LunaIMU {
private:
  uint8_t i2cAddr;
  bool initialized;

  // Direct I2C helper write
  void writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(i2cAddr);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
  }

  // Direct I2C helper read
  uint8_t readReg(uint8_t reg) {
    Wire.beginTransmission(i2cAddr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(i2cAddr, (uint8_t)1);
    if (Wire.available()) {
      return Wire.read();
    }
    return 0;
  }

  // Burst read helper
  bool readBurst(uint8_t startReg, uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(i2cAddr);
    Wire.write(startReg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(i2cAddr, length) < length) return false;
    for (uint8_t i = 0; i < length; i++) {
      buffer[i] = Wire.read();
    }
    return true;
  }

public:
  LunaIMU() : i2cAddr(0x6B), initialized(false) {}

  bool begin() {
    // Check WHO_AM_I on both possible I2C addresses (0x6B and 0x6A)
    uint8_t whoami = 0;
    
    i2cAddr = 0x6B;
    whoami = readReg(0x00);
    if (whoami != 0x05) {
      i2cAddr = 0x6A;
      whoami = readReg(0x00);
    }

    if (whoami != 0x05) {
      Serial.printf("[IMU] QMI8658 not detected (WHO_AM_I = 0x%02X)\n", whoami);
      initialized = false;
      return false;
    }

    Serial.printf("[IMU] Found QMI8658 at address 0x%02X\n", i2cAddr);

    // Soft reset
    writeReg(0x60, 0xB0);
    delay(20);

    // CTRL1: Configuration (Interface Control, address auto-increment)
    // Bit 6 enables address auto-increment for burst reads
    writeReg(0x02, 0x40);

    // CTRL2: Accelerometer Configuration (125Hz, ±2g)
    // aFS is bits [6:4] -> 000 (±2g)
    // aODR is bits [3:0] -> 0110 (125Hz) -> 0x06
    writeReg(0x03, 0x06);

    // CTRL3: Gyroscope Configuration (125Hz, ±512 dps)
    // gFS is bits [6:4] -> 101 (±512 dps) -> 0x50
    // gODR is bits [3:0] -> 0110 (125Hz) -> 0x06
    writeReg(0x04, 0x56);

    // CTRL7: Enable both Accelerometer and Gyroscope
    // Bit 0 = Accel Enable, Bit 1 = Gyro Enable
    writeReg(0x08, 0x03);

    initialized = true;
    Serial.println("[IMU] QMI8658 successfully initialized.");
    return true;
  }

  bool isInitialized() const { return initialized; }

  // Reads Accelerometer (g) and Gyroscope (dps) values
  bool readMotion(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
    if (!initialized) return false;

    uint8_t buffer[12];
    // Read 12 bytes starting from AXL_X_L (0x35)
    if (!readBurst(0x35, buffer, 12)) return false;

    // Convert raw 16-bit signed integers (little-endian)
    int16_t raw_ax = (int16_t)((buffer[1] << 8) | buffer[0]);
    int16_t raw_ay = (int16_t)((buffer[3] << 8) | buffer[2]);
    int16_t raw_az = (int16_t)((buffer[5] << 8) | buffer[4]);

    int16_t raw_gx = (int16_t)((buffer[7] << 8) | buffer[6]);
    int16_t raw_gy = (int16_t)((buffer[9] << 8) | buffer[8]);
    int16_t raw_gz = (int16_t)((buffer[11] << 8) | buffer[10]);

    // Sensitivity multipliers:
    // For ±2g scale, sensitivity is 16384 LSB/g (32768 / 2)
    // For ±512 dps scale, sensitivity is 64 LSB/dps (32768 / 512)
    ax = (float)raw_ax / 16384.0f;
    ay = (float)raw_ay / 16384.0f;
    az = (float)raw_az / 16384.0f;

    gx = (float)raw_gx / 64.0f;
    gy = (float)raw_gy / 64.0f;
    gz = (float)raw_gz / 64.0f;

    return true;
  }
};

#endif // IMU_H
