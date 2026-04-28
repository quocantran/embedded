
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

private:
    LiquidCrystal_I2C _lcd = LiquidCrystal_I2C(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

    bool _blinking = false;             // Đang trong chế độ nhấp nháy?
    bool _blinkState = true;            // Trạng thái hiện tại (bật/tắt)
    unsigned long _lastBlinkTime = 0;
};

#endif // LCD_DRIVER_H
