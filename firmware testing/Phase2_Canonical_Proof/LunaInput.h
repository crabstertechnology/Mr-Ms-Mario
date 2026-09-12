#ifndef LUNA_INPUT_H
#define LUNA_INPUT_H

#include <Arduino.h>
#include <Wire.h>

#define LUNA_CST816T_I2C_ADDR 0x15

class LunaInput {
public:
  LunaInput();

  void begin(int8_t sda = 11, int8_t scl = 10, int8_t rst = 13, int8_t irq = 14);
  bool update();

  bool isPressed() const { return m_isPressed; }
  bool justPressed() const { return m_justPressed; }
  bool justReleased() const { return m_justReleased; }

  int16_t getX() const { return m_touchX; }
  int16_t getY() const { return m_touchY; }
  uint8_t getGesture() const { return m_gesture; }

private:
  int8_t m_pinSda;
  int8_t m_pinScl;
  int8_t m_pinRst;
  int8_t m_pinIrq;

  bool m_isPressed;
  bool m_prevPressed;
  bool m_justPressed;
  bool m_justReleased;

  int16_t m_touchX;
  int16_t m_touchY;
  uint8_t m_gesture;

  bool readRawTouch(int16_t &x, int16_t &y, uint8_t &gesture);
};

#endif // LUNA_INPUT_H
