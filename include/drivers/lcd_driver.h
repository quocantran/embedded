
#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

class LcdDriver {
public:
    void init();

    void printAt(uint8_t col, uint8_t row, const String &text);

    void printLine(uint8_t row, const String &text);

    void clearLine(uint8_t row);

    void clear();

    void startBlink();

    void stopBlink();

    void updateBlink();

    void printCustomChar(uint8_t col, uint8_t row, uint8_t charIndex);

    // Chỉ số ký tự tùy chỉnh
    static const uint8_t CHAR_PUMP = 0;     // Icon bơm nước
    static const uint8_t CHAR_DROP = 1;     // Icon giọt nước
    static const uint8_t CHAR_SUN  = 2;     // Icon mặt trời
    static const uint8_t CHAR_WARN = 3;     // Icon cảnh báo

private:
    LiquidCrystal_I2C _lcd = LiquidCrystal_I2C(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

    bool _blinking = false;             // Đang trong chế độ nhấp nháy?
    bool _blinkState = true;            // Trạng thái hiện tại (bật/tắt)
    unsigned long _lastBlinkTime = 0;   // Thời điểm blink cuối cùng

    void _createCustomChars();
};

#endif // LCD_DRIVER_H
