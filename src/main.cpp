#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "can_receiver.hpp"
#include "converters.hpp"

IPAddress ip(192,168,4,6); //ip
IPAddress gateway(192,168, 11, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws

const char* ssid = "kart";
const char* password = "kosenkuso";

DynamicJsonDocument doc(1024);

// CAN_device_t CAN_cfg はここでやらないと何故かエラーが出ます。（グローバルにしないといけないのかも）
CAN_device_t CAN_cfg;
CanReceiver canReceiver = CanReceiver(&CAN_cfg);
uint8_t dataLength;
char *data;
TaskHandle_t canReceiveTask;

void onRequest(AsyncWebServerRequest *request){
  //Handle Unknown Request
  request->send(404);
}

void sendDataWs()
{
    size_t len = measureJson(doc);
    AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (buffer) {
      serializeJson(doc, (char *)buffer->get(), len + 1);
      ws.textAll(buffer);
      Serial.println("send");
    }
}

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

  WiFi.config(ip, gateway, subnet); //static_ip
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to ws...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Can't connect");
      ESP.restart();
      }
    }
  Serial.println("Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
  });

  server.onNotFound(onRequest);

  server.begin();
}

void loop()
{
  int ms = millis();
/*   for (int i = 0; i < 4; i++)
  {
    data[i] = (ms >> (8 * (3 - i))) & 0xFF;
  } */
  for (int i = 0; i < dataLength// + 4
  ; i++)
  {
    printf("%02X ", data[i]);
  }
  printf("\n");
  Serial.println(dataLength);
  
  doc["millis"] = ms;
  doc["rpm"] = fromBytesToInt16(data, dataLength, 0, 2);
  doc["throttlePosition"] = fromBytesToInt16(data, dataLength, 2, 2)/10;
  doc["engineTemp"] = fromBytesToInt16(data, dataLength, 4, 2)/10;
  doc["oilTemp"] = fromBytesToInt16(data, dataLength, 6, 2)/10;
  doc["oilPressure"] = fromBytesToInt16(data, dataLength, 8, 2);
  doc["gearVoltage"] = fromBytesToInt16(data, dataLength, 10, 2)/1000;

  doc["latitude"] = fromBytesToInt32Reverse(data, dataLength, 24, 4);
  doc["longitude"] = fromBytesToInt32Reverse(data, dataLength, 28, 4);
  doc["altitude"] = fromBytesToInt32Reverse(data, dataLength, 32, 4);
  doc["gpsSpeed2D"] = fromBytesToInt32Reverse(data, dataLength, 38, 3);
  doc["gpsSpeed3D"] = fromBytesToInt32Reverse(data, dataLength, 41, 3);
  sendDataWs();
  delay(100);
}