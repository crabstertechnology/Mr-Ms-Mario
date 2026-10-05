#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "config.h"

struct Note {
  uint16_t frequency;
  uint16_t duration;
};

class LunaAudio {
private:
  int buzzerPin;
  Note noteQueue[48];
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
    digitalWrite(buzzerPin, LOW);
  }

  void enqueueNote(uint16_t freq, uint16_t dur) {
    if (queueCount >= 48) return; // Queue full
    
    noteQueue[queueTail].frequency = freq;
    noteQueue[queueTail].duration = dur;
    queueTail = (queueTail + 1) % 48;
    queueCount++;
  }

public:
  enum AudioMode {
    AUDIO_MODE_SYNTH = 0,
    AUDIO_MODE_STREAM
  };
  
  volatile AudioMode audioMode;
  volatile bool micStreaming;
  volatile bool prebuffering;
  volatile bool directLoopback;
  volatile bool silentMode;

  LunaAudio() : buzzerPin(BUZZER_PIN) {
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    isPlaying = false;
    currentNoteEndTime = 0;
    interNoteGapDuration = 15; // 15ms gap between notes
    inGap = false;
    audioMode = AUDIO_MODE_SYNTH;
    micStreaming = false;
    prebuffering = true;
    directLoopback = false;
    silentMode = false;
  }

  void begin() {
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
  }

  void update() {
    if (!isPlaying) return;

    unsigned long now = millis();
    if (now >= currentNoteEndTime) {
      if (!inGap) {
        noTone(buzzerPin);
        digitalWrite(buzzerPin, LOW);
        currentNoteEndTime = now + interNoteGapDuration;
        inGap = true;
      } else {
        playNextNote();
      }
    }
  }

  void playSound(SoundEffect effect, int speedPercent = 100) {
    if (silentMode) return;
    clearQueue();
    
    interNoteGapDuration = (15 * 100) / speedPercent;
    if (interNoteGapDuration < 1) interNoteGapDuration = 1;
    
    switch (effect) {
      case SOUND_JUMP:
        for (uint16_t f = 200; f < 1100; f += 60) {
          enqueueNote(f, 15);
        }
        break;

      case SOUND_COIN:
        enqueueNote(988, 80);   // B5
        enqueueNote(0, 10);     // Pause
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
        // Legacy startup — same as boot chime for backwards compatibility
        auto eq = [this, speedPercent](uint16_t freq, uint16_t dur) {
          enqueueNote(freq, (dur * 100) / speedPercent);
        };
        eq(523, 80); eq(0, 20);
        eq(659, 80); eq(0, 20);
        eq(784, 80); eq(0, 20);
        eq(1047, 200);
        break;
      }

      case SOUND_BOOT_CHIME: {
        // Clean, cinematic power-on chime — soft ascending tri-tone with a sparkle
        auto eq = [this, speedPercent](uint16_t freq, uint16_t dur) {
          enqueueNote(freq, (dur * 100) / speedPercent);
        };
        // Low warm pad — two soft base tones
        eq(330, 60); eq(0, 15);
        eq(415, 60); eq(0, 15);
        // Mid rise — smooth step up
        eq(523, 70); eq(0, 15);
        eq(622, 70); eq(0, 20);
        // High sparkle — bright finishing arpeggio
        eq(784, 55); eq(0, 10);
        eq(988, 55); eq(0, 10);
        eq(1175, 160);           // Long sustain on top note
        eq(0, 30);
        eq(988, 90);             // Gentle decay echo
        break;
      }

      case SOUND_ANIM_IDLE: {
        // 0: Luna Idle — cheerful, peaceful robot presence chime (C6 -> E6 -> G6 -> C7 sparkle)
        enqueueNote(1047, 40); enqueueNote(0, 8);
        enqueueNote(1319, 40); enqueueNote(0, 8);
        enqueueNote(1568, 50); enqueueNote(0, 10);
        enqueueNote(2093, 80);
        break;
      }

      case SOUND_ANIM_ANGRY: {
        // 1: Angry Face — fast dissonant robot growl buzz
        enqueueNote(880, 30);
        enqueueNote(831, 30);
        enqueueNote(784, 35);
        enqueueNote(698, 40);
        enqueueNote(880, 30);
        enqueueNote(659, 70);
        break;
      }

      case SOUND_ANIM_HUNGRY_MENU: {
        // 2: Hungry Menu — bright alert chirp
        enqueueNote(1047, 45); enqueueNote(0, 10);
        enqueueNote(1319, 45); enqueueNote(0, 10);
        enqueueNote(1760, 80);
        break;
      }

      case SOUND_ANIM_GETTING_HUNGRY: {
        // 3: Getting Hungry — whining tummy descent
        enqueueNote(1319, 35); enqueueNote(0, 8);
        enqueueNote(1175, 35); enqueueNote(0, 8);
        enqueueNote(988, 45);  enqueueNote(0, 8);
        enqueueNote(880, 70);
        break;
      }

      case SOUND_ANIM_EAT_FISH: {
        // 4: Eat Fish — happy staccato munching chirps
        enqueueNote(1319, 35); enqueueNote(0, 10);
        enqueueNote(1568, 35); enqueueNote(0, 10);
        enqueueNote(1760, 40); enqueueNote(0, 10);
        enqueueNote(2093, 80);
        break;
      }

      case SOUND_ANIM_DRINK_MILK: {
        // 5: Drink Milk — sweet smooth sipping glide
        enqueueNote(1047, 35); enqueueNote(0, 8);
        enqueueNote(1319, 35); enqueueNote(0, 8);
        enqueueNote(1568, 40); enqueueNote(0, 8);
        enqueueNote(1976, 50); enqueueNote(0, 8);
        enqueueNote(2349, 90);
        break;
      }

      case SOUND_ANIM_EAT_SALAD: {
        // 6: Eat Salad — crisp crunch crunch ascending
        enqueueNote(1568, 35); enqueueNote(0, 10);
        enqueueNote(1976, 40); enqueueNote(0, 10);
        enqueueNote(2349, 80);
        break;
      }

      case SOUND_ANIM_GETTING_SICK: {
        // 7: Getting Sick — dramatic uneasy warbling descent
        enqueueNote(1760, 45); enqueueNote(0, 8);
        enqueueNote(1568, 45); enqueueNote(0, 8);
        enqueueNote(1397, 50); enqueueNote(0, 8);
        enqueueNote(1175, 50); enqueueNote(0, 8);
        enqueueNote(988, 70);  enqueueNote(0, 10);
        enqueueNote(880, 100);
        break;
      }

      case SOUND_ANIM_SICK: {
        // 8: Luna Sick — pitiful sigh in audible resonance range
        enqueueNote(1047, 50); enqueueNote(0, 15);
        enqueueNote(988, 60);  enqueueNote(0, 15);
        enqueueNote(880, 110);
        break;
      }

      case SOUND_ANIM_RECOVERED: {
        // 9: Recovered — triumphant revival fanfare
        enqueueNote(880, 40);  enqueueNote(0, 8);
        enqueueNote(1047, 40); enqueueNote(0, 8);
        enqueueNote(1319, 40); enqueueNote(0, 8);
        enqueueNote(1760, 50); enqueueNote(0, 8);
        enqueueNote(2093, 60); enqueueNote(0, 10);
        enqueueNote(2637, 120);
        break;
      }

      case SOUND_ANIM_SLEEP: {
        // 10: Going to Sleep — gentle descending lullaby
        enqueueNote(1568, 50); enqueueNote(0, 12);
        enqueueNote(1319, 50); enqueueNote(0, 12);
        enqueueNote(1047, 60); enqueueNote(0, 12);
        enqueueNote(880, 100);
        break;
      }

      case SOUND_ANIM_SLEEPING: {
        // 11: Sleeping — gentle rhythmic slumber breath blip
        enqueueNote(988, 40); enqueueNote(0, 15);
        enqueueNote(1175, 60);
        break;
      }

      case SOUND_ANIM_WAKEUP: {
        // 12: Waking Up — bright ascending wake chime
        enqueueNote(1047, 40); enqueueNote(0, 8);
        enqueueNote(1319, 40); enqueueNote(0, 8);
        enqueueNote(1568, 40); enqueueNote(0, 8);
        enqueueNote(2093, 60); enqueueNote(0, 10);
        enqueueNote(2637, 100);
        break;
      }

      case SOUND_ANIM_THINKING: {
        // 13: Luna Thinking — sparkling inspirational "Aha!" arpeggio
        enqueueNote(1319, 40); enqueueNote(0, 8);
        enqueueNote(1568, 40); enqueueNote(0, 8);
        enqueueNote(1976, 45); enqueueNote(0, 10);
        enqueueNote(2349, 85);
        break;
      }

      case SOUND_CASTLE: {
        auto eq = [this, speedPercent](uint16_t freq, uint16_t dur) {
          enqueueNote(freq, (dur * 100) / speedPercent);
        };
        eq(740, 80); eq(0, 20);
        eq(698, 80); eq(0, 20);
        eq(622, 80); eq(0, 20);
        eq(587, 80); eq(0, 20);
        eq(740, 80); eq(0, 20);
        eq(698, 80); eq(0, 20);
        eq(622, 80); eq(0, 20);
        eq(587, 160);
        break;
      }

      case SOUND_UNDERWORLD: {
        auto eq = [this, speedPercent](uint16_t freq, uint16_t dur) {
          enqueueNote(freq, (dur * 100) / speedPercent);
        };
        eq(131, 80); eq(0, 40);
        eq(262, 80); eq(0, 40);
        eq(110, 80); eq(0, 40);
        eq(220, 80); eq(0, 40);
        eq(117, 80); eq(0, 40);
        eq(233, 80); eq(0, 40);
        break;
      }

      case SOUND_THEMECHANGE:
        enqueueNote(523, 60);
        enqueueNote(0, 10);
        enqueueNote(659, 60);
        enqueueNote(0, 10);
        enqueueNote(784, 80);
        break;

      case SOUND_ALERT_BEEP:
        // Insistent warning alert: rapid high-urgency pulsed beeps
        enqueueNote(1760, 80);
        enqueueNote(0, 40);
        enqueueNote(1760, 80);
        enqueueNote(0, 40);
        enqueueNote(2349, 120);
        enqueueNote(0, 50);
        enqueueNote(2349, 200);
        break;
        
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
      digitalWrite(buzzerPin, LOW);
      return;
    }

    Note note = noteQueue[queueHead];
    queueHead = (queueHead + 1) % 48;
    queueCount--;

    if (note.frequency > 0) {
      tone(buzzerPin, note.frequency, note.duration);
    } else {
      noTone(buzzerPin);
    }
    
    currentNoteEndTime = millis() + note.duration;
    inGap = false;
  }

  void setVolume(int vol) {}
  void setBassBoost(int level) {}
  void startMusicStream() {}
  void stopMusicStream() {}
  void writeTxStream(const uint8_t* data, size_t len) {}
  bool getRxItem(uint8_t* buffer, size_t* size) {
    return false;
  }
};

#endif // AUDIO_H
