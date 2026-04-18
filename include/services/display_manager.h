/**
 * @file display_manager.h
 * @brief Quản lý hiển thị LCD - 1 trang duy nhất + cảnh báo khẩn
 * 
 * Hiển thị bình thường (LCD 16x2):
 * ┌────────────────┐
 * │ HH:MM  TT°C    │  ← Giờ + Nhiệt độ
 * │ D:XX% M:auto   │  ← Ẩm đất + chế độ hoạt động
 * └────────────────┘
 * 
 * Khi đất dưới ngưỡng nguy hiểm → hiển thị cảnh báo:
 * ┌────────────────┐
 * │TUOI NUOC NGAY! │
 * │DAT QUA KHO!    │
 * └────────────────┘
 * → Đèn nền LCD nhấp nháy
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "drivers/lcd_driver.h"

class DisplayManager {
public:
    /**
     * @brief Khởi tạo với tham chiếu đến LCD driver
     */
    void init(LcdDriver &lcd);

    /**
     * @brief Cập nhật hiển thị LCD
     * @param sensors Dữ liệu cảm biến
     * @param mode Chế độ hoạt động
     * @param state Trạng thái state machine
     * @param pumpOn Bơm đang chạy?
     * @param error Lỗi hiện tại
     * @param ipAddress Địa chỉ IP
     * @param dangerThreshold Ngưỡng nguy hiểm (%)
     */
    void update(const SensorData &sensors, OperationMode mode,
                SystemState state, bool pumpOn, ErrorCode error,
                const String &ipAddress, uint8_t dangerThreshold);

private:
    LcdDriver *_lcd = nullptr;
    unsigned long _lastUpdateTime = 0;

    /**
     * @brief Hiển thị trang chính: Giờ + Nhiệt độ + Ẩm đất + Chế độ
     */
    void _showMain(const SensorData &sensors, OperationMode mode);

    /**
     * @brief Hiển thị cảnh báo đất khô
     */
    void _showDroughtAlert();

    /**
     * @brief Hiển thị lỗi hệ thống
     */
    void _showError(ErrorCode error);
};

#endif // DISPLAY_MANAGER_H
