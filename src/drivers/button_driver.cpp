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

static const uint8_t BUTTON_FILTER_MAX = 8;
static const uint8_t BUTTON_SAMPLE_INTERVAL_MS = 2;

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
    _lastHoldLogTime = 0;
    _lastSampleTime = millis();
    _integrator = (_stableState == LOW) ? BUTTON_FILTER_MAX : 0;

    Serial.printf("[BUTTON] Khoi tao nut nhan GPIO%d (INPUT_PULLUP)\n", PIN_BUTTON);
    Serial.printf("[BUTTON] Trang thai ban dau RAW=%s\n", _lastRawState == LOW ? "LOW(PRESSED)" : "HIGH(RELEASED)");
}

// ============================================================
// Cập nhật trạng thái nút nhấn (GỌI MỖI VÒNG LOOP)
// ============================================================
void ButtonDriver::update() {
    unsigned long now = millis();

    if ((now - _lastSampleTime) < BUTTON_SAMPLE_INTERVAL_MS) {
        return;
    }
    _lastSampleTime = now;

    // Bước 1: Đọc trạng thái thô từ GPIO
    bool rawReading = digitalRead(PIN_BUTTON);

    // Log khi trạng thái raw đổi (chỉ để debug)
    if (rawReading != _lastRawState) {
        Serial.printf("[BUTTON][RAW] %lu ms -> %s\n", now,
                      rawReading == LOW ? "LOW(PRESSED)" : "HIGH(RELEASED)");
    }
    _lastRawState = rawReading;

    // Bo loc tich phan: khang nhieu manh hon debounce timer co ban
    if (rawReading == LOW) {
        if (_integrator < BUTTON_FILTER_MAX) _integrator++;
    } else {
        if (_integrator > 0) _integrator--;
    }

    bool filteredState = _stableState;
    if (_integrator == 0) {
        filteredState = HIGH;
    } else if (_integrator >= BUTTON_FILTER_MAX) {
        filteredState = LOW;
    }

    // Chỉ cập nhật khi trạng thái ổn định thực sự thay đổi
    if (_stableState != filteredState) {
        _lastStableState = _stableState;
        _stableState = filteredState;

        Serial.printf("[BUTTON][STABLE] %lu ms -> %s\n", now,
                      _stableState == LOW ? "LOW(PRESSED)" : "HIGH(RELEASED)");

        // ─── Phát hiện cạnh xuống (nút vừa được NHẤN) ───
        if (_stableState == LOW && _lastStableState == HIGH) {
            _pressStartTime = now;
            _lastHoldLogTime = now;
            _longPressHandled = false;
            Serial.printf("[BUTTON] Bat dau nhan tai %lu ms\n", _pressStartTime);
        }

        // ─── Phát hiện cạnh lên (nút vừa được THẢ) ───
        if (_stableState == HIGH && _lastStableState == LOW) {
            unsigned long holdTime = now - _pressStartTime;
            if (!_longPressHandled) {
                _pendingEvent = ButtonEvent::SHORT_PRESS;
                Serial.printf("[BUTTON] --- Nhan ngan (SHORT PRESS), hold=%lu ms ---\n", holdTime);
            } else {
                Serial.printf("[BUTTON] Tha nut sau LONG PRESS, hold=%lu ms\n", holdTime);
            }
            _longPressHandled = false;
        }

        return;
    }

    // Van dang giu nut sau khi da on dinh LOW -> tiep tuc check long press
    if (_stableState == LOW && !_longPressHandled) {
        unsigned long holdTime = now - _pressStartTime;
        if ((now - _lastHoldLogTime) >= 500) {
            _lastHoldLogTime = now;
            Serial.printf("[BUTTON] Dang giu: %lu ms / %d ms\n", holdTime, BUTTON_LONG_PRESS_MS);
        }
        if (holdTime >= BUTTON_LONG_PRESS_MS) {
            _pendingEvent = ButtonEvent::LONG_PRESS;
            _longPressHandled = true;
            Serial.println(F("[BUTTON] === NHAN GIU (LONG PRESS) ==="));
        }
    }
}

// ============================================================
// Lấy sự kiện và xóa sự kiện đang chờ
// ============================================================
ButtonEvent ButtonDriver::getEvent() {
    ButtonEvent event = _pendingEvent;
    if (event == ButtonEvent::SHORT_PRESS) {
        Serial.println(F("[BUTTON] getEvent() => SHORT_PRESS"));
    } else if (event == ButtonEvent::LONG_PRESS) {
        Serial.println(F("[BUTTON] getEvent() => LONG_PRESS"));
    }
    _pendingEvent = ButtonEvent::NONE; // Xóa sự kiện sau khi đọc
    return event;
}
