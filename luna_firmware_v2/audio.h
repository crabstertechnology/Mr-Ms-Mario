#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "driver/i2s.h"
#include "driver/gpio.h"
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
  volatile bool directLoopback; // When true, RX task writes mic audio directly to I2S (bypasses ring buffers)
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
    directLoopback = false;
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
    static uint32_t totalBytesReceived = 0;
    static uint32_t totalBytesDropped = 0;
    totalBytesReceived += len;
    
    BaseType_t ret = xRingbufferSend(txRingBuffer, data, len, 0);
    if (ret != pdTRUE) {
      totalBytesDropped += len;
    }
    
    if (totalBytesReceived % 8000 < len) {
      Serial.printf("[AUDIO] BLE rx: %u bytes, dropped: %u bytes, prebuffering: %s\n", 
                    totalBytesReceived, totalBytesDropped, prebuffering ? "true" : "false");
    }
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

  void begin() {
    init();
  }

  void init() {
    // Ring buffer: 65536 bytes ≈ 2s of 16kHz 16-bit mono PCM
    txRingBuffer = xRingbufferCreate(16384, RINGBUF_TYPE_BYTEBUF);
    rxRingBuffer = xRingbufferCreate(2048,  RINGBUF_TYPE_BYTEBUF);

    // I2S DMA config: 32-bit sample width is required by the INMP441 microphone to correctly align 24-bit samples
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // Stereo format (req by DAC clocking)
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len  = 256,
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S2)
      .use_apll = false,     // APLL is not supported on ESP32-S3, ESP32-C3, or ESP32-S2
#else
      .use_apll = true,      // APLL = precise audio PLL clock (original ESP32)
