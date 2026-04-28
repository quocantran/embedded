#include "drivers/button_driver.h"

void ButtonDriver::init() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    _lastRawState = digitalRead(PIN_BUTTON);
    _stableState = _lastRawState;
    _lastStableState = _lastRawState;
    _pendingEvent = ButtonEvent::NONE;

    Serial.println(F("[BUTTON] Khoi tao nut nhan GPIO0 (INPUT_PULLUP)"));
}


void ButtonDriver::update() {

    bool rawReading = digitalRead(PIN_BUTTON);

    if (rawReading != _lastRawState) {
        _lastDebounceTime = millis();
    }
    _lastRawState = rawReading;

    if ((millis() - _lastDebounceTime) < BUTTON_DEBOUNCE_MS) {
        return; // Chưa ổn định, bỏ qua
    }

    _lastStableState = _stableState;
    _stableState = rawReading;

    if (_stableState == LOW && _lastStableState == HIGH) {
    
        _pressStartTime = millis();
        _longPressHandled = false;
    }

    if (_stableState == LOW && !_longPressHandled) {
        unsigned long holdTime = millis() - _pressStartTime;
        if (holdTime >= BUTTON_LONG_PRESS_MS) {
            // Nhấn giữ đủ 3 giây → sự kiện LONG_PRESS
            _pendingEvent = ButtonEvent::LONG_PRESS;
            _longPressHandled = true; // Chỉ phát hiện 1 lần cho mỗi lần nhấn
            Serial.println(F("[BUTTON] === NHAN GIU (LONG PRESS) ==="));
        }
    }

    if (_stableState == HIGH && _lastStableState == LOW) {
        if (!_longPressHandled) {
            _pendingEvent = ButtonEvent::SHORT_PRESS;
            Serial.println(F("[BUTTON] --- Nhan ngan (SHORT PRESS) ---"));
        }
        // Reset cờ long press cho lần nhấn tiếp theo
        _longPressHandled = false;
    }
}

ButtonEvent ButtonDriver::getEvent() {
    ButtonEvent event = _pendingEvent;
    _pendingEvent = ButtonEvent::NONE;
    return event;
}
