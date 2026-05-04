
#include "drivers/lcd_driver.h"

void LcdDriver::init() {
    _lcd.init();
    _lcd.backlight();

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
