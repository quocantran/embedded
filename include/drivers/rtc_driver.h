/**
 * @file rtc_driver.h
 * @brief Driver cho module RTC DS1307 + đồng bộ NTP từ internet
 * 
 * Ưu tiên thời gian:
 * 1. NTP (internet) → chính xác nhất, đồng bộ lại RTC
 * 2. RTC (DS1307)   → dùng khi không có internet
 * 3. Thời gian biên dịch → fallback cuối cùng
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <Arduino.h>
#include <RTClib.h>
#include "types.h"
#include "config.h"

class RtcDriver {
public:
    /**
     * @brief Khởi tạo RTC DS1307
     * @return true nếu RTC phản hồi thành công
     */
    bool init();

    /**
     * @brief Đồng bộ thời gian từ NTP (gọi SAU khi WiFi kết nối)
     * Nếu thành công → cập nhật lại RTC DS1307 luôn
     * @return true nếu NTP sync thành công
     */
    bool syncFromNTP();

    /**
     * @brief Đọc thời gian hiện tại vào SensorData
     * Ưu tiên NTP time nếu đã sync, nếu không dùng RTC
     * @param data [out] Cập nhật các trường thời gian
     * @return true nếu đọc thành công
     */
    bool readTime(SensorData &data);

    /**
     * @brief Lấy Unix timestamp hiện tại
     * @return Số giây kể từ 1/1/1970 (hoặc 0 nếu RTC lỗi)
     */
    uint32_t getUnixTime();

    /**
     * @brief Kiểm tra RTC có đang hoạt động không
     * @return true nếu RTC đang chạy bình thường
     */
    bool isRunning();

    /**
     * @brief Kiểm tra đã sync NTP thành công chưa
     */
    bool isNtpSynced() const;

private:
    RTC_DS1307 _rtc;                // Đối tượng RTC từ thư viện RTClib
    bool _initialized = false;      // Cờ đã khởi tạo thành công
    bool _ntpSynced = false;        // Đã sync NTP thành công
};

#endif // RTC_DRIVER_H