#endif
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
      
      // Enable internal pull-down on the SD pin to prevent massive floating noise during tri-stated phases
      gpio_pulldown_en((gpio_num_t)I2S_DIN);
      gpio_pullup_dis((gpio_num_t)I2S_DIN);
      
      Serial.println("[AUDIO] I2S driver OK (dma_buf=256*8, 32-bit duplex, pull-down enabled).");
    } else {
      Serial.printf("[AUDIO] I2S driver FAILED: %d\n", err);
    }

    xTaskCreatePinnedToCore(txAudioTask, "audio_tx", 4096, this, 5, &audioTxTaskHandle, 1);
    xTaskCreatePinnedToCore(rxAudioTask, "audio_rx", 6144, this, 4, &audioRxTaskHandle, 0);
  }

  static void txAudioTask(void* pvParameters) {
    LunaAudio* self = (LunaAudio*)pvParameters;

    // Synth oscillator state (stereo 32-bit, 128 samples to save stack space)
    int32_t synthBuf[256];   // 128 samples × 2 channels × 4 bytes = 1024 bytes per I2S write
    double  phase = 0;

    // Silence block for prebuffer/underrun fill (stereo 32-bit, 128 samples)
    static const int32_t SILENCE[256] = {};
    size_t bytes_written;

    while (true) {
      // STREAM MODE
      if (self->audioMode == AUDIO_MODE_STREAM) {
        if (self->txRingBuffer == NULL) {
          vTaskDelay(pdMS_TO_TICKS(10));
          continue;
        }

        if (self->prebuffering) {
          size_t freeBytes = xRingbufferGetCurFreeSize(self->txRingBuffer);
          size_t filledBytes = 16384 - freeBytes;
          if (filledBytes >= 12000) {
            self->prebuffering = false;
            Serial.printf("[AUDIO] Prebuffer done, filled=%u bytes. Starting playback.\n", filledBytes);
          } else {
            i2s_write(I2S_NUM_0, SILENCE, sizeof(SILENCE), &bytes_written, portMAX_DELAY);
            continue;
          }
        }

        size_t item_size = 0;
        int16_t* item = (int16_t*)xRingbufferReceive(
            self->txRingBuffer, &item_size, pdMS_TO_TICKS(5));

        if (item != NULL && item_size > 0) {
          int vol       = self->_streamVolume;  // 0-100
          int bassLevel = self->_bassBoost;     // 0-10
          int samples   = (int)(item_size / 2);

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

          int32_t stereoBuf[256]; // 128 samples × 2 channels (1024 bytes)
          int stereoSamples = samples;
          if (stereoSamples > 128) stereoSamples = 128;
          for (int i = 0; i < stereoSamples; i++) {
            int32_t val = (int32_t)item[i] << 16; // Shift 16-bit to 32-bit for DAC
            stereoBuf[2 * i]     = val; // Left
            stereoBuf[2 * i + 1] = val; // Right
          }

          if (stereoSamples > 0) {
            i2s_write(I2S_NUM_0, stereoBuf, stereoSamples * 8, &bytes_written, portMAX_DELAY);
          }

          static uint32_t last_print = 0;
          if (millis() - last_print > 1000) {
            last_print = millis();
            Serial.printf("[AUDIO TX] item_size: %d, stereoSamples: %d, sample[0]: %d\n", (int)item_size, stereoSamples, (int)stereoBuf[0]);
          }

          vRingbufferReturnItem(self->txRingBuffer, (void*)item);

        } else {
          vTaskDelay(pdMS_TO_TICKS(2));
        }

      // SYNTH MODE
      } else {
        // *** CRITICAL: During direct loopback the RX task owns i2s_write. ***
        // If we also write silence here we overwrite the mic audio → user hears only noise.
        if (self->directLoopback) {
          vTaskDelay(pdMS_TO_TICKS(10));
          continue;
        }

        int freq = self->currentFrequency;
        if (freq > 0) {
          for (int i = 0; i < 128; i++) {
            phase += (2.0 * M_PI * freq) / 16000.0;
            if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
            int32_t val = (int32_t)(sin(phase) * self->currentVolume) << 16;
            synthBuf[2 * i]     = val;
            synthBuf[2 * i + 1] = val;
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
    // Static buffers: live in DRAM, not on the FreeRTOS task stack (avoids stack overflow)
    static int32_t read_buffer[512]; // 128 stereo 32-bit frames = 2048 bytes
    static int32_t loopback_tx[256]; // 128 stereo 32-bit frames = 1024 bytes for direct i2s_write

    // HPF state
    float prev_in  = 0.0f;
    float prev_out = 0.0f;
    const float alpha = 0.98f; // ~100 Hz HPF @ 16 kHz

    while (true) {
      size_t bytes_read = 0;
      esp_err_t err = i2s_read(I2S_NUM_0, read_buffer, 128 * 2 * sizeof(int32_t), &bytes_read, portMAX_DELAY);
      if (err != ESP_OK || bytes_read == 0) {
        vTaskDelay(pdMS_TO_TICKS(2));
        continue;
      }

      // Each I2S frame is two int32_t words (Left + Right channel)
      int frames = bytes_read / (2 * sizeof(int32_t)); // stereo frames
      if (frames > 128) frames = 128;

      // ---- Amplitude monitor (1 Hz print) ----
      int32_t sum = 0;
      for (int i = 0; i < frames; i++) {
        // >> 16 gives top 16 bits of the 24-bit audio sample — correct 16-bit amplitude metric
        int16_t s = (int16_t)(read_buffer[2 * i] >> 16);
        sum += abs(s);
      }
      micAmplitude = frames > 0 ? (sum / frames) : 0;

      static uint32_t last_print = 0;
      if (millis() - last_print > 1000) {
        last_print = millis();
        Serial.printf("[AUDIO RX] frames=%d amp=%d raw_L=0x%08X raw_R=0x%08X loopback=%d\n",
                      frames, micAmplitude,
                      (unsigned)read_buffer[0], (unsigned)read_buffer[1],
                      (int)self->directLoopback);
      }

      // ---- DIRECT LOOPBACK: RAW passthrough with gain boost ----
      if (self->directLoopback) {
        size_t bytes_written = 0;
        for (int i = 0; i < frames; i++) {
          // Raw mic value is left-aligned 24-bit in a 32-bit word.
          // Apply 50x gain to make quiet INMP441 signal audible through speaker.
          int64_t raw = (int64_t)(int32_t)read_buffer[2 * i];
          int64_t boosted = raw * 50LL;
          if (boosted >  2147483647LL) boosted =  2147483647LL;
          if (boosted < -2147483648LL) boosted = -2147483648LL;
          loopback_tx[2 * i]     = (int32_t)boosted; // L
          loopback_tx[2 * i + 1] = (int32_t)boosted; // R
        }
        size_t want = frames * 2 * sizeof(int32_t);
        i2s_write(I2S_NUM_0, loopback_tx, want, &bytes_written, 0);

        static uint32_t lp_print = 0;
        if (millis() - lp_print > 1000) {
          lp_print = millis();
          Serial.printf("[LOOPBACK] wrote %d/%d bytes, amp=%d\n", (int)bytes_written, (int)want, micAmplitude);
        }

      // ---- BLE STREAMING: mic → rxRingBuffer → main loop → BLE (16-bit PCM) ----
      } else if (self->micStreaming && self->rxRingBuffer != NULL) {
        int16_t mono_buf[128];
        for (int i = 0; i < frames; i++) {
          mono_buf[i] = (int16_t)(read_buffer[2 * i] >> 16);
        }
        xRingbufferSend(self->rxRingBuffer, mono_buf, frames * sizeof(int16_t), 0);
      }
    }
  }
};

#endif // AUDIO_H
