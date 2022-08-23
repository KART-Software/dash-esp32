#include "can_receiver.hpp"

CanReceiver::CanReceiver(CAN_device_t *device, ESP32CAN *can) : device(device), can(can) {}

void CanReceiver::initialize()
{
    device->speed = CAN_SPEED_500KBPS;
    device->tx_pin_id = GPIO_NUM_16;
    device->rx_pin_id = GPIO_NUM_17;
    device->rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));
    setFilter();
    can->CANInit();
    detectDataLength();
}

bool CanReceiver::receive(char *data, uint8_t startIndex)
{
    // これで動いてくれたらうれしい
    // 動いたとしてもだいぶ遅いかも
    for (uint8_t id = CAN_ID_START; id <= canIdEnd; id++)
    {
        CAN_frame_t rx_frame;
        for (int _ = 0; _ < 10; _++)
        {
            if (xQueueReceive(device->rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
            {
                if (rx_frame.MsgID == id)
                {
                    break;
                }
            }
        }
        for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
        {
            data[startIndex + (id - CAN_ID_START) * 8 + i] = rx_frame.data.u8[i];
        }
        return true;
    }
    return false;
}

void CanReceiver::detectDataLength()
{
    // これで動いてくれたらうれしい
    CAN_frame_t rx_frame;
    canIdEnd = CAN_ID_START;
    dataLength = 0;
    for (int _ = 0; _ < 30; _++)
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
                dataLength = (canIdEnd - CAN_ID_START) * 8 + rx_frame.FIR.B.DLC;
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

    filter.FM = Single_Mode;
    // ↓これあってるかわからない
    filter.ACR0 = CAN_ID_START >> 4;
    filter.ACR1 = (CAN_ID_START & 0x0F) << 4;
    filter.ACR2 = 0x00;
    filter.ACR3 = 0x00;

    filter.AMR0 = 0x00;
    filter.AMR1 = 0xFF;
    filter.AMR2 = 0xFF;
    filter.AMR3 = 0xFF;
    ESP32Can.CANConfigFilter(&filter);
}