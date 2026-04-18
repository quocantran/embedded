/**
 * @file scheduler_service.h
 * @brief Dịch vụ quản lý lịch tưới theo thời gian
 * 
 * Hỗ trợ tối đa 4 lịch tưới, mỗi lịch có:
 * - Giờ/phút kích hoạt
 * - Ngày trong tuần (bit mask)
 * - Thời gian tưới gợi ý
 * 
 * Khi lịch kích hoạt, hệ thống KHÔNG tưới mù quáng:
 * - Đất đã ướt (> ngưỡng cao) → BỎ QUA
 * - Đất hơi ẩm (giữa 2 ngưỡng) → tưới NGẮN
 * - Đất khô (< ngưỡng thấp) → tưới ĐẦY ĐỦ
 */

#ifndef SCHEDULER_SERVICE_H
#define SCHEDULER_SERVICE_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

class SchedulerService {
public:
    /**
     * @brief Kiểm tra xem có lịch tưới nào cần kích hoạt không
     * @param sensors Dữ liệu cảm biến (giờ, ngày, độ ẩm đất)
     * @param schedules Mảng 4 lịch tưới
     * @param config Cấu hình hệ thống
     * @return Quyết định tưới (shouldWater=true nếu cần tưới)
     */
    IrrigationDecision checkSchedules(
        const SensorData &sensors,
        const ScheduleEntry schedules[MAX_SCHEDULES],
        const SystemConfig &config
    );

    /**
     * @brief Reset cờ đã kích hoạt cho ngày mới
     * 
     * Mỗi lịch chỉ kích hoạt 1 lần/ngày. Khi sang ngày mới,
     * reset tất cả cờ để lịch có thể kích hoạt lại.
     */
    void resetDailyFlags();

private:
    // Cờ đánh dấu lịch đã kích hoạt trong ngày hiện tại
    // Tránh kích hoạt lặp trong cùng 1 phút
    bool _triggered[MAX_SCHEDULES] = {false, false, false, false};
    uint8_t _lastCheckMinute = 255;  // Phút kiểm tra cuối (tránh lặp)

    /**
     * @brief Kiểm tra 1 lịch tưới cụ thể có cần kích hoạt không
     * @param entry Mục lịch tưới
     * @param hour Giờ hiện tại
     * @param minute Phút hiện tại
     * @param dayOfWeek Ngày trong tuần (0=CN)
     * @param index Chỉ số lịch (0-3)
     * @return true nếu lịch này cần kích hoạt BÂY GIỜ
     */
    bool _shouldTrigger(const ScheduleEntry &entry, uint8_t hour, 
                        uint8_t minute, uint8_t dayOfWeek, uint8_t index);

    /**
     * @brief Điều chỉnh thời gian tưới dựa trên độ ẩm đất hiện tại
     * @param requestedSec Thời gian tưới gợi ý từ lịch
     * @param soilPercent Độ ẩm đất hiện tại (%)
     * @param config Cấu hình hệ thống
     * @return Thời gian tưới điều chỉnh (giây), 0 nếu không cần tưới
     */
    uint16_t _adjustDuration(uint16_t requestedSec, int soilPercent,
                             const SystemConfig &config);
};

#endif // SCHEDULER_SERVICE_H
