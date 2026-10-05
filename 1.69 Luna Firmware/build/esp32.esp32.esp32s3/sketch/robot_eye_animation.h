#line 1 "W:\\Mr.mario\\1.69 Luna Firmware\\robot_eye_animation.h"
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

public:
  RobotEyeAnimation() 
    : state(ROBOT_EYE_IDLE), animIndex(0), currentFrame(0), 
      frameCount(ANIM_0_FRAME_COUNT), frameDelayMs(80), 
      lastFrameTime(0), playing(true), fps(12.5f), 
      loopCount(0), cycleCompleted(false) {}

  void play() {
    playing = true;
    lastFrameTime = millis();
  }

  void stop() {
    playing = false;
  }

  void reset() {
    currentFrame = 0;
    loopCount = 0;
    cycleCompleted = false;
    lastFrameTime = millis();
  }

  void nextAnimation() {
    int nextIdx = (animIndex + 1) % TOTAL_ANIMATIONS;
    setAnimationIndex(nextIdx);
  }

  void setAnimationIndex(int idx) {
    if (idx < 0 || idx >= TOTAL_ANIMATIONS) idx = 0;
    animIndex = idx;
    frameCount = anim_frame_counts[animIndex];
    fps = 12.5f;
    frameDelayMs = 80; // 80ms (~12.5 FPS): fluid, responsive, zero delay
    currentFrame = 0;
    loopCount = 0;
    cycleCompleted = false;
    lastFrameTime = millis();
    playing = true;
  }

  int getAnimationIndex() const {
    return animIndex;
  }

  void setFrame(int frame) {
    if (frame >= 0 && frame < frameCount) {
      currentFrame = frame;
      lastFrameTime = millis();
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

    unsigned long now = millis();
    if (now - lastFrameTime >= (unsigned long)frameDelayMs) {
      int framesToAdvance = (now - lastFrameTime) / frameDelayMs;
      if (framesToAdvance < 1) framesToAdvance = 1;
      lastFrameTime += (unsigned long)(framesToAdvance * frameDelayMs);
      currentFrame += framesToAdvance;
      if (currentFrame >= frameCount) {
        currentFrame = currentFrame % frameCount;
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
