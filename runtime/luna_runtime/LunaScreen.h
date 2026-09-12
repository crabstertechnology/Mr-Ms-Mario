#ifndef LUNA_SCREEN_H
#define LUNA_SCREEN_H

#include "LunaTypes.h"
#include "LunaNode.h"

#define LUNA_MAX_NODES_PER_SCREEN 16

class LunaScreen {
public:
  LunaScreen();
  void init(const LunaScreenDef* def);

  const char* getId() const { return m_def ? m_def->id : ""; }
  const char* getName() const { return m_def ? m_def->name : ""; }
  uint16_t getBackgroundColor() const { return m_def ? m_def->backgroundColor : LUNA_COLOR_BLACK; }
  const char* getBackgroundPattern() const { return m_def ? m_def->backgroundPattern : nullptr; }
  bool isScrollable() const { return m_def ? m_def->isScrollable : false; }
  int16_t getMaxScrollY() const { return m_def ? m_def->maxScrollY : 280; }

  const char* getSwipeLeftTarget() const { return m_def ? m_def->swipeLeftTarget : nullptr; }
  const char* getSwipeRightTarget() const { return m_def ? m_def->swipeRightTarget : nullptr; }

  uint8_t getNodeCount() const { return m_nodeCount; }
  LunaNode* getNode(uint8_t index);
  const LunaNode* getNode(uint8_t index) const;

  LunaNode* findNodeAt(int16_t px, int16_t py);
  LunaNode* findNodeById(const char* id);

  void update(uint32_t currentMillis);
  bool hasActiveAnimation() const;

private:
  const LunaScreenDef* m_def;
  LunaNode m_nodes[LUNA_MAX_NODES_PER_SCREEN];
  uint8_t m_nodeCount;
};

#endif // LUNA_SCREEN_H
