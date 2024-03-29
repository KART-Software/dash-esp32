/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara
​
   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8
​
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.
​
   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "can_receiver.hpp"

// CAN_device_t CAN_cfg はここでやらないと何故かエラーが出ます。（グローバルにしないといけないのかも）
CAN_device_t CAN_cfg;
CanReceiver canReceiver = CanReceiver(&CAN_cfg);
uint8_t dataLength;
char *data;
TaskHandle_t canReceiveTask;

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
// StateIndicator bleIndicator = StateIndicator(BLUETOOTH_LED_PIN);
bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

class MyServerCallbacks : public BLEServerCallbacks
{
  StateIndicator bleIndicator;

public:
  MyServerCallbacks() : bleIndicator(StateIndicator(BLUETOOTH_LED_PIN))
  {
    bleIndicator.initialize();
  }
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    bleIndicator.setStateConnected();
  };
  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    bleIndicator.setStateNoConnection();
  }
};

void setup()
{
  Serial.begin(115200);
  canReceiver.initialize();
  dataLength = canReceiver.getDataLength();
  data = new char[dataLength + 4];
  for (int i = 0; i < dataLength + 4; i++)
  {
    data[i] = 0;
  }
  canReceiver.setListToWrite(data, 4);
  xTaskCreatePinnedToCore(startCanReceiver, "CanReceiveTask", 8192, (void *)&canReceiver, 1, &canReceiveTask, 1);

  // Create the BLE Device
  BLEDevice::init(DEVICE_NAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_INDICATE);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop()
{
  int ms = millis();
  for (int i = 0; i < 4; i++)
  {
    data[i] = (ms >> (8 * (3 - i))) & 0xFF;
  }
  for (int i = 0; i < dataLength + 4; i++)
  {
    printf("%02X ", data[i]);
  }
  printf("\n");

  std::string str_data = std::string(data, dataLength + 4);

  // notify changed value
  if (deviceConnected)
  {
    pCharacteristic->setValue(str_data);
    pCharacteristic->notify();
    delay(1000 / BLUETOOTH_SEND_FREQUENCY); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}