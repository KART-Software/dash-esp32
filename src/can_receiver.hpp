#ifndef _CAN_RECEIVER_H_
#define _CAN_RECEIVER_H_

#include <ESP32CAN.h>
#include <CAN_config.h>
#include <Arduino.h>

class CanReceiver
{
public:
    CanReceiver(CAN_device_t *device, ESP32CAN *can = &ESP32Can);
    bool receive(uint8_t *data);
    void initialize();

private:
    ESP32CAN *can;
    CAN_device_t *device;
};

#endif