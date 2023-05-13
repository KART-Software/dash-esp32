#ifndef _STATE_INDICATOR_H_
#define _STATE_INDICATOR_H_

#include <Arduino.h>

enum class CommunicationState
{
    NoConnection,
    Connected
};

class StateIndicator
{
public:
    StateIndicator(uint8_t ledPin);
    void initialize(); // この関数はsetup関数内で実行されるようにしてください。LEDが光りません。
    void setStateNoConnection();
    void setStateConnected();
    CommunicationState getState();

private:
    CommunicationState state = CommunicationState::NoConnection;
    uint8_t ledPin;
};

#endif