#ifndef _CONSTANT_H_
#define _CONSTANT_H_

#define TX_PIN GPIO_NUM_17
#define RX_PIN GPIO_NUM_16

#define CAN_LED_PIN 12
#define BLUETOOTH_LED_PIN 13

#define CAN_ID_START_1 0x5F0
#define CAN_ID_END_1 0x5F1
#define CAN_DATA_LENGTH_1 12
#define CAN_ID_START_2 0x6F0
#define CAN_ID_END_2 0x6F3
#define CAN_DATA_LENGTH_2 32

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define DEVICE_NAME "KART_DASH_ESP32"
#define BLUETOOTH_SEND_FREQUENCY 30

#endif