#ifndef _CAN_RECEIVER_H_
#define _CAN_RECEIVER_H_

#include <ESP32CAN.h>
#include <CAN_config.h>
#include <Arduino.h>
#include "constant.h"

class CanReceiver
{
public:
    CanReceiver(CAN_device_t *device, ESP32CAN *can = &ESP32Can);
    bool receive(char *data, uint8_t startIndex);
    void initialize();
    uint8_t getDataLength();

private:
    ESP32CAN *can;
    CAN_device_t *device;
    uint8_t dataLength;
    uint8_t canIdEnd;
    void detectDataLength();
    void setFilter();
};

#endif