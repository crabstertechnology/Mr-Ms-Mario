#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "driver/i2s.h"
#include <math.h>
#include "config.h"
#include "freertos/ringbuf.h"

struct Note {
  uint16_t frequency;
  uint16_t duration;
};

// Global mic amplitude accessible by the rendering display
extern volatile int micAmplitude;

class LunaAudio {
private:
  Note noteQueue[100];
  int queueHead;
  int queueTail;
  int queueCount;
  
  bool isPlaying;
  unsigned long currentNoteEndTime;
  unsigned long interNoteGapDuration;
  bool inGap;

  TaskHandle_t audioTxTaskHandle;
  TaskHandle_t audioRxTaskHandle;

public:
  enum AudioMode {
    AUDIO_MODE_SYNTH = 0,
    AUDIO_MODE_STREAM
  };
  volatile AudioMode audioMode;
  volatile bool micStreaming;
  volatile bool prebuffering;
  RingbufHandle_t txRingBuffer;
  RingbufHandle_t rxRingBuffer;

private:
  void clearQueue() {
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    isPlaying = false;
    inGap = false;
    setTone(0);
  }

  void enqueueNote(uint16_t freq, uint16_t dur) {
    if (queueCount >= 100) return; // Queue full
    
    noteQueue[queueTail].frequency = freq;
    noteQueue[queueTail].duration = dur;
    queueTail = (queueTail + 1) % 100;
    queueCount++;
  }

public:
  volatile int currentFrequency;
  volatile int currentVolume;  // Synth volume (0-32767)
  volatile int _streamVolume;  // Stream volume 0-100
  volatile int _bassBoost;     // Bass boost 0-10
  float _lpfState;             // Low-pass filter state for bass shelf EQ

  LunaAudio() : currentFrequency(0), currentVolume(8000),
                _streamVolume(70), _bassBoost(3), _lpfState(0.0f) {
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    isPlaying = false;
    currentNoteEndTime = 0;
    interNoteGapDuration = 15; // 15ms gap between notes
    inGap = false;
    audioTxTaskHandle = NULL;
    audioRxTaskHandle = NULL;
    txRingBuffer = NULL;
    rxRingBuffer = NULL;
    audioMode = AUDIO_MODE_SYNTH;
    micStreaming = false;
    prebuffering = true;
  }

  /// Set speaker output volume (0 = mute, 100 = full scale)
  void setVolume(int vol) {
    _streamVolume = constrain(vol, 0, 100);
  }

  /// Set bass boost level (0 = flat, 10 = heavy bass)
  /// Implemented as a low-shelf blend: output = dry + (boost/10) * lowpass
  void setBassBoost(int level) {
    _bassBoost = constrain(level, 0, 10);
  }

  void writeTxStream(const uint8_t* data, size_t len) {
    if (txRingBuffer == NULL) return;
    // NON-BLOCKING write: drop data if buffer full rather than stalling the BLE callback.
    // Stalling the BLE callback blocks the BLE stack causing slow-motion / connection drops.
    xRingbufferSend(txRingBuffer, data, len, 0);
  }

  void startMusicStream() {
    // Flush any stale data from a previous stream
    if (txRingBuffer != NULL) {
      void* item;
      size_t item_size;
      while ((item = xRingbufferReceive(txRingBuffer, &item_size, 0)) != NULL) {
        vRingbufferReturnItem(txRingBuffer, item);
      }
    }
    _lpfState = 0.0f;   // reset bass filter state
    prebuffering = true;
    audioMode = AUDIO_MODE_STREAM;
  }

  void stopMusicStream() {
    audioMode = AUDIO_MODE_SYNTH;
    prebuffering = true;
  }

  bool getRxItem(uint8_t* buffer, size_t* size) {
    if (rxRingBuffer == NULL) return false;
    size_t item_size = 0;
    uint8_t* item = (uint8_t*)xRingbufferReceive(rxRingBuffer, &item_size, 0);
    if (item != NULL) {
      if (item_size > 256) item_size = 256;
      memcpy(buffer, item, item_size);
      *size = item_size;
      vRingbufferReturnItem(rxRingBuffer, (void*)item);
      return true;
    }
    return false;
  }

  void setTone(uint16_t freq) {
    currentFrequency = freq;
  }

