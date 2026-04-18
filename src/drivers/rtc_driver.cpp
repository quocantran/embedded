/**
 * @file rtc_driver.cpp
 * @brief Triển khai driver RTC DS1307 + đồng bộ NTP
 * 
 * Ưu tiên thời gian:
 * 1. NTP (internet) → sync lại RTC chip DS1307 luôn
 * 2. RTC (DS1307)   → dùng khi không có internet  
 * 3. Thời gian biên dịch → fallback cuối cùng
 */

#include "drivers/rtc_driver.h"
#include <time.h>

// ============================================================
// Khởi tạo RTC
// ============================================================
bool RtcDriver::init() {
    if (!_rtc.begin()) {
        Serial.println(F("[RTC] LOI: Khong tim thay DS1307 tren I2C!"));
        _initialized = false;
        return false;
    }

    // Nếu RTC chưa chạy (pin mới, mất nguồn), đặt thời gian biên dịch tạm
    if (!_rtc.isrunning()) {
        Serial.println(F("[RTC] RTC chua chay - dat thoi gian bien dich tam"));
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    _initialized = true;
    
    // In thời gian hiện tại từ RTC để debug  
    DateTime now = _rtc.now();
    Serial.printf("[RTC] Thoi gian RTC: %02d/%02d/%04d %02d:%02d:%02d\n",
                  now.day(), now.month(), now.year(),
                  now.hour(), now.minute(), now.second());

    return true;
}

// ============================================================
// Đồng bộ thời gian từ NTP (gọi SAU khi WiFi kết nối)
// ============================================================
bool RtcDriver::syncFromNTP() {
    Serial.println(F("[RTC] Dang dong bo NTP..."));

    // Cấu hình SNTP client của ESP32
    configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER);

    // Chờ tối đa 10 giây để lấy thời gian NTP
    struct tm timeinfo;
    int retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 10) {
        delay(1000);
        retry++;
        Serial.printf("[RTC] Cho NTP... %d/10\n", retry);
    }

    if (retry >= 10) {
        Serial.println(F("[RTC] CANH BAO: Khong the dong bo NTP - dung gio RTC cu"));
        _ntpSynced = false;
        return false;
    }

    // NTP thành công! Cập nhật RTC DS1307 chip
    DateTime ntpTime(
        timeinfo.tm_year + 1900,  // tm_year tính từ 1900
        timeinfo.tm_mon + 1,       // tm_mon 0-11
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );

    if (_initialized) {
        _rtc.adjust(ntpTime); // Ghi thời gian NTP vào chip RTC
        Serial.println(F("[RTC] Da cap nhat RTC chip tu NTP"));
    }

    _ntpSynced = true;

    Serial.printf("[RTC] NTP thanh cong: %02d/%02d/%04d %02d:%02d:%02d (GMT+7)\n",
                  ntpTime.day(), ntpTime.month(), ntpTime.year(),
                  ntpTime.hour(), ntpTime.minute(), ntpTime.second());

    return true;
}

// ============================================================
// Đọc thời gian vào SensorData
// Ưu tiên: NTP > RTC
// ============================================================
bool RtcDriver::readTime(SensorData &data) {
    // Nếu đã sync NTP → dùng thời gian ESP32 internal (chính xác hơn)
    if (_ntpSynced) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            data.hour = timeinfo.tm_hour;
            data.minute = timeinfo.tm_min;
            data.second = timeinfo.tm_sec;
            data.day = timeinfo.tm_mday;
            data.month = timeinfo.tm_mon + 1;
            data.year = timeinfo.tm_year + 1900;
            data.dayOfWeek = timeinfo.tm_wday; // 0=CN
            data.rtcValid = true;
            return true;
        }
        // NTP lỗi → fallback xuống RTC
    }

    // Fallback: đọc từ RTC chip DS1307
    if (!_initialized) {
        data.rtcValid = false;
        return false;
    }

    DateTime now = _rtc.now();

    // Kiểm tra giá trị hợp lệ (năm phải >= 2020)
    if (now.year() < 2020 || now.year() > 2099) {
        data.rtcValid = false;
        Serial.println(F("[RTC] CANH BAO: Thoi gian khong hop le"));
        return false;
    }

    data.hour = now.hour();
    data.minute = now.minute();
    data.second = now.second();
    data.day = now.day();
    data.month = now.month();
    data.year = now.year();
    data.dayOfWeek = now.dayOfTheWeek(); // 0=CN, 1=T2, ..., 6=T7
    data.rtcValid = true;

    return true;
}

// ============================================================
// Lấy Unix timestamp
// ============================================================
uint32_t RtcDriver::getUnixTime() {
    if (_ntpSynced) {
        time_t now;
        time(&now);
        return (uint32_t)now;
    }
    if (!_initialized) return 0;
    return _rtc.now().unixtime();
}

// ============================================================
// Kiểm tra RTC có đang chạy không
// ============================================================
bool RtcDriver::isRunning() {
    if (_ntpSynced) return true;
    if (!_initialized) return false;
    return _rtc.isrunning();
}

// ============================================================
// Kiểm tra đã sync NTP chưa
// ============================================================
bool RtcDriver::isNtpSynced() const {
    return _ntpSynced;
}
