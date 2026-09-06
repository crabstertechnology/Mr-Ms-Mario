#ifndef LUNA_RTC_H
#define LUNA_RTC_H

#include <Arduino.h>
#include <Wire.h>

#define PCF85063_I2C_ADDR 0x51

// PCF85063A register map
#define PCF85063_REG_CTRL1   0x00  // Control_1: STOP (bit5), 12/24h (bit1), CAP_SEL (bit0)
#define PCF85063_REG_SECONDS 0x04  // Seconds (OS flag in bit7)

// ── Compile-time date/time helpers ───────────────────────────────────────────
// __DATE__ format: "Sep  5 2026"   __TIME__ format: "20:58:36"
// These macros let us seed the RTC with the actual build timestamp on first boot.

#define BUILD_YEAR  (__DATE__[7]-'0')*1000 + (__DATE__[8]-'0')*100 + (__DATE__[9]-'0')*10 + (__DATE__[10]-'0')

#define BUILD_MONTH ( \
  (__DATE__[0]=='J' && __DATE__[1]=='a') ? 1  : \
  (__DATE__[0]=='F')                     ? 2  : \
  (__DATE__[0]=='M' && __DATE__[2]=='r') ? 3  : \
  (__DATE__[0]=='A' && __DATE__[1]=='p') ? 4  : \
  (__DATE__[0]=='M' && __DATE__[2]=='y') ? 5  : \
  (__DATE__[0]=='J' && __DATE__[2]=='n') ? 6  : \
  (__DATE__[0]=='J' && __DATE__[2]=='l') ? 7  : \
  (__DATE__[0]=='A' && __DATE__[1]=='u') ? 8  : \
  (__DATE__[0]=='S')                     ? 9  : \
  (__DATE__[0]=='O')                     ? 10 : \
  (__DATE__[0]=='N')                     ? 11 : 12 )

#define BUILD_DAY   ((__DATE__[4]==' ') ? (__DATE__[5]-'0') : (__DATE__[4]-'0')*10+(__DATE__[5]-'0'))
#define BUILD_HOUR  ((__TIME__[0]-'0')*10 + (__TIME__[1]-'0'))
#define BUILD_MIN   ((__TIME__[3]-'0')*10 + (__TIME__[4]-'0'))
#define BUILD_SEC   ((__TIME__[6]-'0')*10 + (__TIME__[7]-'0'))
// ─────────────────────────────────────────────────────────────────────────────


class LunaRTC {
private:
  uint16_t currentYear  = 2024;
  uint8_t  currentMonth = 1;
  uint8_t  currentDay   = 1;

  static uint8_t bcdToDec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
  }

  static uint8_t decToBcd(uint8_t val) {
    return (((val / 10) << 4) | (val % 10));
  }

  // Sakamoto's algorithm for day of week: 0 = Sun, 1 = Mon, …, 6 = Sat
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
    return 1;
  }

  // Write a single byte to a register
  bool writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return (Wire.endTransmission() == 0);
  }

  // Read a single byte from a register
  bool readReg(uint8_t reg, uint8_t &val) {
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(PCF85063_I2C_ADDR, 1) < 1) return false;
    val = Wire.read();
    return true;
  }

