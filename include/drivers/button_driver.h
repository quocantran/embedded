
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
    bool _lastRawState = HIGH;          // Trạng thái đọc thô gần nhất (debug)
    bool _stableState = HIGH;           // Trạng thái ổn định sau debounce
    bool _lastStableState = HIGH;       // Trạng thái ổn định trước đó

    unsigned long _pressStartTime = 0;  // Thời điểm bắt đầu nhấn
    unsigned long _lastHoldLogTime = 0; // Log tien trinh nhan giu moi 500ms
    unsigned long _lastSampleTime = 0;  // Moc thoi gian lay mau cho bo loc

    uint8_t _integrator = 0;            // Bo dem tich phan chong nhieu

    bool _longPressHandled = false;     // Long press đã được xử lý cho lần nhấn này?
    ButtonEvent _pendingEvent = ButtonEvent::NONE;  // Sự kiện đang chờ xử lý
};

#endif // BUTTON_DRIVER_H
