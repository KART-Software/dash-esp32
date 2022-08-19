#include "status.hpp"

StateIndicator::StateIndicator(uint8_t ledPin) : ledPin(ledPin)
{
}

void StateIndicator::initialize()
{
    pinMode(ledPin, OUTPUT);
}

void StateIndicator::setStateNoConnection()
{
    state = CommunicationState::NoConnection;
    digitalWrite(ledPin, LOW);
}

void StateIndicator::setStateConnected()
{
    state = CommunicationState::Connected;
    digitalWrite(ledPin, HIGH);
}

CommunicationState StateIndicator::getState()
{
    return state;
}