public:
  LunaRTC() {}

  bool begin() {
    // Check the device is on the bus
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    if (Wire.endTransmission() != 0) {
      Serial.println(F("[RTC] PCF85063 not found on I2C bus!"));
      return false;
    }

    // Always clear STOP bit and set 24h mode in Control_1 (0x00)
    writeReg(PCF85063_REG_CTRL1, 0x00);

    // Read seconds register to check OS (Oscillator Stop) bit
    uint8_t secReg = 0;
    if (!readReg(PCF85063_REG_SECONDS, secReg)) {
      Serial.println(F("[RTC] Failed to read seconds register!"));
      return false;
    }

    bool osBit = (secReg & 0x80) != 0;
    if (osBit) {
      Serial.println(F("[RTC] Oscillator was stopped — clearing OS bit and starting clock."));
      // Clear STOP bit and OS bit: Control_1 = 0x00 (24h mode, oscillator running)
      writeReg(PCF85063_REG_CTRL1, 0x00);
      // Use compile-time date/time as the initial fallback when RTC oscillator
      // was stopped (e.g. first power-on or battery removed).
      setTimeFull(BUILD_YEAR, BUILD_MONTH, BUILD_DAY,
                  BUILD_HOUR, BUILD_MIN, BUILD_SEC);
      Serial.println(F("[RTC] Time initialised from compile-time. Send TIME: command to sync."));
    } else {
      Serial.println(F("[RTC] PCF85063 oscillator is running, reading current time."));
    }
    return true;
  }

  bool readTime(int &hour, int &minute, int &second,
                String &dayStr, String &dateStr) {
    // Ensure STOP bit in Control_1 is NOT set
    uint8_t ctrl1 = 0;
    if (readReg(PCF85063_REG_CTRL1, ctrl1)) {
      if (ctrl1 & 0x20) { // STOP bit (bit 5) active
        writeReg(PCF85063_REG_CTRL1, 0x00); // Clear STOP bit to start clock oscillator
      }
    }

    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(PCF85063_REG_SECONDS);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(PCF85063_I2C_ADDR, 7) < 7) return false;

    uint8_t secReg  = Wire.read();  // 0x04
    uint8_t minReg  = Wire.read();  // 0x05
    uint8_t hrReg   = Wire.read();  // 0x06
    uint8_t dayReg  = Wire.read();  // 0x07
    uint8_t wdayReg = Wire.read();  // 0x08
    uint8_t monReg  = Wire.read();  // 0x09
    uint8_t yrReg   = Wire.read();  // 0x0A

    // If oscillator-stop flag is set, the time is invalid
    if (secReg & 0x80) {
      Serial.println(F("[RTC] OS bit set during read — time invalid"));
      return false;
    }

    second = bcdToDec(secReg & 0x7F);
    minute = bcdToDec(minReg & 0x7F);
    hour   = bcdToDec(hrReg  & 0x3F);

    uint8_t  day   = bcdToDec(dayReg  & 0x3F);
    uint8_t  wday  = bcdToDec(wdayReg & 0x07);
    uint8_t  month = bcdToDec(monReg  & 0x1F);
    uint16_t year  = 2000 + bcdToDec(yrReg);

    // Sanity checks — reject garbage values
    if (second > 59 || minute > 59 || hour > 23) return false;
    if (day < 1 || day > 31 || month < 1 || month > 12) return false;

    currentDay   = day;
    currentMonth = month;
    currentYear  = year;

    const char* dayNames[]   = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char* monthNames[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                "Jul","Aug","Sep","Oct","Nov","Dec"};

    if (wday < 7) {
      dayStr = dayNames[wday];
    } else {
      dayStr = dayNames[calculateWeekday(currentYear, currentMonth, currentDay)];
    }

    char dBuf[12];
    snprintf(dBuf, sizeof(dBuf), "%02d %s", currentDay, monthNames[currentMonth - 1]);
    dateStr = String(dBuf);

    return true;
  }

  // Full time write using PCF85063 STOP-bit protocol to prevent partial writes
  bool setTimeFull(uint16_t year, uint8_t month, uint8_t day,
                   int hour, int minute, int second) {
    if (year < 2000) year += 2000;
    if (year > 2099) year = 2099;

    // Clamp values to valid ranges
    second = constrain(second, 0, 59);
    minute = constrain(minute, 0, 59);
    hour   = constrain(hour,   0, 23);
    day    = constrain(day,    1, 31);
    month  = constrain(month,  1, 12);

    currentYear  = year;
    currentMonth = month;
    currentDay   = day;

    uint8_t wday    = calculateWeekday(year, month, day);
    uint8_t yrShort = (uint8_t)(year % 100);

    // Step 1: Set STOP bit (bit 5 of Control_1) to halt oscillator during write
    writeReg(PCF85063_REG_CTRL1, 0x20); // STOP = 1
    delayMicroseconds(50);

    // Step 2: Write all 7 time registers in one burst starting at 0x04
    Wire.beginTransmission(PCF85063_I2C_ADDR);
    Wire.write(PCF85063_REG_SECONDS);
    Wire.write(decToBcd(second) & 0x7F);  // Clear OS bit (bit 7)
    Wire.write(decToBcd(minute) & 0x7F);
    Wire.write(decToBcd(hour)   & 0x3F);
    Wire.write(decToBcd(day)    & 0x3F);
    Wire.write(decToBcd(wday)   & 0x07);
    Wire.write(decToBcd(month)  & 0x1F);
    Wire.write(decToBcd(yrShort));
    bool ok = (Wire.endTransmission() == 0);

    // Step 3: Clear STOP bit to restart oscillator
    writeReg(PCF85063_REG_CTRL1, 0x00); // STOP = 0, 24h mode
    delayMicroseconds(50);

    if (ok) {
      Serial.printf("[RTC] Time set: %04d-%02d-%02d %02d:%02d:%02d (wday=%d)\n",
                    year, month, day, hour, minute, second, wday);
    } else {
      Serial.println(F("[RTC] setTimeFull write failed!"));
    }
    return ok;
  }

  // Convenience: set just H:M:S, keeping stored date (optionally parse date from dateStr)
  bool setTime(int hour, int minute, int second,
               String dayStr = "", String dateStr = "") {
    if (dateStr.length() > 0) {
      int spaceIdx = dateStr.indexOf(' ');
      if (spaceIdx > 0) {
        int d = dateStr.substring(0, spaceIdx).toInt();
        String mStr = dateStr.substring(spaceIdx + 1);
        int m = monthStrToInt(mStr);
        if (d >= 1 && d <= 31) currentDay   = (uint8_t)d;
        if (m >= 1 && m <= 12) currentMonth = (uint8_t)m;
      }
    }
    return setTimeFull(currentYear, currentMonth, currentDay, hour, minute, second);
  }

  // Getters for cached date (used when RTC date read is unavailable)
  uint16_t getYear()  const { return currentYear;  }
  uint8_t  getMonth() const { return currentMonth; }
  uint8_t  getDay()   const { return currentDay;   }
};

#endif // LUNA_RTC_H
