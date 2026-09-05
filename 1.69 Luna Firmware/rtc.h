#ifndef LUNA_RTC_H
#define LUNA_RTC_H

#include <Arduino.h>
#include <Wire.h>

#define PCF85063_I2C_ADDR 0x51

class LunaRTC {
private:
  uint16_t currentYear = 2026;
  uint8_t currentMonth = 9;
  uint8_t currentDay = 5;

  static uint8_t bcdToDec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
  }

  static uint8_t decToBcd(uint8_t val) {
    return (((val / 10) << 4) | (val % 10));
  }

  // Sakamoto's algorithm for day of week: 0 = Sun, 1 = Mon, ..., 6 = Sat
  static uint8_t calculateWeekday(uint16_t y, uint8_t m, uint8_t d) {
    static const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
  }

  static int monthStrToInt(const String& monStr) {
    String m = monStr;
    m.toLowerCase();
    if (m.startsWith("jan")) return 1;
    if (m.startsWith("feb")) return 2;
    if (m.startsWith("mar")) return 3;
    if (m.startsWith("apr")) return 4;
    if (m.startsWith("may")) return 5;
    if (m.startsWith("jun")) return 6;
    if (m.startsWith("jul")) return 7;
    if (m.startsWith("aug")) return 8;
    if (m.startsWith("sep")) return 9;
    if (m.startsWith("oct")) return 10;
    if (m.startsWith("nov")) return 11;
    if (m.startsWith("dec")) return 12;
    return 9; // Fallback to Sept
  }

public:
  LunaRTC() {}

  bool begin() {
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    if (Wire.endTransmission() != 0) {
      Serial.println(F("[RTC] PCF85063 not found on I2C bus!"));
      return false;
    }

    // Read seconds register 0x04 to check OS (Oscillator Stop) bit
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(0x04);
    if (Wire.endTransmission(false) == 0 && Wire.requestFrom(PCF85063_I2C_ADDR, 1) == 1) {
      uint8_t secReg = Wire.read();
      bool osBit = (secReg & 0x80) != 0;
      if (osBit) {
        Serial.println(F("[RTC] Oscillator stopped. Initializing PCF85063 with default time..."));
        // Initialize Control_1 (0x00) for 24-hour mode & clear STOP
        Wire.beginTransmission(PCF85063_I2C_ADDR);
        Wire.write(0x00);
        Wire.write(0x00); // STOP=0, 24H=0
        Wire.endTransmission();

        // Set default time: 20:04:00, 5 Sep 2026
        setTimeFull(2026, 9, 5, 20, 4, 0);
      } else {
        Serial.println(F("[RTC] PCF85063 initialized and running successfully."));
      }
      return true;
    }
    return false;
  }

  bool readTime(int &hour, int &minute, int &second, String &dayStr, String &dateStr) {
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(0x04);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(PCF85063_I2C_ADDR, 7) < 7) return false;

    uint8_t secReg  = Wire.read();
    uint8_t minReg  = Wire.read();
    uint8_t hrReg   = Wire.read();
    uint8_t dayReg  = Wire.read();
    uint8_t wdayReg = Wire.read();
    uint8_t monReg  = Wire.read();
    uint8_t yrReg   = Wire.read();

    second = bcdToDec(secReg & 0x7F);
    minute = bcdToDec(minReg & 0x7F);
    hour   = bcdToDec(hrReg & 0x3F);

    uint8_t day   = bcdToDec(dayReg & 0x3F);
    uint8_t wday  = bcdToDec(wdayReg & 0x07);
    uint8_t month = bcdToDec(monReg & 0x1F);
    uint16_t year = 2000 + bcdToDec(yrReg);

    if (day >= 1 && day <= 31 && month >= 1 && month <= 12) {
      currentDay = day;
      currentMonth = month;
      currentYear = year;
    }

    const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if (wday < 7) {
      dayStr = dayNames[wday];
    } else {
      dayStr = dayNames[calculateWeekday(currentYear, currentMonth, currentDay)];
    }

    char dBuf[12];
    snprintf(dBuf, sizeof(dBuf), "%02d %s", currentDay, (currentMonth >= 1 && currentMonth <= 12) ? monthNames[currentMonth - 1] : "Sep");
    dateStr = String(dBuf);

    return true;
  }

  bool setTimeFull(uint16_t year, uint8_t month, uint8_t day, int hour, int minute, int second) {
    if (year < 2000) year += 2000;
    currentYear = year;
    currentMonth = month;
    currentDay = day;

    uint8_t wday = calculateWeekday(year, month, day);
    uint8_t yrShort = year % 100;

    // Ensure Control_1 (0x00) is running in 24h mode
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();

    // Write time registers starting at 0x04
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(0x04);
    Wire.write(decToBcd(second) & 0x7F); // OS bit (bit 7) = 0
    Wire.write(decToBcd(minute) & 0x7F);
    Wire.write(decToBcd(hour) & 0x3F);
    Wire.write(decToBcd(day) & 0x3F);
    Wire.write(decToBcd(wday) & 0x07);
    Wire.write(decToBcd(month) & 0x1F);
    Wire.write(decToBcd(yrShort));
    return (Wire.endTransmission() == 0);
  }

  bool setTime(int hour, int minute, int second, String dayStr = "", String dateStr = "") {
    if (dateStr.length() > 0) {
      int spaceIdx = dateStr.indexOf(' ');
      if (spaceIdx > 0) {
        int d = dateStr.substring(0, spaceIdx).toInt();
        String mStr = dateStr.substring(spaceIdx + 1);
        int m = monthStrToInt(mStr);
        if (d >= 1 && d <= 31) currentDay = d;
        if (m >= 1 && m <= 12) currentMonth = m;
      }
    }
    return setTimeFull(currentYear, currentMonth, currentDay, hour, minute, second);
  }
};

#endif // LUNA_RTC_H
