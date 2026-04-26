
#include "services/irrigation_service.h"

IrrigationDecision IrrigationService::evaluate(
    const SensorData &sensors,
    const SystemConfig &config,
    uint32_t lastWateringTime,
    uint16_t todayWaterUsed,
    uint32_t currentUnixTime
) {
    IrrigationDecision decision;
    decision.shouldWater = false;
    decision.level = WateringLevel::NONE;
    decision.durationSec = 0;
    decision.targetPulses = 0;
    decision.reason = "";

    // ─── Kiểm tra 1: Cảm biến có hợp lệ không? ───
    if (!sensors.soilValid) {
        decision.reason = "Cam bien dat loi - khong tuoi";
        return decision;
    }

    // ─── Kiểm tra 2: Hysteresis ───
    // Nếu đất > ngưỡng cao → CHẮC CHẮN không tưới
    if (sensors.soilPercent >= config.soilThresholdHigh) {
        _wasWatering = false;
        decision.reason = "Dat du am (" + String(sensors.soilPercent) + 
                         "% >= " + String(config.soilThresholdHigh) + "%)";
        return decision;
    }

    // Nếu đất nằm giữa 2 ngưỡng VÀ không đang tưới → không tưới
    if (sensors.soilPercent >= config.soilThresholdLow && !_wasWatering) {
        decision.reason = "Dat trong vung an toan (" + String(sensors.soilPercent) + "%)";
        return decision;
    }

    // ─── Kiểm tra 3: Cooldown ───
    // Nếu đất quá khô ở mức nguy hiểm, cho phép bỏ qua cooldown
    bool criticalDry = (sensors.soilPercent <= config.dangerThreshold);
    if (sensors.rtcValid && lastWateringTime > 0 && currentUnixTime > 0) {
        if (!_cooldownPassed(lastWateringTime, config.cooldownMinutes, currentUnixTime)) {
            if (!criticalDry) {
                decision.reason = "Chua qua thoi gian cho (" + 
                                 String(config.cooldownMinutes) + " phut)";
                return decision;
            }
            Serial.printf("[TUOI] Bo qua cooldown do dat nguy hiem: %d%% <= %d%%\n",
                          sensors.soilPercent, config.dangerThreshold);
        }
    }

    // ─── Kiểm tra 4: Ngân sách nước hàng ngày ───
    if (todayWaterUsed >= config.dailyWaterBudget) {
        decision.reason = "Het ngan sach nuoc hom nay (" + 
                         String(todayWaterUsed) + "/" + 
                         String(config.dailyWaterBudget) + "s)";
        return decision;
    }

    // ─── Kiểm tra 5: Giờ tưới (tránh giữa trưa) ───
    if (sensors.rtcValid && _isNoonPeriod(sensors.hour)) {
        decision.shouldWater = false;
        decision.level = WateringLevel::DEFER;
        decision.reason = "Hoan tuoi - dang gio trua (" + String(sensors.hour) + "h)";
        return decision;
    }

    // ─── ĐẤT CẦN TƯỚI → Tính mức độ tưới ───
    float temp = sensors.dhtValid ? sensors.temperature : 25.0f;
    float humid = sensors.dhtValid ? sensors.humidity : 50.0f;
    uint8_t hour = sensors.rtcValid ? sensors.hour : 12;

    WateringLevel level = _calculateLevel(sensors.soilPercent, temp, humid, hour);

    if (level == WateringLevel::NONE || level == WateringLevel::DEFER) {
        decision.level = level;
        decision.reason = "Khong can tuoi luc nay";
        return decision;
    }

    // ─── Tính thời gian tưới và số xung ───
    uint8_t pulses = _levelToPulses(level);
    uint16_t duration = pulses * WATERING_PULSE_SEC;

    // Giới hạn bởi ngân sách còn lại
    uint16_t budgetLeft = config.dailyWaterBudget - todayWaterUsed;
    if (duration > budgetLeft) {
        duration = budgetLeft;
        pulses = duration / WATERING_PULSE_SEC;
        if (pulses == 0) pulses = 1; // Tối thiểu 1 xung
    }

    decision.shouldWater = true;
    decision.level = level;
    decision.durationSec = duration;
    decision.targetPulses = pulses;
    decision.reason = "Dat kho " + String(sensors.soilPercent) + "%, " +
                     String(temp, 1) + "C, " + String(humid, 0) + "%RH → " +
                     String(pulses) + " xung";
    _wasWatering = true;

    Serial.printf("[TUOI] Quyet dinh: %s\n", decision.reason.c_str());
    return decision;
}

bool IrrigationService::checkHysteresis(int soilPercent, uint8_t lowThresh,
                                         uint8_t highThresh, bool currentlyWatering) {
    if (soilPercent < lowThresh) {
        // Dưới ngưỡng thấp → NÊN tưới
        return true;
    }
    if (soilPercent >= highThresh) {
        // Trên ngưỡng cao → KHÔNG tưới
        return false;
    }
    // Giữa 2 ngưỡng → giữ nguyên trạng thái
    return currentlyWatering;
}

bool IrrigationService::_isNoonPeriod(uint8_t hour) {
    return (hour >= 11 && hour <= 14);
}

bool IrrigationService::_cooldownPassed(uint32_t lastWateringTime, 
                                         uint8_t cooldownMin,
                                         uint32_t currentTime) {
    if (lastWateringTime == 0) return true; // Chưa tưới lần nào
    
    uint32_t cooldownSec = (uint32_t)cooldownMin * 60;
    return (currentTime - lastWateringTime) >= cooldownSec;
}

WateringLevel IrrigationService::_calculateLevel(int soilPercent, float temperature,
                                                   float humidity, uint8_t hour) {
    // ─── Đất RẤT khô (< 20%) ───
    if (soilPercent < 20) {
        if (temperature > 35.0f && humidity < 40.0f) {
            // Nóng + khô → tưới DÀI
            return WateringLevel::LONG;
        }
        if (temperature > 30.0f) {
            return WateringLevel::LONG;
        }
        // Đất rất khô, điều kiện bình thường → tưới VỪA
        return WateringLevel::MEDIUM;
    }

    // ─── Đất khô vừa (20% - ngưỡng thấp) ───
    if (soilPercent < 40) {
        if (humidity > 70.0f) {
            // Ẩm KK cao → tưới NGẮN (nước bay hơi chậm)
            return WateringLevel::SHORT;
        }
        if (temperature > 30.0f && humidity < 50.0f) {
            // Nóng + khô KK → tưới VỪA
            return WateringLevel::MEDIUM;
        }
        // Điều kiện trung bình
        return WateringLevel::SHORT;
    }

    // ─── Đất ẩm vừa (>= 40%) → không cần tưới ───
    return WateringLevel::NONE;
}

uint8_t IrrigationService::_levelToPulses(WateringLevel level) {
    switch (level) {
        case WateringLevel::SHORT:  return 2;   // 2 × 30s = 60s
        case WateringLevel::MEDIUM: return 3;   // 3 × 30s = 90s
        case WateringLevel::LONG:   return 6;   // 6 × 30s = 180s
        default:                    return 0;
    }
}
