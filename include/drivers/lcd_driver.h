/**
 * @file lcd_driver.h
 * @brief Driver cho LCD I2C 16x2 - hiển thị và cảnh báo nhấp nháy
 * 
 * Tính năng:
 * - Hiển thị text trên 2 hàng 16 ký tự
 * - Tạo ký tự tùy chỉnh (icon bơm, giọt nước)
 * - Nhấp nháy backlight khi cảnh báo
 * - Xóa từng hàng hoặc toàn bộ
 */

#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

class LcdDriver {
public:
    /**
     * @brief Khởi tạo LCD (init, bật backlight, tạo custom chars)
     */
    void init();

    /**
     * @brief In text lên LCD tại vị trí chỉ định
     * @param col Cột bắt đầu (0-15)
     * @param row Hàng (0 hoặc 1)
     * @param text Chuỗi cần in
     */
    void printAt(uint8_t col, uint8_t row, const String &text);

    /**
     * @brief In text trên toàn bộ 1 hàng (tự pad khoảng trắng)
     * @param row Hàng (0 hoặc 1)
     * @param text Chuỗi cần in (tối đa 16 ký tự)
     */
    void printLine(uint8_t row, const String &text);

    /**
     * @brief Xóa 1 hàng LCD
     * @param row Hàng cần xóa (0 hoặc 1)
     */
    void clearLine(uint8_t row);

    /**
     * @brief Xóa toàn bộ LCD
     */
    void clear();

    /**
     * @brief Bật chế độ nhấp nháy backlight cảnh báo
     * 
     * Khi bật, backlight sẽ nhấp nháy với tần suất LCD_BLINK_INTERVAL_MS.
     * Gọi updateBlink() trong loop() để cập nhật trạng thái nhấp nháy.
     */
    void startBlink();

    /**
     * @brief Tắt chế độ nhấp nháy, bật backlight cố định
     */
    void stopBlink();

    /**
     * @brief Cập nhật trạng thái nhấp nháy (gọi trong loop)
     * 
     * Non-blocking: kiểm tra millis() và toggle backlight nếu đến lúc.
     */
    void updateBlink();

    /**
     * @brief In ký tự tùy chỉnh (icon) tại vị trí
     * @param col Cột
     * @param row Hàng
     * @param charIndex Chỉ số ký tự tùy chỉnh (0-7)
     */
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

    /**
     * @brief Tạo các ký tự tùy chỉnh 5x8 pixel
     */
    void _createCustomChars();
};

#endif // LCD_DRIVER_H
