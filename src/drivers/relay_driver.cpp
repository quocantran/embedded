/**
 * @file relay_driver.cpp
 * @brief Triển khai driver relay điều khiển bơm nước
 * 
 * Relay module 5V thông dụng:
 * - HIGH = BẬT relay (bơm chạy)
 * - LOW  = TẮT relay (bơm dừng)
 * - Theo dõi thời gian bật để bảo vệ timeout
 */

#include "drivers/relay_driver.h"

// ============================================================
// Khởi tạo chân relay
// ============================================================
void RelayDriver::init() {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);   // Mặc định TẮT bơm
    _isOn = false;
    _onStartTime = 0;

    Serial.println(F("[RELAY] Khoi tao relay - BOM TAT"));
}

// ============================================================
// Bật relay (bơm chạy)
// ============================================================
void RelayDriver::on() {
    if (!_isOn) {
        digitalWrite(PIN_RELAY, HIGH);
        _isOn = true;
        _onStartTime = millis();
        Serial.println(F("[RELAY] >>> BOM BAT <<<"));
    }
}

// ============================================================
// Tắt relay (bơm dừng)
// ============================================================
void RelayDriver::off() {
    if (_isOn) {
        digitalWrite(PIN_RELAY, LOW);
        uint32_t duration = getOnDurationSec();
        _isOn = false;
        _onStartTime = 0;
        Serial.printf("[RELAY] >>> BOM TAT <<< (da chay %lu giay)\n", duration);
    }
}

// ============================================================
// Đảo trạng thái relay (dùng cho nút nhấn manual)
// ============================================================
void RelayDriver::toggle() {
    if (_isOn) {
        off();
    } else {
        on();
    }
}

// ============================================================
// Kiểm tra relay có đang bật không
// ============================================================
bool RelayDriver::isOn() const {
    return _isOn;
}

// ============================================================
// Lấy thời gian relay đã bật liên tục (giây)
// ============================================================
uint32_t RelayDriver::getOnDurationSec() const {
    if (!_isOn || _onStartTime == 0) return 0;
    return (millis() - _onStartTime) / 1000;
}

// ============================================================
// Reset bộ đếm thời gian bật
// ============================================================
void RelayDriver::resetOnTimer() {
    if (_isOn) {
        _onStartTime = millis();
    }
}
