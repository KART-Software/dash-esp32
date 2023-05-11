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
    dataLength = CAN_DATA_LENGTH;
    canIdEnd = CAN_ID_END;
}

bool CanReceiver::receive(char *data, uint8_t startIndex)
{
    // TODOリファクタ（？）
    for (uint16_t id = CAN_ID_START; id <= canIdEnd; id++)
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
            else
            {
                return false;
            }
        }
        for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
        {
            data[startIndex + (id - CAN_ID_START) * 8 + i] = rx_frame.data.u8[i];
        }
    }
    return true;
}

void CanReceiver::detectDataLength()
{
    // TODOリファクタ（？）
    CAN_frame_t rx_frame;
    canIdEnd = CAN_ID_START;
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
    // メモ
    // CAN ID は11ビットであることに注意。
    //   ID 0b abcd efgh ijkl
    // MASK 0b mnop qrst uvwx
    //         ↑           ↑
    //      ここから     ここまで が使うビット
    // これを踏まえて、AN97076のマニュアルを見てばかやまじ

    // この場合CAN_ID_START(0x5F0)に対して0x5F□が許される。
    filter.ACR0 = (CAN_ID_START >> 3) & 0xFF;
    filter.ACR1 = (CAN_ID_START & 0b111) << 5;
    filter.ACR2 = 0x00;
    filter.ACR3 = 0x00;

    filter.AMR0 = 0b00000001;
    filter.AMR1 = 0xFF;
    filter.AMR2 = 0xFF;
    filter.AMR3 = 0xFF;
    can->CANConfigFilter(&filter);
}