  void playSound(SoundEffect effect, int speedPercent = 100) {
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
        auto eq = [this, speedPercent](uint16_t freq, uint16_t dur) {
          enqueueNote(freq, (dur * 100) / speedPercent);
        };
        eq(659, 50); eq(0, 10);
        eq(659, 50); eq(0, 10);
        eq(659, 50); eq(0, 30);
        eq(523, 50); eq(0, 10);
        eq(659, 50); eq(0, 30);
        eq(784, 50); eq(0, 50);
        eq(392, 50); eq(0, 50);
        eq(523, 50); eq(0, 10);
        eq(392, 50); eq(0, 30);
        eq(330, 50); eq(0, 10);
        eq(440, 50); eq(0, 10);
        eq(494, 50); eq(0, 10);
        eq(466, 50); eq(0, 10);
        eq(440, 50); eq(0, 30);
        eq(392, 50); eq(0, 20);
        eq(659, 50); eq(0, 10);
        eq(784, 50); eq(0, 10);
        eq(880, 50); eq(0, 10);
        eq(698, 50); eq(0, 10);
        eq(784, 50); eq(0, 10);
        eq(659, 50); eq(0, 10);
        eq(523, 50); eq(0, 10);
        eq(494, 50); eq(0, 50);
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
      setTone(0);
      return;
    }

    Note note = noteQueue[queueHead];
    queueHead = (queueHead + 1) % 100;
    queueCount--;

