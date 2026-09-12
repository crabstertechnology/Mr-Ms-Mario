#include "LunaNode.h"
#include <string.h>

LunaNode::LunaNode()
  : m_def(nullptr)
  , m_state(LUNA_BTN_STATE_NORMAL)
  , m_animStartMillis(0)
  , m_animatedRotation(0)
{
}

void LunaNode::init(const LunaNodeDef* def) {
  m_def = def;
  m_state = LUNA_BTN_STATE_NORMAL;
  m_animStartMillis = 0;
  m_animatedRotation = 0;
}

const LunaStyle& LunaNode::getStyle() const {
  if (m_def && m_state == LUNA_BTN_STATE_PRESSED && m_def->pressedStyle != nullptr) {
    return *(m_def->pressedStyle);
  }
  return m_def->style;
}

void LunaNode::setState(uint8_t state) {
  m_state = state;
}

bool LunaNode::containsPoint(int16_t px, int16_t py) const {
  if (!m_def) return false;
  const LunaRect& r = m_def->layout;
  return (px >= r.x && px < (r.x + r.width) &&
          py >= r.y && py < (r.y + r.height));
}

const LunaAction* LunaNode::getActionForTrigger(const char* trigger) const {
  if (!m_def || !trigger) return nullptr;
  for (uint8_t i = 0; i < m_def->eventCount; i++) {
    if (strcmp(m_def->events[i].trigger, trigger) == 0) {
      return &(m_def->events[i].action);
    }
  }
  return nullptr;
}

void LunaNode::update(uint32_t currentMillis) {
  if (!m_def || m_def->animationCount == 0) return;

  for (uint8_t i = 0; i < m_def->animationCount; i++) {
    const LunaAnimation& anim = m_def->animations[i];
    if (strcmp(anim.property, "rotation") == 0) {
      if (m_animStartMillis == 0) {
        m_animStartMillis = currentMillis;
      }
      uint32_t elapsed = currentMillis - m_animStartMillis;
      if (anim.durationMs > 0) {
        uint32_t cycleTime = elapsed % anim.durationMs;
        float progress = (float)cycleTime / (float)anim.durationMs; // linear easing
        int16_t range = anim.toValue - anim.fromValue;
        m_animatedRotation = anim.fromValue + (int16_t)(progress * range);
      }
    }
  }
}
