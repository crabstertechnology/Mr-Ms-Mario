#ifndef ROBOT_EYE_ANIMATION_H
#define ROBOT_EYE_ANIMATION_H

#include <Arduino.h>
#include <pgmspace.h>
#include <Adafruit_GFX.h>
#include <TJpg_Decoder.h>
#include "video_frames_data.h"

#define SPRITE_AI_ANIMATION_COUNT TOTAL_ANIMATIONS

enum RobotEyeState {
  ROBOT_EYE_IDLE = 0,
  ROBOT_EYE_BLINK,
  ROBOT_EYE_LOOK_LEFT,
  ROBOT_EYE_LOOK_RIGHT,
  ROBOT_EYE_HAPPY,
  ROBOT_EYE_SURPRISED,
  ROBOT_EYE_SLEEPY
};

// Global target canvas pointer for high-performance direct blitting into PSRAM
static GFXcanvas16* _videoDecTargetCanvas = nullptr;

static bool _videoTftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (!_videoDecTargetCanvas) return false;
  uint16_t* dest = _videoDecTargetCanvas->getBuffer();
  if (!dest) return false;

  // Ultra-fast path: block fully within canvas boundary (covers 95%+ of all MCUs)
  // Eliminates per-row bounds checks, multiplications, and branches!
  if (x >= 0 && (x + w) <= VIDEO_FRAME_WIDTH && y >= 0 && (y + h) <= VIDEO_FRAME_HEIGHT) {
    uint16_t* dstRow = &dest[y * VIDEO_FRAME_WIDTH + x];
    const uint16_t* srcRow = bitmap;
    for (int16_t r = 0; r < h; r++) {
      memcpy(dstRow, srcRow, w * sizeof(uint16_t));
      dstRow += VIDEO_FRAME_WIDTH;
      srcRow += w;
    }
    return true;
  }

  // Clipped path for boundary blocks
  for (int16_t r = 0; r < h; r++) {
    int16_t cy = y + r;
    if (cy >= VIDEO_FRAME_HEIGHT) break;
    if (cy < 0) continue;
    int16_t cx = (x < 0) ? 0 : x;
    int16_t cw = w;
    if (cx + cw > VIDEO_FRAME_WIDTH) cw = VIDEO_FRAME_WIDTH - cx;
    if (cw > 0) {
      memcpy(&dest[cy * VIDEO_FRAME_WIDTH + cx], &bitmap[r * w], cw * sizeof(uint16_t));
    }
  }
  return true;
}

class RobotEyeAnimation {
private:
  RobotEyeState state;
  int animIndex;
  int currentFrame;
  int frameCount;
  int frameDelayMs;
  unsigned long lastFrameTime;
  bool playing;
  float fps;
  int loopCount;
  bool cycleCompleted;
  bool needsRedraw;

public:
  RobotEyeAnimation() 
    : state(ROBOT_EYE_IDLE), animIndex(0), currentFrame(0), 
      frameCount(ANIM_0_FRAME_COUNT), frameDelayMs(100), 
      lastFrameTime(0), playing(true), fps(10.0f), 
      loopCount(0), cycleCompleted(false), needsRedraw(true) {}

  void play() {
    playing = true;
    lastFrameTime = millis();
    needsRedraw = true;
  }

  void stop() {
    playing = false;
  }

  void pause() {
    playing = false;
  }

  void resume() {
    if (!playing) {
      playing = true;
      lastFrameTime = millis();
      needsRedraw = true;
    }
  }

  void reset() {
    currentFrame = 0;
    loopCount = 0;
    cycleCompleted = false;
    lastFrameTime = millis();
    needsRedraw = true;
  }

  void nextAnimation() {
    int nextIdx = (animIndex + 1) % TOTAL_ANIMATIONS;
    setAnimationIndex(nextIdx);
  }

