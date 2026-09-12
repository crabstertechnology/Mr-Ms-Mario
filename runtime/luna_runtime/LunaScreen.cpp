#include "LunaScreen.h"
#include <string.h>

LunaScreen::LunaScreen()
  : m_def(nullptr)
  , m_nodeCount(0)
{
}

void LunaScreen::init(const LunaScreenDef* def) {
  m_def = def;
  m_nodeCount = 0;

  if (m_def) {
    uint8_t count = m_def->nodeCount;
    if (count > LUNA_MAX_NODES_PER_SCREEN) {
      count = LUNA_MAX_NODES_PER_SCREEN;
    }
    m_nodeCount = count;
    for (uint8_t i = 0; i < m_nodeCount; i++) {
      m_nodes[i].init(&(m_def->nodes[i]));
    }
  }
}

LunaNode* LunaScreen::getNode(uint8_t index) {
  if (index < m_nodeCount) {
    return &m_nodes[index];
  }
  return nullptr;
}

const LunaNode* LunaScreen::getNode(uint8_t index) const {
  if (index < m_nodeCount) {
    return &m_nodes[index];
  }
  return nullptr;
}

LunaNode* LunaScreen::findNodeAt(int16_t px, int16_t py) {
  // Hit test in reverse order (topmost node first)
  for (int16_t i = (int16_t)m_nodeCount - 1; i >= 0; i--) {
    if (m_nodes[i].containsPoint(px, py)) {
      return &m_nodes[i];
    }
  }
  return nullptr;
}

LunaNode* LunaScreen::findNodeById(const char* id) {
  if (!id) return nullptr;
  for (uint8_t i = 0; i < m_nodeCount; i++) {
    if (strcmp(m_nodes[i].getId(), id) == 0) {
      return &m_nodes[i];
    }
  }
  return nullptr;
}

void LunaScreen::update(uint32_t currentMillis) {
  for (uint8_t i = 0; i < m_nodeCount; i++) {
    m_nodes[i].update(currentMillis);
  }
}

bool LunaScreen::hasActiveAnimation() const {
  for (uint8_t i = 0; i < m_nodeCount; i++) {
    if (m_nodes[i].hasAnimation()) {
      return true;
    }
  }
  return false;
}
