#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"

// Forward declaration of handlers implemented in the main sketch
extern void handleBLEExpression(Expression expr);
extern void handleBLEExpressionWithLabel(Expression expr, String label);
extern void handleBLEAudio(SoundEffect sound);
extern void handleBLEText(String text);

extern bool negativeDisplay;

class LunaBLE {
private:
  BLEServer* pServer;
  BLECharacteristic* pExpressionChar;
  BLECharacteristic* pAudioChar;
  BLECharacteristic* pTextChar;
  BLECharacteristic* pStatusChar;
  bool deviceConnected;
  bool oldDeviceConnected;
  bool isInitialized;
  bool advertising;

  class ServerCallbacks : public BLEServerCallbacks {
  private:
    LunaBLE& ble;
  public:
    ServerCallbacks(LunaBLE& instance) : ble(instance) {}
    void onConnect(BLEServer* pServer) override {
      ble.deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) override {
      ble.deviceConnected = false;
    }
  };

  class ExpressionCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
      uint8_t* data = pChar->getData();
      size_t len = pChar->getLength();
      if (len > 0) {
        uint8_t exprVal = data[0];
        String label = "";
        if (len > 1) {
          for (size_t i = 1; i < len; i++) {
            label += (char)data[i];
          }
        }
        if (exprVal <= EXPR_ALL_GIF) {
          handleBLEExpressionWithLabel((Expression)exprVal, label);
        }
      }
    }
  };

  class AudioCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
      uint8_t* data = pChar->getData();
      size_t len = pChar->getLength();
      if (len > 0) {
        uint8_t audioVal = data[0];
        if (audioVal <= SOUND_THEMECHANGE) {
          handleBLEAudio((SoundEffect)audioVal);
        }
      }
    }
  };

  class TextCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
      uint8_t* data = pChar->getData();
      size_t len = pChar->getLength();
      if (len > 0) {
        String val = "";
        val.reserve(len);
        for (size_t i = 0; i < len; i++) {
          val += (char)data[i];
        }
        handleBLEText(val);
      }
    }
  };

public:
  LunaBLE() : pServer(nullptr), deviceConnected(false), oldDeviceConnected(false), isInitialized(false), advertising(false) {}

  void init() {
    if (isInitialized) return;

    // Initialize BLE Device
    BLEDevice::init(negativeDisplay ? "Ms. Luna Robot" : "Mr. Luna Robot");

    // Create BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(*this));

    // Create BLE Service
    BLEService* pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristics
    pExpressionChar = pService->createCharacteristic(
                        EXPRESSION_CHAR_UUID,
                        BLECharacteristic::PROPERTY_WRITE
                      );
    pExpressionChar->setCallbacks(new ExpressionCallbacks());

    pAudioChar = pService->createCharacteristic(
                   AUDIO_CHAR_UUID,
                   BLECharacteristic::PROPERTY_WRITE
                 );
    pAudioChar->setCallbacks(new AudioCallbacks());

    pTextChar = pService->createCharacteristic(
                  TEXT_CHAR_UUID,
                  BLECharacteristic::PROPERTY_WRITE
                );
    pTextChar->setCallbacks(new TextCallbacks());

    pStatusChar = pService->createCharacteristic(
                    STATUS_CHAR_UUID,
                    BLECharacteristic::PROPERTY_READ |
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
    pStatusChar->addDescriptor(new BLE2902());

    // Start Service
    pService->start();

    // Start Advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinInterval(0x20);  // 20ms advertising interval (32 * 0.625ms)
    pAdvertising->setMaxInterval(0x40);  // 40ms advertising interval (64 * 0.625ms)
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    isInitialized = true;
    advertising = true;
  }

  void setBLEActive(bool active) {
    if (active) {
      if (!isInitialized) {
        init();
      } else if (!advertising) {
        BLEDevice::startAdvertising();
        advertising = true;
      }
    } else {
      if (isInitialized && advertising) {
        BLEDevice::getAdvertising()->stop();
        if (deviceConnected && pServer) {
          pServer->disconnect(0);
        }
        advertising = false;
      }
    }
  }

  void updateStatus(unsigned long uptimeSeconds, unsigned int touchCount, float batteryEst, Expression currentExpr, String currentLabel) {
    if (!isConnected()) return;

    // Create comma-separated status payload: uptime_sec,touch_cnt,battery_val,expr_val,label_val
    String payload = String(uptimeSeconds) + "," + String(touchCount) + "," + String(batteryEst, 2) + "," + String((int)currentExpr) + "," + currentLabel;
    pStatusChar->setValue(payload.c_str());
    pStatusChar->notify();
  }

  void sendLog(String logMsg) {
    if (!isConnected()) return;
    String payload = "LOG:" + logMsg;
    pStatusChar->setValue(payload.c_str());
    pStatusChar->notify();
  }

  void handleConnectionState() {
    bool currentConnected = isConnected();
    // Disconnecting
    if (!currentConnected && oldDeviceConnected) {
      delay(200); // reduced delay for faster reconnection!
      if (advertising) {
        BLEDevice::startAdvertising(); // restart advertising cleanly
      }
      oldDeviceConnected = currentConnected;
    }
    // Connecting
    if (currentConnected && !oldDeviceConnected) {
      // do stuff when connecting
      oldDeviceConnected = currentConnected;
    }
  }

  bool isConnected() const {
    if (pServer) {
      return pServer->getConnectedCount() > 0;
    }
    return deviceConnected;
  }
};

#endif // BLUETOOTH_H