  void setAnimationIndex(int idx) {
    if (idx < 0 || idx >= TOTAL_ANIMATIONS) idx = 0;
    animIndex = idx;
    frameCount = anim_frame_counts[animIndex];
    fps = 10.0f;
    frameDelayMs = 100; // 100ms (10 FPS): natural, smooth, relaxed playback
    currentFrame = 0;
    loopCount = 0;
    cycleCompleted = false;
    lastFrameTime = millis();
    playing = true;
    needsRedraw = true;
  }

  void requestRedraw() {
    needsRedraw = true;
  }

  int getAnimationIndex() const {
    return animIndex;
  }

  void setFrame(int frame) {
    if (frame >= 0 && frame < frameCount) {
      currentFrame = frame;
      lastFrameTime = millis();
      needsRedraw = true;
    }
  }

  void setFPS(float newFps) {
    if (newFps > 0.1f) {
      fps = newFps;
      frameDelayMs = (int)(1000.0f / fps);
    }
  }

  void setFrameDelay(int ms) {
    if (ms > 10) {
      frameDelayMs = ms;
      fps = 1000.0f / ms;
    }
  }

  bool isPlaying() const {
    return playing;
  }

  void setState(RobotEyeState newState) {
    state = newState;
  }

  RobotEyeState getState() const {
    return state;
  }

  bool update() {
    if (!playing || frameCount <= 0) return false;

    // Immediate draw request (e.g. anim switch, reset, wake-up)
    if (needsRedraw) {
      needsRedraw = false;
      return true;
    }

    unsigned long now = millis();
    if (now - lastFrameTime >= (unsigned long)frameDelayMs) {
      // Advance strictly 1 frame per tick — NEVER drop or skip video frames!
      currentFrame++;

      // Prevent phase drift while keeping cadence locked
      if (now - lastFrameTime >= (unsigned long)(frameDelayMs * 2)) {
        lastFrameTime = now;
      } else {
        lastFrameTime += (unsigned long)frameDelayMs;
      }

      if (currentFrame >= frameCount) {
        // For one-shot transitional animations (10: Going to Sleep, 12: Waking Up, 7: Idle to Sick, 9: Sick to Idle, 1: Angry, 13: Thinking),
        // clamp to last frame so it doesn't wrap to frame 0 and flash before state machine transitions!
        if (animIndex == 10 || animIndex == 12 || animIndex == 7 || animIndex == 9 || animIndex == 1 || animIndex == 13) {
          currentFrame = frameCount - 1;
        } else {
          currentFrame = 0;
        }
        loopCount++;
        cycleCompleted = true;
      }
      return true;
    }
    return false;
  }

  bool isCycleCompleted() const { return cycleCompleted; }
  void clearCycleCompleted() { cycleCompleted = false; }
  int getLoopCount() const { return loopCount; }

  // Hardware-optimized direct JPEG blit into display PSRAM canvas
  void draw(GFXcanvas16& canvas) {
    if (frameCount <= 0) return;
    _videoDecTargetCanvas = &canvas;
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(false);
    TJpgDec.setCallback(_videoTftOutput);

    const uint8_t* const* curFrames = anim_frame_pointers[animIndex];
    const uint32_t* curSizes = anim_size_pointers[animIndex];
    if (curFrames != nullptr && curSizes != nullptr) {
      const uint8_t* fData = curFrames[currentFrame];
      uint32_t fSize = curSizes[currentFrame];
      if (fData != nullptr && fSize > 0) {
        TJpgDec.drawJpg(0, 0, fData, fSize);
      }
    }
  }

  const uint16_t* getCurrentFrameData() const {
    return nullptr;
  }

  int getCurrentFrame() const { return currentFrame; }
  int getFrameCount() const { return frameCount; }
  int getWidth() const { return VIDEO_FRAME_WIDTH; }
  int getHeight() const { return VIDEO_FRAME_HEIGHT; }
  int getXOffset() const { return 0; }
  int getYOffset() const { return 0; }
};

#endif // ROBOT_EYE_ANIMATION_H
