
#include "services/scheduler_service.h"

IrrigationDecision SchedulerService::checkSchedules(
    const SensorData &sensors,
    const ScheduleEntry schedules[MAX_SCHEDULES],
    const SystemConfig &config
) {
    IrrigationDecision decision;
    decision.shouldWater = false;
    decision.level = WateringLevel::NONE;
    decision.durationSec = 0;
    decision.targetPulses = 0;
    decision.reason = "";

    // Kiểm tra RTC có hợp lệ không
    if (!sensors.rtcValid) {
        decision.reason = "RTC loi - khong the kiem tra lich";
        return decision;
    }

    // Tránh kiểm tra lặp trong cùng 1 phút
    if (sensors.minute == _lastCheckMinute) {
        return decision; // Đã kiểm tra phút này rồi
    }
    _lastCheckMinute = sensors.minute;

    // Duyệt qua tất cả lịch tưới
    for (uint8_t i = 0; i < MAX_SCHEDULES; i++) {
        if (_shouldTrigger(schedules[i], sensors.hour, sensors.minute, 
                          sensors.dayOfWeek, i)) {
            
            Serial.printf("[LICH] Lich tuoi #%d kich hoat luc %02d:%02d\n",
                         i, sensors.hour, sensors.minute);

            // Điều chỉnh thời gian tưới theo độ ẩm đất thực tế
            uint16_t adjustedSec = _adjustDuration(schedules[i].durationSec,
                                                    sensors.soilPercent, config);

            if (adjustedSec == 0) {
                if (schedules[i].durationSec == 0) {
                    decision.reason = "Lich #" + String(i) + " co thoi luong = 0 giay";
                    Serial.printf("[LICH] Bo qua - lich #%d co thoi luong 0 giay\n", i);
                } else {
                    decision.reason = "Lich #" + String(i) + " kich hoat nhung dat du am (" +
                                     String(sensors.soilPercent) + "%)";
                    Serial.printf("[LICH] Bo qua - dat du am %d%%\n", sensors.soilPercent);
                }
                _triggered[i] = true; // Đánh dấu đã xử lý
                continue;
            }

            // Tính số xung progressive
            uint8_t pulses = adjustedSec / WATERING_PULSE_SEC;
            if (pulses == 0) pulses = 1;

            decision.shouldWater = true;
            decision.durationSec = adjustedSec;
            decision.targetPulses = pulses;
            decision.reason = "Lich #" + String(i) + " → " + String(adjustedSec) + 
                             "s (" + String(pulses) + " xung)";

            // Xác định mức độ
            if (pulses >= 5) decision.level = WateringLevel::LONG;
            else if (pulses >= 3) decision.level = WateringLevel::MEDIUM;
            else decision.level = WateringLevel::SHORT;

            _triggered[i] = true;
            Serial.printf("[LICH] Tuoi %d giay (%d xung)\n", adjustedSec, pulses);
            return decision; // Chỉ xử lý 1 lịch tại 1 thời điểm
        }
    }

    return decision;
}

void SchedulerService::resetDailyFlags() {
    for (int i = 0; i < MAX_SCHEDULES; i++) {
        _triggered[i] = false;
    }
    _lastCheckMinute = 255;
    Serial.println(F("[LICH] Reset co lich tuoi cho ngay moi"));
}

bool SchedulerService::_shouldTrigger(const ScheduleEntry &entry, uint8_t hour,
                                       uint8_t minute, uint8_t dayOfWeek, 
                                       uint8_t index) {
    // Kiểm tra lịch có bật không
    if (!entry.enabled) return false;

    // Kiểm tra đã kích hoạt chưa (tránh lặp)
    if (_triggered[index]) return false;

    // Kiểm tra giờ/phút khớp
    if (entry.hour != hour || entry.minute != minute) return false;

    // Kiểm tra ngày trong tuần (bit mask)
    // dayOfWeek: 0=CN, 1=T2, ..., 6=T7
    if (!(entry.daysMask & (1 << dayOfWeek))) return false;

    return true; // Tất cả điều kiện khớp → kích hoạt!
}

uint16_t SchedulerService::_adjustDuration(uint16_t requestedSec, int soilPercent,
                                            const SystemConfig &config) {
    // Đất đã đủ ẩm → không tưới
    if (soilPercent >= config.soilThresholdHigh) {
        return 0;
    }

    // Đất nằm giữa 2 ngưỡng → tưới 50% 
    if (soilPercent >= config.soilThresholdLow) {
        return requestedSec / 2;
    }

    // Đất dưới ngưỡng thấp → tưới đầy đủ
    return requestedSec;
}
