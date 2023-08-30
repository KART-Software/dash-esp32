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

StateIndicator WsConnectionIndicator = StateIndicator(WS_LED_PIN);

  
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    //client connected
    WsConnectionIndicator.setStateConnected();
    printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    WsConnectionIndicator.setStateNoConnection();
    printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end
    printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)
    printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
      if(info->opcode == WS_TEXT){
        data[len] = 0;
        printf("%s\n", (char*)data);
      } else {
        for(size_t i=0; i < info->len; i++){
          printf("%02x ", data[i]);
        }
        printf("\n");
      }
      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
      if(info->message_opcode == WS_TEXT){
        data[len] = 0;
        printf("%s\n", (char*)data);
      } else {
        for(size_t i=0; i < len; i++){
          printf("%02x ", data[i]);
        }
        printf("\n");
      }

      if((info->index + len) == info->len){
        printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

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
  data = new char[dataLength ];
  for (int i = 0; i < dataLength; i++)
  {
    data[i] = 0;
  }
  canReceiver.setListToWrite(data, 0);
  xTaskCreatePinnedToCore(startCanReceiver, "CanReceiveTask", 8192, (void *)&canReceiver, 1, &canReceiveTask, 1);

  WiFi.config(ip, gateway, subnet); //static_ip
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to ws...");
  int wifiReconnectionCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    if (WiFi.status() == WL_CONNECT_FAILED) {
      if (wifiReconnectionCount > 5) {
        wifiReconnectionCount = 0;
        ESP.restart();
      }
      Serial.println("Can't connect");
      //ESP.restart();
      wifiReconnectionCount++;
      }
    }
  Serial.println("Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.addHandler(&ws);
  ws.onEvent(onEvent);

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
  doc["rpm"] = fromBytesToInt16Reverse(data, dataLength, 0, 2);
  doc["throttlePosition"] = fromBytesToInt16Reverse(data, dataLength, 2, 2)/10.0;
  doc["engineTemp"] = fromBytesToInt16Reverse(data, dataLength, 4, 2)/10.0;
  doc["oilTemp"] = fromBytesToInt16Reverse(data, dataLength, 6, 2)/10.0;
  doc["oilPressure"] = fromBytesToInt16Reverse(data, dataLength, 8, 2);
  doc["gearVoltage"] = fromBytesToInt16Reverse(data, dataLength, 10, 2)/1000.0;

  doc["latitude"] = fromBytesToInt32(data, dataLength, 24, 4);
  doc["longitude"] = fromBytesToInt32(data, dataLength, 28, 4);
  doc["altitude"] = fromBytesToInt32(data, dataLength, 32, 4);
  doc["gpsSpeed2D"] = fromBytesToInt32(data, dataLength, 38, 3);
  doc["gpsSpeed3D"] = fromBytesToInt32(data, dataLength, 41, 3);
  sendDataWs();
  delay(100);
}