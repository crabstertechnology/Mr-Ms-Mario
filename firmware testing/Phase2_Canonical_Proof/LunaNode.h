#ifndef LUNA_NODE_H
#define LUNA_NODE_H

#include "LunaTypes.h"

class LunaNode {
public:
  LunaNode();
  void init(const LunaNodeDef* def);

  const char* getId() const { return m_def ? m_def->id : ""; }
  const char* getType() const { return m_def ? m_def->type : ""; }
  uint8_t getNodeType() const { return m_def ? m_def->nodeType : LUNA_NODE_CONTAINER; }
  const LunaRect& getLayout() const { return m_def->layout; }
  const LunaStyle& getStyle() const;
  const LunaTypography& getTypography() const { return m_def->typography; }
  const char* getText() const { return m_def ? m_def->text : nullptr; }
  const char* getSubtitle() const { return m_def ? m_def->subtitle : nullptr; }
  uint16_t getSubtextColor() const { return m_def ? m_def->subtextColor : 0; }
  uint8_t getSpinnerSize() const { return m_def ? m_def->spinnerSize : 30; }
  const char* getAssetId() const { return m_def ? m_def->assetId : nullptr; }
  const LunaFont* getFont() const { return m_def ? m_def->font : nullptr; }


  // State Management
  uint8_t getState() const { return m_state; }
  void setState(uint8_t state);

  // Interaction
  bool containsPoint(int16_t px, int16_t py) const;
  const LunaAction* getActionForTrigger(const char* trigger) const;

  // Animation Update
  void update(uint32_t currentMillis);
  int16_t getAnimatedRotation() const { return m_animatedRotation; }
  bool hasAnimation() const { return m_def && m_def->animationCount > 0; }

private:
  const LunaNodeDef* m_def;
  uint8_t m_state;
  uint32_t m_animStartMillis;
  int16_t m_animatedRotation;
};

#endif // LUNA_NODE_H
