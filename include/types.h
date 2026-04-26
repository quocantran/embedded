
#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include "config.h"


enum class SystemState : uint8_t {
    IDLE = 0,           // Chờ - không làm gì, đợi đến chu kỳ tiếp
    READ_SENSOR,        // Đang đọc tất cả cảm biến
    ANALYZE,            // Phân tích dữ liệu, kiểm tra an toàn
    DECIDE,             // Ra quyết định tưới (chỉ trong AUTO/SCHEDULE)
    WATERING,           // Đang tưới progressive (bơm 30s → kiểm tra → lặp)
    LOGGING,            // Ghi log dữ liệu vào EEPROM/Serial
    ERROR               // Lỗi hệ thống - chờ phục hồi
};

enum class OperationMode : uint8_t {
    AUTO = 0,           // Tự động hoàn toàn
    MANUAL = 1,         // Thủ công qua nút nhấn
    SCHEDULE = 2        // Theo lịch cài đặt
};

enum class WateringLevel : uint8_t {
    NONE = 0,           // Không tưới
    SHORT,              // Tưới ngắn: 1-2 xung (30-60s)
    MEDIUM,             // Tưới vừa: 3 xung (90s)
    LONG,               // Tưới dài: 6 xung (180s)
    DEFER               // Hoãn tưới (trưa nắng 11h-14h)
};

enum class WateringSubState : uint8_t {
    PUMP_ON = 0,        // Bơm đang chạy (1 xung 30s)
    CHECK_SOIL,         // Tắt bơm, đợi + đọc lại cảm biến đất
    COMPLETE            // Hoàn thành phiên tưới
};

enum class ButtonEvent : uint8_t {
    NONE = 0,           // Không có sự kiện
    SHORT_PRESS,        // Nhấn ngắn (< 3s, phát hiện khi thả)
    LONG_PRESS          // Nhấn giữ (>= 3s, phát hiện khi đang giữ)
};

enum class ErrorCode : uint8_t {
    NONE = 0,           // Không có lỗi
    DHT_FAIL,           // Cảm biến DHT11 không phản hồi / giá trị NaN
    SOIL_FAIL,          // Cảm biến đất giá trị ngoài phạm vi
    RTC_FAIL,           // Module RTC DS1307 không hoạt động
    EEPROM_FAIL,        // Đọc/ghi EEPROM thất bại (CRC lỗi)
    PUMP_FAIL,          // Bơm chạy nhưng đất không tăng ẩm (bơm hỏng/hết nước?)
    PUMP_TIMEOUT        // Bơm vượt quá thời gian cho phép
};


struct SensorData {
    // --- Dữ liệu DHT11 ---
    float temperature;          // Nhiệt độ (°C), NaN nếu lỗi
    float humidity;             // Độ ẩm không khí (%), NaN nếu lỗi

    // --- Dữ liệu cảm biến đất ---
    int soilRaw;                // Giá trị ADC thô (0-4095)
    int soilPercent;            // Độ ẩm đất đã hiệu chuẩn (0-100%)

    // --- Dữ liệu thời gian từ RTC ---
    uint8_t hour;               // Giờ (0-23)
    uint8_t minute;             // Phút (0-59)
    uint8_t second;             // Giây (0-59)
    uint8_t day;                // Ngày trong tháng (1-31)
    uint8_t month;              // Tháng (1-12)
    uint16_t year;              // Năm (VD: 2026)
    uint8_t dayOfWeek;          // Ngày trong tuần (0=CN, 1=T2, ..., 6=T7)

    // --- Cờ trạng thái ---
    bool dhtValid;              // true nếu DHT đọc thành công
    bool soilValid;             // true nếu giá trị đất hợp lệ
    bool rtcValid;              // true nếu RTC hoạt động bình thường
};

struct __attribute__((packed)) SystemConfig {
    uint16_t soilDryRaw;        // Giá trị ADC khi đất khô (hiệu chuẩn)
    uint16_t soilWetRaw;        // Giá trị ADC khi đất ướt (hiệu chuẩn)
    uint8_t soilThresholdLow;   // Ngưỡng thấp (%) - dưới mức này: xét tưới
    uint8_t soilThresholdHigh;  // Ngưỡng cao (%) - trên mức này: KHÔNG tưới
    uint8_t dangerThreshold;    // Ngưỡng nguy hiểm (%) - cảnh báo nhấp nháy
    uint8_t maxPumpTimeMin;     // Thời gian bơm tối đa (phút)
    uint16_t dailyWaterBudget;  // Ngân sách nước/ngày (giây)
    uint8_t cooldownMinutes;    // Thời gian chờ giữa 2 lần tưới (phút)
    uint8_t manualTimeoutMin;   // Timeout chế độ manual (phút)
    uint8_t mode;               // Chế độ hoạt động (cast sang OperationMode)
};

struct __attribute__((packed)) ScheduleEntry {
    uint8_t enabled;            // 1 = bật, 0 = tắt
    uint8_t hour;               // Giờ kích hoạt (0-23)
    uint8_t minute;             // Phút kích hoạt (0-59)
    uint8_t daysMask;           // Bit mask ngày trong tuần
    uint16_t durationSec;       // Thời gian tưới gợi ý (giây)
    uint8_t reserved[2];        // Dự phòng (căn chỉnh 8 bytes/mục)
};

struct __attribute__((packed)) RuntimeData {
    uint32_t lastWateringTime;  // Unix timestamp lần tưới gần nhất
    uint16_t todayWateringSec;  // Tổng thời gian đã tưới hôm nay (giây)
    uint8_t todayDate;          // Ngày hiện tại (1-31) để reset budget
    uint8_t crc;                // CRC8 kiểm tra toàn vẹn
};

struct __attribute__((packed)) AdaptiveData {
    float avgResponseRate;      // Tốc độ phản hồi trung bình (%/giây tưới)
    uint16_t totalSessions;     // Tổng số phiên tưới đã ghi nhận
    uint8_t dryTrendCounter;    // Đếm xu hướng khô liên tiếp (0-255)
    uint8_t crc;                // CRC8 kiểm tra toàn vẹn
};

struct IrrigationDecision {
    bool shouldWater;           // Có nên tưới không?
    WateringLevel level;        // Mức độ tưới
    uint16_t durationSec;       // Thời gian tưới đề xuất (giây)
    uint8_t targetPulses;       // Số xung progressive cần thực hiện
    String reason;              // Lý do quyết định (tiếng Việt)
};

struct SystemStatus {
    SystemState state;          // Trạng thái state machine
    OperationMode mode;         // Chế độ hoạt động
    SensorData sensors;         // Dữ liệu cảm biến mới nhất
    bool pumpRunning;           // Bơm đang chạy?
    uint32_t pumpOnTimeSec;     // Thời gian bơm đã chạy phiên này (giây)
    uint16_t todayWaterUsed;    // Nước đã dùng hôm nay (giây)
    uint16_t dailyBudgetLeft;   // Ngân sách nước còn lại (giây)
    ErrorCode lastError;        // Mã lỗi gần nhất
    String ipAddress;           // Địa chỉ IP WiFi AP
};

#endif // TYPES_H
