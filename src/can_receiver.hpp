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
    CanReceiver();
    bool receive(char *data, uint8_t startIndex);
    void initialize();
    uint8_t getDataLength();
    void start(char *data, uint8_t startIndex);

private:
    StateIndicator canIndicator;
    ESP32CAN *can;
    CAN_device_t *device;
    uint8_t dataLength;
    uint16_t canIdEnd;
    void detectDataLength();
    void setFilter();
};

#endif