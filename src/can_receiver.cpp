#include "can_receiver.hpp"

CanReceiver::CanReceiver(CAN_device_t *device, ESP32CAN *can) : device(device), can(can) {}

void CanReceiver::initialize()
{
    device->speed = CAN_SPEED_500KBPS;
    device->tx_pin_id = TX_PIN;
    device->rx_pin_id = RX_PIN;
    device->rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));
    setFilter();
    can->CANInit();
    // detectDataLength();
    dataLength = CAN_DATA_LENGTH_1 + CAN_DATA_LENGTH_2;
}

bool CanReceiver::receive(char *data, uint8_t startIndex)
{
    // return receive1(data, startIndex) && receive2(data, startIndex + CAN_DATA_LENGTH_1);
    return receive2(data, startIndex + CAN_DATA_LENGTH_1);
}

bool CanReceiver::receive1(char *data, uint8_t startIndex)
{
    // TODOリファクタ（？）
    for (uint32_t id = CAN_ID_START_1; id <= CAN_ID_END_1; id++)
    {
        CAN_frame_t rx_frame;
        for (int count = 0; count < 10; count++)
        {
            if (xQueueReceive(device->rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
            {
                if (rx_frame.MsgID == id)
                {
                    for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
                    {
                        data[startIndex + (id - CAN_ID_START_1) * 8 + i] = rx_frame.data.u8[i];
                    }
                    break;
                }
            }
            if (count >= 9)
            {
                Serial.print(id);
                return false;
            }
        }
    }
    return true;
}

bool CanReceiver::receive2(char *data, uint8_t startIndex)
{
    // TODOリファクタ（？）
    for (uint32_t id = CAN_ID_START_2; id <= CAN_ID_END_2; id++)
    {
        CAN_frame_t rx_frame;
        for (int count = 0; count < 10; count++)
        {
            if (xQueueReceive(device->rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
            {
                if (rx_frame.MsgID == id)
                {
                    for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
                    {
                        data[startIndex + (id - CAN_ID_START_2) * 8 + i] = rx_frame.data.u8[i];
                    }
                    break;
                }
            }
            if (count >= 9)
            {
                Serial.print(id);
                return false;
            }
        }
    }
    return true;
}

void CanReceiver::detectDataLength()
{
    // TODOリファクタ（？）
    CAN_frame_t rx_frame;
    canIdEnd = CAN_ID_START_1;
    dataLength = 0;
    for (int _ = 0; _ < 100; _++)
    {
        if (xQueueReceive(device->rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
        {
            if (rx_frame.MsgID > canIdEnd)
            {
                canIdEnd = rx_frame.MsgID;
                if (rx_frame.FIR.B.DLC < 8)
                {
                    break;
                }
            }
        }
    }
    for (int _ = 0; _ < 30; _++)
    {
        if (xQueueReceive(device->rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
        {
            if (rx_frame.MsgID == canIdEnd)
            {
                dataLength = (canIdEnd - CAN_ID_START_1) * 8 + rx_frame.FIR.B.DLC;
                break;
            }
        }
    }
}

uint8_t CanReceiver::getDataLength()
{
    return dataLength;
}

void CanReceiver::setFilter()
{
    CAN_filter_t filter;

    filter.FM = Dual_Mode;
    // CAN_ID_START_1 と CAN_ID_START_2 の2つのIDに対してフィルターを設定したいので、Dual_Mode
    // 詳しくはAN97076のマニュアルを見てばかやまじ

    // 以下は CAN_ID_START から16個の CAN ID をフィルターする。

    filter.ACR0 = (CAN_ID_START_1 >> 3) & 0xFF;
    filter.ACR1 = (CAN_ID_START_1 & 0b111) << 5;
    filter.ACR2 = (CAN_ID_START_2 >> 3) & 0xFF;
    filter.ACR3 = (CAN_ID_START_2 & 0b111) << 5;

    filter.AMR0 = 0b00000001;
    filter.AMR1 = 0b11111111;
    filter.AMR2 = 0b00000001;
    filter.AMR3 = 0b11111111;
    can->CANConfigFilter(&filter);
}