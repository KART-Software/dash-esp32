#include "can_receiver.hpp"

CanReceiver::CanReceiver(CAN_device_t *device, ESP32CAN *can) : device(device), can(can)
{
}

void CanReceiver::initialize()
{
    device->speed = CAN_SPEED_500KBPS;
    device->tx_pin_id = GPIO_NUM_16;
    device->rx_pin_id = GPIO_NUM_17;
    device->rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));
    can->CANInit();
}

bool CanReceiver::receive(uint8_t *data)
{
    CAN_frame_t rx_frame;
    if (xQueueReceive(device->rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
    {
        if (rx_frame.FIR.B.FF == CAN_frame_std && rx_frame.MsgID == 0x5F0)
        {
            data[0] = rx_frame.FIR.B.DLC;
            for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
            {
                data[i + 1] = rx_frame.data.u8[i];
            }
            return true;
        }
    }
    return false;
}