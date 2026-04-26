
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

void LcdDriver::printAt(uint8_t col, uint8_t row, const String &text) {
    if (row > 1 || col > 15) return;
    _lcd.setCursor(col, row);
    _lcd.print(text);
}

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

void LcdDriver::clearLine(uint8_t row) {
    printLine(row, "");
}

void LcdDriver::clear() {
    _lcd.clear();
}

void LcdDriver::startBlink() {
    if (!_blinking) {
        _blinking = true;
        _lastBlinkTime = millis();
        Serial.println(F("[LCD] Bat nhap nhay canh bao"));
    }
}

void LcdDriver::stopBlink() {
    if (_blinking) {
        _blinking = false;
        _blinkState = true;
        _lcd.backlight(); // Đảm bảo backlight bật
        Serial.println(F("[LCD] Tat nhap nhay canh bao"));
    }
}

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

void LcdDriver::printCustomChar(uint8_t col, uint8_t row, uint8_t charIndex) {
    if (row > 1 || col > 15 || charIndex > 7) return;
    _lcd.setCursor(col, row);
    _lcd.write(charIndex);
}

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
