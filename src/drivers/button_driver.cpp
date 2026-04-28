/**
 * @file button_driver.cpp
 * @brief Triển khai driver nút nhấn với debounce và phát hiện long press
 * 
 * Thuật toán debounce:
 * 1. Đọc trạng thái thô từ GPIO
 * 2. Nếu thay đổi so với lần đọc trước → reset timer debounce
 * 3. Nếu timer debounce >= 50ms → trạng thái ổn định
 * 4. Phát hiện cạnh lên/xuống từ trạng thái ổn định
 * 
 * Phát hiện sự kiện:
 * - LONG_PRESS: phát hiện ngay khi nhấn giữ >= 3 giây (không cần thả)
 * - SHORT_PRESS: phát hiện khi thả nút (chỉ nếu chưa phải long press)
 * 
 * Kết nối phần cứng:
 *   ESP32 GPIO4 ──── [NÚT NHẤN] ──── GND
 *   (Internal pull-up bật, nhấn = LOW, thả = HIGH)
 */

#include "drivers/button_driver.h"

// ============================================================
// Khởi tạo nút nhấn
// ============================================================
void ButtonDriver::init() {
    // INPUT_PULLUP: kéo lên nội bộ, nút nối GND
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    // Đọc trạng thái ban đầu
    _lastRawState = digitalRead(PIN_BUTTON);
    _stableState = _lastRawState;
    _lastStableState = _lastRawState;
    _pendingEvent = ButtonEvent::NONE;

    Serial.println(F("[BUTTON] Khoi tao nut nhan GPIO4 (INPUT_PULLUP)"));
}

// ============================================================
// Cập nhật trạng thái nút nhấn (GỌI MỖI VÒNG LOOP)
// ============================================================
void ButtonDriver::update() {
    // Bước 1: Đọc trạng thái thô từ GPIO
    bool rawReading = digitalRead(PIN_BUTTON);

    // Bước 2: Kiểm tra thay đổi trạng thái thô → reset debounce timer
    if (rawReading != _lastRawState) {
        _lastDebounceTime = millis();
    }
    _lastRawState = rawReading;

    // Bước 3: Kiểm tra debounce timer đã hết chưa
    if ((millis() - _lastDebounceTime) < BUTTON_DEBOUNCE_MS) {
        return; // Chưa ổn định, bỏ qua
    }

    // Bước 4: Trạng thái đã ổn định → cập nhật
    _lastStableState = _stableState;
    _stableState = rawReading;

    // ─── Phát hiện cạnh xuống (nút vừa được NHẤN) ───
    if (_stableState == LOW && _lastStableState == HIGH) {
        // Bắt đầu nhấn - ghi nhận thời điểm
        _pressStartTime = millis();
        _longPressHandled = false;
    }

    // ─── Phát hiện long press (đang GIỮ nút) ───
    if (_stableState == LOW && !_longPressHandled) {
        unsigned long holdTime = millis() - _pressStartTime;
        if (holdTime >= BUTTON_LONG_PRESS_MS) {
            // Nhấn giữ đủ 3 giây → sự kiện LONG_PRESS
            _pendingEvent = ButtonEvent::LONG_PRESS;
            _longPressHandled = true; // Chỉ phát hiện 1 lần cho mỗi lần nhấn
            Serial.println(F("[BUTTON] === NHAN GIU (LONG PRESS) ==="));
        }
    }

    // ─── Phát hiện cạnh lên (nút vừa được THẢ) ───
    if (_stableState == HIGH && _lastStableState == LOW) {
        // Nút vừa thả - kiểm tra đây có phải short press không
        if (!_longPressHandled) {
            // Không phải long press → đây là SHORT_PRESS
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
