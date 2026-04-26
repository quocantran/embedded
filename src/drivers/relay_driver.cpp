
#include "drivers/relay_driver.h"

void RelayDriver::init() {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);   // Mặc định TẮT bơm
    _isOn = false;
    _onStartTime = 0;

    Serial.println(F("[RELAY] Khoi tao relay - BOM TAT"));
}

void RelayDriver::on() {
    if (!_isOn) {
        digitalWrite(PIN_RELAY, HIGH);
        _isOn = true;
        _onStartTime = millis();
        Serial.println(F("[RELAY] >>> BOM BAT <<<"));
    }
}

void RelayDriver::off() {
    if (_isOn) {
        digitalWrite(PIN_RELAY, LOW);
        uint32_t duration = getOnDurationSec();
        _isOn = false;
        _onStartTime = 0;
        Serial.printf("[RELAY] >>> BOM TAT <<< (da chay %lu giay)\n", duration);
    }
}

void RelayDriver::toggle() {
    if (_isOn) {
        off();
    } else {
        on();
    }
}

bool RelayDriver::isOn() const {
    return _isOn;
}

uint32_t RelayDriver::getOnDurationSec() const {
    if (!_isOn || _onStartTime == 0) return 0;
    return (millis() - _onStartTime) / 1000;
}

void RelayDriver::resetOnTimer() {
    if (_isOn) {
        _onStartTime = millis();
    }
}
