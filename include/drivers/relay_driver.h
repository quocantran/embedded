
#ifndef RELAY_DRIVER_H
#define RELAY_DRIVER_H

#include <Arduino.h>
#include "config.h"

class RelayDriver {
public:
    void init();

    void on();

    void off();

    void toggle();

    bool isOn() const;

    uint32_t getOnDurationSec() const;

    void resetOnTimer();

private:
    bool _isOn = false;                     // Trạng thái hiện tại
    unsigned long _onStartTime = 0;         // Thời điểm bắt đầu bật (millis)
};

#endif // RELAY_DRIVER_H
