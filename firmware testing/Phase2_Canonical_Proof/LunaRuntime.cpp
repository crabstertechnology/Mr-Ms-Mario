#include "LunaRuntime.h"
#include <esp_heap_caps.h>

LunaRuntime::LunaRuntime()
  : m_projectDef(nullptr)
  , m_screenCount(0)
  , m_activeScreen(nullptr)
  , m_dirty(true)
  , m_pressedNode(nullptr)
{
}

bool LunaRuntime::begin(const LunaProjectDef* projectDef, Arduino_Canvas* canvas, uint16_t width, uint16_t height) {
  m_projectDef = projectDef;
  m_screenCount = 0;
  m_activeScreen = nullptr;
  m_pressedNode = nullptr;
  m_dirty = true;

  m_renderer.begin(canvas, width, height);
  m_input.begin();

  if (!m_projectDef) {
    return false;
  }

  uint8_t count = m_projectDef->screenCount;
  if (count > LUNA_MAX_SCREENS) {
    count = LUNA_MAX_SCREENS;
  }
  m_screenCount = count;

  for (uint8_t i = 0; i < m_screenCount; i++) {
    m_screens[i].init(&(m_projectDef->screens[i]));
  }

  // Set initial screen
  if (m_projectDef->initialScreenId) {
    m_activeScreen = findScreenById(m_projectDef->initialScreenId);
  }
  if (!m_activeScreen && m_screenCount > 0) {
    m_activeScreen = &m_screens[0];
  }

  return (m_activeScreen != nullptr);
}

LunaScreen* LunaRuntime::findScreenById(const char* screenId) {
  if (!screenId) return nullptr;
  for (uint8_t i = 0; i < m_screenCount; i++) {
    if (strcmp(m_screens[i].getId(), screenId) == 0) {
      return &m_screens[i];
    }
  }
  return nullptr;
}

const char* LunaRuntime::getActiveScreenId() const {
  return m_activeScreen ? m_activeScreen->getId() : "";
}

bool LunaRuntime::navigateTo(const char* targetScreenId) {
  LunaScreen* target = findScreenById(targetScreenId);
  if (target && target != m_activeScreen) {
    if (m_pressedNode) {
      m_pressedNode->setState(LUNA_BTN_STATE_NORMAL);
      m_pressedNode = nullptr;
    }
    m_activeScreen = target;
    m_dirty = true;
    return true;
  }
  return false;
}

void LunaRuntime::updateInput() {
  m_input.update();

  if (!m_activeScreen) return;

  // Touch Press Detection
  if (m_input.justPressed()) {
    int16_t tx = m_input.getX();
    int16_t ty = m_input.getY();

    LunaNode* node = m_activeScreen->findNodeAt(tx, ty);
    Serial.printf("[Touch] Down at (%d, %d) | Hit: %s\n", tx, ty, node ? node->getId() : "none");
    if (node && (node->getNodeType() == LUNA_NODE_BUTTON || node->getActionForTrigger("onClick") != nullptr)) {
      node->setState(LUNA_BTN_STATE_PRESSED);
      m_pressedNode = node;
      m_dirty = true;
    }
  }

  // Touch Release Detection
  if (m_input.justReleased()) {
    int16_t tx = m_input.getX();
    int16_t ty = m_input.getY();
    Serial.printf("[Touch] Up at (%d, %d)\n", tx, ty);
    if (m_pressedNode) {
      m_pressedNode->setState(LUNA_BTN_STATE_NORMAL);

      if (m_pressedNode->containsPoint(tx, ty)) {
        const LunaAction* action = m_pressedNode->getActionForTrigger("onClick");
        if (action && action->type == LUNA_ACTION_NAVIGATE && action->targetScreenId) {
          Serial.printf("[Action] Navigate to: %s\n", action->targetScreenId);
          navigateTo(action->targetScreenId);
        }
      }

      m_pressedNode = nullptr;
      m_dirty = true;
    }
  }

  // Gesture Navigation Detection (CST816T: 0x03=Left swipe, 0x04=Right swipe)
  uint8_t gesture = m_input.getGesture();
  if (gesture == 0x03 && m_activeScreen->getSwipeLeftTarget()) {
    navigateTo(m_activeScreen->getSwipeLeftTarget());
  } else if (gesture == 0x04 && m_activeScreen->getSwipeRightTarget()) {
    navigateTo(m_activeScreen->getSwipeRightTarget());
  }
}

void LunaRuntime::updateRuntime() {
  if (!m_activeScreen) return;

  uint32_t now = millis();
  m_activeScreen->update(now);

  if (m_activeScreen->hasActiveAnimation()) {
    m_dirty = true;
  }
}

void LunaRuntime::renderIfNeeded() {
  if (m_dirty && m_activeScreen) {
    m_renderer.drawScreen(*m_activeScreen);
    m_renderer.present();
    m_dirty = false;
  }
}

uint32_t LunaRuntime::getFreeInternalHeap() const {
  return heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}

uint32_t LunaRuntime::getFreePsram() const {
  return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
}
