#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected!");
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected, restarting advertising...");
      // No need to clear peer devices unless you use bonding/pairing
      pServer->getAdvertising()->start();
    }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32_BLE");
  BLEDevice::setMTU(100); // Set MTU to 100 bytes (max is 517, but depends on client support)
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  static int msgCount = 0;
  if (deviceConnected) {
    msgCount++;
    String msg = "Hello message from ESP32 #" + String(msgCount);
    pCharacteristic->setValue(msg.c_str());
    pCharacteristic->notify();
    Serial.println("Sending: " + msg); // Print the sending message
    delay(2000); // Send every second
  }
}