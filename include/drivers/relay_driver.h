/**
 * @file relay_driver.h
 * @brief Driver điều khiển relay bơm nước 5V
 * 
 * Tính năng:
 * - Bật/tắt relay với logging serial
 * - Theo dõi thời gian relay đã bật (pump uptime)
 * - Toggle (đảo trạng thái) cho chế độ manual
 * - Giới hạn thời gian bật tối đa (bảo vệ phần cứng)
 */

#ifndef RELAY_DRIVER_H
#define RELAY_DRIVER_H

#include <Arduino.h>
#include "config.h"

class RelayDriver {
public:
    /**
     * @brief Khởi tạo chân relay (OUTPUT, mặc định TẮT)
     */
    void init();

    /**
     * @brief Bật relay (bơm chạy)
     */
    void on();

    /**
     * @brief Tắt relay (bơm dừng)
     */
    void off();

    /**
     * @brief Đảo trạng thái relay (dùng cho nút nhấn manual)
     */
    void toggle();

    /**
     * @brief Kiểm tra relay có đang bật không
     * @return true nếu relay đang BẬT
     */
    bool isOn() const;

    /**
     * @brief Lấy thời gian relay đã bật liên tục (giây)
     * @return Số giây relay đã bật, 0 nếu đang tắt
     */
    uint32_t getOnDurationSec() const;

    /**
     * @brief Reset bộ đếm thời gian bật (gọi khi bắt đầu phiên tưới mới)
     */
    void resetOnTimer();

private:
    bool _isOn = false;                     // Trạng thái hiện tại
    unsigned long _onStartTime = 0;         // Thời điểm bắt đầu bật (millis)
};

#endif // RELAY_DRIVER_H
