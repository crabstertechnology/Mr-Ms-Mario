#include "LunaInput.h"

LunaInput::LunaInput()
  : m_pinSda(11)
  , m_pinScl(10)
  , m_pinRst(13)
  , m_pinIrq(14)
  , m_isPressed(false)
  , m_prevPressed(false)
  , m_justPressed(false)
  , m_justReleased(false)
  , m_touchX(0)
  , m_touchY(0)
  , m_gesture(0)
{
}

void LunaInput::begin(int8_t sda, int8_t scl, int8_t rst, int8_t irq) {
  m_pinSda = sda;
  m_pinScl = scl;
  m_pinRst = rst;
  m_pinIrq = irq;

  if (m_pinRst >= 0) {
    pinMode(m_pinRst, OUTPUT);
    digitalWrite(m_pinRst, LOW);
    delay(10);
    digitalWrite(m_pinRst, HIGH);
    delay(50);
  }

  Wire.begin(m_pinSda, m_pinScl, 400000);
  Wire.setTimeOut(50);

  if (m_pinIrq >= 0) {
    pinMode(m_pinIrq, INPUT_PULLUP);
  }
}

bool LunaInput::readRawTouch(int16_t &x, int16_t &y, uint8_t &gesture) {
  if (m_pinIrq >= 0 && digitalRead(m_pinIrq) == HIGH) {
    return false;
  }

  Wire.beginTransmission(LUNA_CST816T_I2C_ADDR);
  Wire.write(0x01);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom((uint16_t)LUNA_CST816T_I2C_ADDR, (uint8_t)6) < 6) {
    return false;
  }

  gesture           = Wire.read();
  uint8_t fingerNum = Wire.read();
  uint8_t xH        = Wire.read();
  uint8_t xL        = Wire.read();
  uint8_t yH        = Wire.read();
  uint8_t yL        = Wire.read();

  if (fingerNum == 0) {
    return false;
  }

  int rawX = ((xH & 0x0F) << 8) | xL;
  int rawY = ((yH & 0x0F) << 8) | yL;

  x = (int16_t)constrain(rawX, 0, 239);
  y = (int16_t)constrain(rawY, 0, 279);
  return true;
}

bool LunaInput::update() {
  m_prevPressed = m_isPressed;

  int16_t x = 0;
  int16_t y = 0;
  uint8_t gesture = 0;

  bool touched = readRawTouch(x, y, gesture);
  if (touched) {
    m_isPressed = true;
    m_touchX = x;
    m_touchY = y;
    m_gesture = gesture;
  } else {
    m_isPressed = false;
  }

  m_justPressed  = (!m_prevPressed && m_isPressed);
  m_justReleased = (m_prevPressed && !m_isPressed);

  return m_isPressed;
}
