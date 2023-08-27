#ifndef _CAN_RECEIVER_H_
#define _CAN_RECEIVER_H_

#include <ESP32CAN.h>
#include <CAN_config.h>
#include <Arduino.h>
#include "constant.h"
#include "status.hpp"

class CanReceiver
{
public:
    CanReceiver(CAN_device_t *device);
    bool receive();
    void initialize();
    void setListToWrite(char *data, uint8_t startIndex);
    uint8_t getDataLength();
    void start();

private:
    StateIndicator canIndicator = StateIndicator(CAN_LED_PIN);
    ESP32CAN *can = &ESP32Can;
    CAN_device_t *device;
    uint8_t dataLength;
    uint16_t canIdEnd;
    char *data;
    uint8_t dataStartIndex;
    void detectDataLength();
    void setFilter();
};

void startCanReceiver(void *data);

#endif