    setTone(note.frequency);
    currentNoteEndTime = millis() + note.duration;
    inGap = false;
  }

  void update() {
    if (!isPlaying) return;

    unsigned long now = millis();
    if (now >= currentNoteEndTime) {
      if (!inGap) {
        setTone(0);
        currentNoteEndTime = now + interNoteGapDuration;
        inGap = true;
      } else {
        playNextNote();
      }
    }
  }

  void init() {
    // Ring buffer: 65536 bytes ≈ 2s of 16kHz 16-bit mono PCM
    // Large buffer absorbs BLE scheduling jitter (BLE delivers in bursts, not smoothly)
    txRingBuffer = xRingbufferCreate(16384, RINGBUF_TYPE_BYTEBUF);
    rxRingBuffer = xRingbufferCreate(2048,  RINGBUF_TYPE_BYTEBUF);

    // I2S DMA config — mirroring how Bluetooth speakers work:
    //   dma_buf_count * dma_buf_len * 2 bytes = total DMA memory
    //   256 * 8 * 2 = 4096 bytes ≈ 128ms of audio at 16kHz
    //   Larger DMA buffers = smoother playback under BLE burst delivery
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len  = 256,   // max safe value for ESP32-S3 I2S driver
      .use_apll = true,      // APLL = precise audio PLL clock — eliminates sample rate drift and noise
      .tx_desc_auto_clear = true
    };

    i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCLK,
      .ws_io_num  = I2S_WS,
      .data_out_num = I2S_DOUT,
      .data_in_num  = I2S_DIN
    };

    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err == ESP_OK) {
      i2s_set_pin(I2S_NUM_0, &pin_config);
      Serial.println("[AUDIO] I2S driver OK (dma_buf=256*8).");
    } else {
      Serial.printf("[AUDIO] I2S driver FAILED: %d\n", err);
    }

    xTaskCreatePinnedToCore(txAudioTask, "audio_tx", 4096, this, 5, &audioTxTaskHandle, 1);
    xTaskCreatePinnedToCore(rxAudioTask, "audio_rx", 2048, this, 4, &audioRxTaskHandle, 0);
  }

  static void txAudioTask(void* pvParameters) {
    LunaAudio* self = (LunaAudio*)pvParameters;

    // ── Synth oscillator state ────────────────────────────────────────────────
    int16_t synthBuf[256];   // 256 samples × 2 bytes = 512 bytes per I2S write
    double  phase = 0;

    // ── Silence block for prebuffer/underrun fill ─────────────────────────────
    static const int16_t SILENCE[256] = {};
    size_t bytes_written;

    while (true) {
      // ── STREAM MODE ──────────────────────────────────────────────────────────
      if (self->audioMode == AUDIO_MODE_STREAM) {

        // ── Prebuffer check ───────────────────────────────────────────────────
        // For RINGBUF_TYPE_BYTEBUF: measure filled bytes as (totalSize - freeSize)
        // The ring buffer was created with 16384 bytes.
        if (self->prebuffering && self->txRingBuffer != NULL) {
          size_t freeBytes = xRingbufferGetCurFreeSize(self->txRingBuffer);
          size_t filledBytes = 16384 - freeBytes;
          // Wait for 8192 bytes (~256ms of audio) before starting — absorbs BLE jitter
          if (filledBytes >= 8192) {
            self->prebuffering = false;
            Serial.printf("[AUDIO] Prebuffer done, filled=%u bytes. Starting playback.\n", filledBytes);
          } else {
            // Still buffering: push silence to keep I2S DMA busy
            i2s_write(I2S_NUM_0, SILENCE, sizeof(SILENCE), &bytes_written, portMAX_DELAY);
            continue;
          }
        }

        // ── Pull one chunk from ring buffer and send to I2S ───────────────────
        size_t item_size = 0;
        int16_t* item = (int16_t*)xRingbufferReceive(
            self->txRingBuffer, &item_size, pdMS_TO_TICKS(5));

        if (item != NULL && item_size > 0) {
          // ── Apply bass-shelf EQ + volume in-place ───────────────────────────
          int vol       = self->_streamVolume;  // 0-100
          int bassLevel = self->_bassBoost;     // 0-10
          int samples   = (int)(item_size / 2);

          // Low-shelf: alpha=0.89 → fc≈300Hz at 16kHz
          const float alpha    = 0.89f;
          const float bassGain = (float)bassLevel / 10.0f;

          for (int i = 0; i < samples; i++) {
            int32_t s = (int32_t)item[i];
            if (bassLevel > 0) {
              self->_lpfState = alpha * self->_lpfState + (1.0f - alpha) * (float)s;
              s = (int32_t)((float)s + bassGain * self->_lpfState);
              if (s >  32767) s =  32767;
              if (s < -32768) s = -32768;
            }
            if (vol != 100) s = (s * vol) / 100;
            item[i] = (int16_t)s;
          }

          i2s_write(I2S_NUM_0, item, item_size, &bytes_written, portMAX_DELAY);
          vRingbufferReturnItem(self->txRingBuffer, (void*)item);

        } else {
          // Underrun: output silence to keep I2S clock running
          // (DO NOT re-enable prebuffering — a brief gap is tolerable)
          i2s_write(I2S_NUM_0, SILENCE, sizeof(SILENCE), &bytes_written, portMAX_DELAY);
        }

      // ── SYNTH MODE ───────────────────────────────────────────────────────────
      } else {
        int freq = self->currentFrequency;
        if (freq > 0) {
          for (int i = 0; i < 256; i++) {
            phase += (2.0 * M_PI * freq) / 16000.0;
            if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
            synthBuf[i] = (int16_t)(sin(phase) * self->currentVolume);
          }
          i2s_write(I2S_NUM_0, synthBuf, sizeof(synthBuf), &bytes_written, portMAX_DELAY);
        } else {
          i2s_write(I2S_NUM_0, SILENCE, sizeof(SILENCE), &bytes_written, portMAX_DELAY);
          vTaskDelay(pdMS_TO_TICKS(8));
        }
      }
    }
  }

  static void rxAudioTask(void* pvParameters) {
    LunaAudio* self = (LunaAudio*)pvParameters;
    int16_t read_buffer[128]; // 256 bytes
    while (true) {
      size_t bytes_read;
      esp_err_t err = i2s_read(I2S_NUM_0, read_buffer, sizeof(read_buffer), &bytes_read, portMAX_DELAY);
      if (err == ESP_OK && bytes_read > 0) {
        int32_t sum = 0;
        int count = bytes_read / 2;
        for (int i = 0; i < count; i++) {
          sum += abs(read_buffer[i]);
        }
        if (count > 0) {
          micAmplitude = sum / count;
        }

        // If streaming is enabled, write to rxRingBuffer
        if (self->micStreaming && self->rxRingBuffer != NULL) {
          xRingbufferSend(self->rxRingBuffer, read_buffer, bytes_read, 0); // No wait
        }
      }
      vTaskDelay(pdMS_TO_TICKS(5));
    }
  }
};

#endif // AUDIO_H
