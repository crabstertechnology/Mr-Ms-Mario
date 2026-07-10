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
  volatile int currentVolume;

  LunaAudio() : currentFrequency(0), currentVolume(8000) {
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

  void writeTxStream(const uint8_t* data, size_t len) {
    if (txRingBuffer == NULL) return;
    audioMode = AUDIO_MODE_STREAM;
    xRingbufferSend(txRingBuffer, data, len, pdMS_TO_TICKS(5));
  }

  void startMusicStream() {
    // Clear any stale data in the ring buffer
    if (txRingBuffer != NULL) {
      void* item;
      size_t item_size;
      while ((item = xRingbufferReceive(txRingBuffer, &item_size, 0)) != NULL) {
        vRingbufferReturnItem(txRingBuffer, item);
      }
    }
    prebuffering = true; // wait for enough data before playing
    audioMode = AUDIO_MODE_STREAM;
  }

  void stopMusicStream() {
    audioMode = AUDIO_MODE_SYNTH;
    prebuffering = true; // reset for next time
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
    txRingBuffer = xRingbufferCreate(32768, RINGBUF_TYPE_BYTEBUF);
    rxRingBuffer = xRingbufferCreate(8192, RINGBUF_TYPE_BYTEBUF);

    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true
    };
    
    i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCLK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_DOUT,
      .data_in_num = I2S_DIN
    };
    
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err == ESP_OK) {
      i2s_set_pin(I2S_NUM_0, &pin_config);
      Serial.println("I2S Duplex driver initialized successfully.");
    } else {
      Serial.printf("Failed to install I2S driver: %d\n", err);
    }

    xTaskCreatePinnedToCore(
      txAudioTask,
      "audio_tx_task",
      3072,
      this,
      4,
      &audioTxTaskHandle,
      0
    );

    xTaskCreatePinnedToCore(
      rxAudioTask,
      "audio_rx_task",
      2048,
      this,
      4,
      &audioRxTaskHandle,
      1
    );
  }

  static void txAudioTask(void* pvParameters) {
    LunaAudio* self = (LunaAudio*)pvParameters;
    int16_t buffer[128];
    double phase = 0;
    
    while (true) {
      if (self->audioMode == AUDIO_MODE_STREAM) {
        size_t ringbuf_bytes = 0;
        if (self->txRingBuffer != NULL) {
          vRingbufferGetInfo(self->txRingBuffer, NULL, NULL, NULL, NULL, &ringbuf_bytes);
        }
        
        if (self->prebuffering) {
          if (ringbuf_bytes >= 8192) {
            self->prebuffering = false;
          }
        }
        
        if (!self->prebuffering) {
          size_t item_size = 0;
          int16_t* item = (int16_t*)xRingbufferReceive(self->txRingBuffer, &item_size, 0);
          if (item != NULL) {
            size_t bytes_written;
            i2s_write(I2S_NUM_0, item, item_size, &bytes_written, portMAX_DELAY);
            vRingbufferReturnItem(self->txRingBuffer, (void*)item);
          } else {
            // Buffer underrun! Start prebuffering again.
            self->prebuffering = true;
          }
        }
        
        if (self->prebuffering) {
          // Play silence while prebuffering
          memset(buffer, 0, sizeof(buffer));
          size_t bytes_written;
          i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
          vTaskDelay(pdMS_TO_TICKS(4));
        }
      } else {
        int freq = self->currentFrequency;
        if (freq > 0) {
          for (int i = 0; i < 128; i++) {
            phase += (2.0 * PI * freq) / 16000.0;
            if (phase >= 2.0 * PI) phase -= 2.0 * PI;
            buffer[i] = (int16_t)(sin(phase) * self->currentVolume);
          }
          size_t bytes_written;
          i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
        } else {
          memset(buffer, 0, sizeof(buffer));
          size_t bytes_written;
          i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
          vTaskDelay(pdMS_TO_TICKS(10));
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
