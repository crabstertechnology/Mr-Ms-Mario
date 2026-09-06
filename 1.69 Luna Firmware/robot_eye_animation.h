#ifndef ROBOT_EYE_ANIMATION_H
#define ROBOT_EYE_ANIMATION_H

#include <Arduino.h>
#include <pgmspace.h>
#include "sprite_ai_data.h"

enum RobotEyeState {
  ROBOT_EYE_IDLE = 0,
  ROBOT_EYE_BLINK,
  ROBOT_EYE_LOOK_LEFT,
  ROBOT_EYE_LOOK_RIGHT,
  ROBOT_EYE_HAPPY,
  ROBOT_EYE_SURPRISED,
  ROBOT_EYE_SLEEPY
};

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

public:
  RobotEyeAnimation() 
    : state(ROBOT_EYE_IDLE), animIndex(0), currentFrame(0), frameCount(SPRITE_AI_FRAME_COUNT), 
      frameDelayMs(125), lastFrameTime(0), playing(true), fps(8.0f) {}

  void play() {
    playing = true;
    lastFrameTime = millis();
  }

  void stop() {
    playing = false;
  }

  void reset() {
    currentFrame = 0;
    lastFrameTime = millis();
  }

  void nextAnimation() {
    animIndex = (animIndex + 1) % SPRITE_AI_ANIMATION_COUNT;
    currentFrame = 0;
    lastFrameTime = millis();
  }

  void setAnimationIndex(int idx) {
    if (idx >= 0 && idx < SPRITE_AI_ANIMATION_COUNT) {
      animIndex = idx;
      currentFrame = 0;
      lastFrameTime = millis();
    }
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
    if (state != newState) {
      state = newState;
      currentFrame = 0;
      lastFrameTime = millis();
    }
  }

  RobotEyeState getState() const {
    return state;
  }

  bool update() {
    if (!playing || frameCount <= 0) return false;

    unsigned long now = millis();
    if (now - lastFrameTime >= (unsigned long)frameDelayMs) {
      lastFrameTime = now;
      currentFrame = (currentFrame + 1) % frameCount;
      return true;
    }
    return false;
  }

  const uint16_t* getCurrentFrameData() const {
    return getSpriteAiFrame(animIndex, currentFrame);
  }

  int getCurrentFrame() const { return currentFrame; }
  int getFrameCount() const { return frameCount; }
  int getWidth() const { return SPRITE_AI_FRAME_WIDTH; }
  int getHeight() const { return SPRITE_AI_FRAME_HEIGHT; }
  int getXOffset() const { return 0; }
  int getYOffset() const { return 20; } // Center 240x240 inside 240x280 display (top: 20px, bottom: 20px)
};

#endif // ROBOT_EYE_ANIMATION_H
