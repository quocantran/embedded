/**
 * @file lcd_driver.cpp
 * @brief Triển khai driver LCD I2C 16x2 với ký tự tùy chỉnh và nhấp nháy
 * 
 * Ký tự tùy chỉnh 5x8 pixel:
 * - CHAR_PUMP (0): Icon máy bơm
 * - CHAR_DROP (1): Icon giọt nước
 * - CHAR_SUN  (2): Icon mặt trời
 * - CHAR_WARN (3): Icon cảnh báo tam giác
 */

#include "drivers/lcd_driver.h"

// Dữ liệu ký tự tùy chỉnh 5x8 pixel
// Mỗi byte = 1 hàng, 5 bit thấp = 5 pixel
static const uint8_t PUMP_CHAR[8] PROGMEM = {
    0b00100,  //   *
    0b01110,  //  ***
    0b11111,  // *****
    0b01110,  //  ***
    0b00100,  //   *
    0b01010,  //  * *
    0b10001,  // *   *
    0b00000   //
};

static const uint8_t DROP_CHAR[8] PROGMEM = {
    0b00100,  //   *
    0b00100,  //   *
    0b01010,  //  * *
    0b01010,  //  * *
    0b10001,  // *   *
    0b10001,  // *   *
    0b01110,  //  ***
    0b00000   //
};

static const uint8_t SUN_CHAR[8] PROGMEM = {
    0b00100,  //   *
    0b10101,  // * * *
    0b01110,  //  ***
    0b11111,  // *****
    0b01110,  //  ***
    0b10101,  // * * *
    0b00100,  //   *
    0b00000   //
};

static const uint8_t WARN_CHAR[8] PROGMEM = {
    0b00100,  //   *
    0b00100,  //   *
    0b01010,  //  * *
    0b01110,  //  ***
    0b11011,  // ** **
    0b11111,  // *****
    0b11111,  // *****
    0b00000   //
};

// ============================================================
// Khởi tạo LCD
// ============================================================
void LcdDriver::init() {
    _lcd.init();
    _lcd.backlight();

    // Tạo các ký tự tùy chỉnh
    _createCustomChars();

    // Hiển thị thông báo khởi động
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(F("SMART IRRIGATION"));
    _lcd.setCursor(0, 1);
    _lcd.print(F("  Dang khoi tao "));

    Serial.println(F("[LCD] Khoi tao LCD thanh cong"));
}

// ============================================================
// In text tại vị trí cụ thể
// ============================================================
void LcdDriver::printAt(uint8_t col, uint8_t row, const String &text) {
    if (row > 1 || col > 15) return;
    _lcd.setCursor(col, row);
    _lcd.print(text);
}

// ============================================================
// In text trên toàn bộ 1 hàng (tự pad khoảng trắng để xóa ký tự cũ)
// ============================================================
void LcdDriver::printLine(uint8_t row, const String &text) {
    if (row > 1) return;

    // Cắt chuỗi nếu dài hơn 16 ký tự
    String padded = text;
    while (padded.length() < LCD_COLS) {
        padded += ' '; // Pad khoảng trắng để xóa ký tự thừa từ nội dung cũ
    }

    _lcd.setCursor(0, row);
    _lcd.print(padded.substring(0, LCD_COLS));
}

// ============================================================
// Xóa 1 hàng
// ============================================================
void LcdDriver::clearLine(uint8_t row) {
    printLine(row, "");
}

// ============================================================
// Xóa toàn bộ LCD
// ============================================================
void LcdDriver::clear() {
    _lcd.clear();
}

// ============================================================
// Bật nhấp nháy cảnh báo
// ============================================================
void LcdDriver::startBlink() {
    if (!_blinking) {
        _blinking = true;
        _lastBlinkTime = millis();
        Serial.println(F("[LCD] Bat nhap nhay canh bao"));
    }
}

// ============================================================
// Tắt nhấp nháy, bật backlight cố định
// ============================================================
void LcdDriver::stopBlink() {
    if (_blinking) {
        _blinking = false;
        _blinkState = true;
        _lcd.backlight(); // Đảm bảo backlight bật
        Serial.println(F("[LCD] Tat nhap nhay canh bao"));
    }
}

// ============================================================
// Cập nhật nhấp nháy (gọi trong loop - non-blocking)
// ============================================================
void LcdDriver::updateBlink() {
    if (!_blinking) return;

    unsigned long now = millis();
    if (now - _lastBlinkTime >= LCD_BLINK_INTERVAL_MS) {
        _lastBlinkTime = now;
        _blinkState = !_blinkState;

        if (_blinkState) {
            _lcd.backlight();
        } else {
            _lcd.noBacklight();
        }
    }
}

// ============================================================
// In ký tự tùy chỉnh
// ============================================================
void LcdDriver::printCustomChar(uint8_t col, uint8_t row, uint8_t charIndex) {
    if (row > 1 || col > 15 || charIndex > 7) return;
    _lcd.setCursor(col, row);
    _lcd.write(charIndex);
}

// ============================================================
// Private: Tạo ký tự tùy chỉnh từ PROGMEM
// ============================================================
void LcdDriver::_createCustomChars() {
    uint8_t buffer[8];

    // Icon máy bơm
    memcpy_P(buffer, PUMP_CHAR, 8);
    _lcd.createChar(CHAR_PUMP, buffer);

    // Icon giọt nước
    memcpy_P(buffer, DROP_CHAR, 8);
    _lcd.createChar(CHAR_DROP, buffer);

    // Icon mặt trời
    memcpy_P(buffer, SUN_CHAR, 8);
    _lcd.createChar(CHAR_SUN, buffer);

    // Icon cảnh báo
    memcpy_P(buffer, WARN_CHAR, 8);
    _lcd.createChar(CHAR_WARN, buffer);
}
