#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "config.h"

struct Note {
  uint16_t frequency;
  uint16_t duration;
};

class MarioAudio {
private:
  int buzzerPin;
  Note noteQueue[100];
  int queueHead;
  int queueTail;
  int queueCount;
  
  bool isPlaying;
  unsigned long currentNoteEndTime;
  unsigned long interNoteGapDuration;
  bool inGap;

  void clearQueue() {
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    isPlaying = false;
    inGap = false;
    noTone(buzzerPin);
  }

  void enqueueNote(uint16_t freq, uint16_t dur) {
    if (queueCount >= 100) return; // Queue full
    
    noteQueue[queueTail].frequency = freq;
    noteQueue[queueTail].duration = dur;
    queueTail = (queueTail + 1) % 100;
    queueCount++;
  }

public:
  MarioAudio(int pin) : buzzerPin(pin) {
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    isPlaying = false;
    currentNoteEndTime = 0;
    interNoteGapDuration = 15; // 15ms gap between notes
    inGap = false;
    pinMode(buzzerPin, OUTPUT);
  }

  void playSound(SoundEffect effect, int speedPercent = 100) {
    clearQueue();
    
    // Scale the gap between notes based on speed
    interNoteGapDuration = (15 * 100) / speedPercent;
    if (interNoteGapDuration < 1) interNoteGapDuration = 1;
    
    switch (effect) {
      case SOUND_JUMP:
        // Sliding frequencies from 200Hz to 1200Hz
        for (uint16_t f = 200; f < 1100; f += 60) {
          enqueueNote(f, 15);
        }
        break;

      case SOUND_COIN:
        enqueueNote(988, 80);   // B5
        enqueueNote(0, 10);     // Brief pause
        enqueueNote(1319, 280);  // E6
        break;

      case SOUND_POWERUP:
        enqueueNote(330, 70);   // E5
        enqueueNote(392, 70);   // G5
        enqueueNote(659, 70);   // E6
        enqueueNote(523, 70);   // C6
        enqueueNote(587, 70);   // D6
        enqueueNote(784, 70);   // G6
        break;

      case SOUND_POWERDOWN:
        enqueueNote(784, 70);   // G6
        enqueueNote(587, 70);   // D6
        enqueueNote(523, 70);   // C6
        enqueueNote(659, 70);   // E6
        enqueueNote(392, 70);   // G5
        enqueueNote(330, 70);   // E5
        break;

      case SOUND_GAMEOVER:
        enqueueNote(523, 150);  // C5
        enqueueNote(392, 150);  // G4
        enqueueNote(330, 150);  // E4
        enqueueNote(440, 120);  // A4
        enqueueNote(494, 120);  // B4
        enqueueNote(440, 120);  // A4
        enqueueNote(415, 120);  // Ab4
        enqueueNote(466, 120);  // Bb4
        enqueueNote(415, 120);  // Ab4
        enqueueNote(392, 250);  // G4
        break;

      case SOUND_CHIRP:
        enqueueNote(880, 40);   // A5
        enqueueNote(0, 20);
        enqueueNote(1200, 50);  // D6
        break;

      case SOUND_STARTUP: {
        // Classic Mario overworld theme melody sequence scaled by speedPercent
        auto eq = [this, speedPercent](uint16_t freq, uint16_t dur) {
          enqueueNote(freq, (dur * 100) / speedPercent);
        };
        eq(659, 50); eq(0, 10);   // E5 50 10
        eq(659, 50); eq(0, 10);   // E5 50 10
        eq(659, 50); eq(0, 30);   // E5 50 30
        eq(523, 50); eq(0, 10);   // C5 50 10
        eq(659, 50); eq(0, 30);   // E5 50 30
        eq(784, 50); eq(0, 50);   // G5 50 50
        eq(392, 50); eq(0, 50);   // G4 50 50
        eq(523, 50); eq(0, 10);   // C5 50 10
        eq(392, 50); eq(0, 30);   // G4 50 30
        eq(330, 50); eq(0, 10);   // E4 50 10
        eq(440, 50); eq(0, 10);   // A4 50 10
        eq(494, 50); eq(0, 10);   // B4 50 10
        eq(466, 50); eq(0, 10);   // AS4 50 10
        eq(440, 50); eq(0, 30);   // A4 50 30
        eq(392, 50); eq(0, 20);   // G4 50 20
        eq(659, 50); eq(0, 10);   // E5 50 10
        eq(784, 50); eq(0, 10);   // G5 50 10
        eq(880, 50); eq(0, 10);   // A5 50 10
        eq(698, 50); eq(0, 10);   // F5 50 10
        eq(784, 50); eq(0, 10);   // G5 50 10
        eq(659, 50); eq(0, 10);   // E5 50 10
        eq(523, 50); eq(0, 10);   // C5 50 10
        eq(587, 50); eq(0, 10);   // D5 50 10
        eq(494, 50); eq(0, 50);   // B4 50 50
        break;
      }
        
      default:
        break;
    }

    if (queueCount > 0) {
      isPlaying = true;
      playNextNote();
    }
  }

  void playNextNote() {
    if (queueCount == 0) {
      isPlaying = false;
      noTone(buzzerPin);
      return;
    }

    Note note = noteQueue[queueHead];
    queueHead = (queueHead + 1) % 100;
    queueCount--;

    if (note.frequency > 0) {
      tone(buzzerPin, note.frequency, note.duration);
    } else {
      noTone(buzzerPin);
    }
    
    currentNoteEndTime = millis() + note.duration;
    inGap = false;
  }

  void update() {
    if (!isPlaying) return;

    unsigned long now = millis();
    if (now >= currentNoteEndTime) {
      if (!inGap) {
        // Stop playing note and start a brief silent gap for note separation
        noTone(buzzerPin);
        currentNoteEndTime = now + interNoteGapDuration;
        inGap = true;
      } else {
        // Gap finished, play next note
        playNextNote();
      }
    }
  }
};

#endif // AUDIO_H
