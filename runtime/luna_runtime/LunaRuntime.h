#ifndef LUNA_RUNTIME_H
#define LUNA_RUNTIME_H

#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "LunaTypes.h"
#include "LunaScreen.h"
#include "LunaRenderer.h"
#include "LunaInput.h"

#define LUNA_MAX_SCREENS 8

class LunaRuntime {
public:
  LunaRuntime();

  // Initialization
  bool begin(const LunaProjectDef* projectDef, Arduino_Canvas* canvas, uint16_t width = 240, uint16_t height = 280);

  // Standard Render / Update Loop Tick
  void updateInput();
  void updateRuntime();
  void renderIfNeeded();

  // Navigation
  bool navigateTo(const char* targetScreenId);
  const char* getActiveScreenId() const;
  LunaScreen* getActiveScreen() { return m_activeScreen; }

  // State & Flags
  void requestRedraw() { m_dirty = true; }
  bool isDirty() const { return m_dirty; }

  // Telemetry
  uint32_t getFreeInternalHeap() const;
  uint32_t getFreePsram() const;
  uint32_t getLastRenderDurationUs() const { return m_renderer.getLastRenderDurationUs(); }
  float getMeasuredFps() const { return m_renderer.getMeasuredFps(); }

  LunaRenderer& getRenderer() { return m_renderer; }
  LunaInput& getInput() { return m_input; }

private:
  const LunaProjectDef* m_projectDef;
  LunaScreen m_screens[LUNA_MAX_SCREENS];
  uint8_t m_screenCount;
  LunaScreen* m_activeScreen;

  LunaRenderer m_renderer;
  LunaInput m_input;
  bool m_dirty;

  // Active interaction tracking
  LunaNode* m_pressedNode;

  LunaScreen* findScreenById(const char* screenId);
};

#endif // LUNA_RUNTIME_H
