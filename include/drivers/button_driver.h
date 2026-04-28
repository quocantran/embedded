#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>
#include "config.h"
#include "types.h"

class ButtonDriver {
public:

    void init();

    void update();

    ButtonEvent getEvent();

private:
    bool _lastRawState = HIGH;          
    bool _stableState = HIGH;           
    bool _lastStableState = HIGH;       

    unsigned long _lastDebounceTime = 0;
    unsigned long _pressStartTime = 0;

    bool _longPressHandled = false;
    ButtonEvent _pendingEvent = ButtonEvent::NONE;
};

#endif // BUTTON_DRIVER_H
