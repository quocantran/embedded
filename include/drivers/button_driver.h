/**
 * @file button_driver.h
 * @brief Driver nút nhấn vật lý với debounce và phát hiện long press
 * 
 * Nút nhấn nối giữa GPIO và GND, sử dụng internal pull-up của ESP32.
 * 
 * Hành vi:
 * - Nhấn ngắn (< 3 giây, phát hiện khi THẢ):
 *     Trong MANUAL mode → bật/tắt bơm
 * - Nhấn giữ (>= 3 giây, phát hiện khi ĐANG GIỮ):
 *     → Chuyển sang MANUAL mode (hoặc thoát về AUTO)
 * 
 * Kết nối phần cứng trên breadboard 830 lỗ:
 * - Một chân nút nhấn → GPIO4 (ESP32)
 * - Chân còn lại → GND (ESP32)
 * - KHÔNG cần điện trở pull-up ngoài (dùng internal pull-up)
 */

#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>
#include "config.h"
#include "types.h"

class ButtonDriver {
public:
    /**
     * @brief Khởi tạo chân nút nhấn (INPUT_PULLUP)
     */
    void init();

    /**
     * @brief Cập nhật trạng thái nút nhấn (GỌI MỖI VÒNG LOOP)
     * 
     * Hàm này xử lý debounce và phát hiện sự kiện nhấn.
     * Phải được gọi liên tục trong loop() để hoạt động chính xác.
     */
    void update();

    /**
     * @brief Lấy sự kiện nút nhấn và xóa sự kiện đang chờ
     * @return ButtonEvent::SHORT_PRESS, LONG_PRESS, hoặc NONE
     * 
     * Mỗi sự kiện chỉ trả về 1 lần. Sau khi đọc, sự kiện được xóa.
     */
    ButtonEvent getEvent();

private:
    bool _lastRawState = HIGH;          // Trạng thái đọc thô gần nhất (debug)
    bool _stableState = HIGH;           // Trạng thái ổn định sau debounce
    bool _lastStableState = HIGH;       // Trạng thái ổn định trước đó

    unsigned long _pressStartTime = 0;  // Thời điểm bắt đầu nhấn
    unsigned long _lastHoldLogTime = 0; // Log tien trinh nhan giu moi 500ms
    unsigned long _lastSampleTime = 0;  // Moc thoi gian lay mau cho bo loc

    uint8_t _integrator = 0;            // Bo dem tich phan chong nhieu

    bool _longPressHandled = false;     // Long press đã được xử lý cho lần nhấn này?
    ButtonEvent _pendingEvent = ButtonEvent::NONE;  // Sự kiện đang chờ xử lý
};

#endif // BUTTON_DRIVER_H
