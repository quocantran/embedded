
#include "services/adaptive_engine.h"

void AdaptiveEngine::recordSession(int soilBefore, int soilAfter,
                                    uint16_t durationSec, AdaptiveData &adaptive) {
    if (durationSec == 0) return;

    // Tính tốc độ phản hồi: % tăng / giây tưới
    float increase = (float)(soilAfter - soilBefore);
    float responseRate = increase / (float)durationSec;

    // Cập nhật trung bình trượt (EMA - Exponential Moving Average)
    // Hệ số alpha = 0.3: ưu tiên dữ liệu mới nhưng vẫn giữ lịch sử
    if (adaptive.totalSessions == 0) {
        // Phiên đầu tiên → lấy trực tiếp
        adaptive.avgResponseRate = responseRate;
    } else {
        // EMA: new_avg = alpha * new_value + (1 - alpha) * old_avg
        adaptive.avgResponseRate = 0.3f * responseRate + 0.7f * adaptive.avgResponseRate;
    }

    adaptive.totalSessions++;

    Serial.printf("[THICH UNG] Phien #%d: dat tang %d%% trong %ds (rate=%.4f, avg=%.4f)\n",
                  adaptive.totalSessions, (int)increase, durationSec,
                  responseRate, adaptive.avgResponseRate);
}

uint16_t AdaptiveEngine::adjustDuration(uint16_t requestedSec, 
                                         const AdaptiveData &adaptive) {
    // Cần ít nhất 3 phiên dữ liệu để bắt đầu điều chỉnh
    if (adaptive.totalSessions < 3) {
        return requestedSec; // Chưa đủ dữ liệu → giữ nguyên
    }

    // Nếu đất phản hồi tốt (rate > 0.5%/s) → giảm 20% thời gian
    if (adaptive.avgResponseRate > 0.5f) {
        uint16_t adjusted = (uint16_t)(requestedSec * 0.8f);
        Serial.printf("[THICH UNG] Dat phan hoi tot → giam %ds → %ds\n", 
                      requestedSec, adjusted);
        return max(adjusted, (uint16_t)DEFAULT_WATERING_PULSE_SEC); // Tối thiểu 1 xung
    }

    // Nếu đất phản hồi kém (rate < 0.1%/s) → tăng 30% thời gian
    if (adaptive.avgResponseRate < 0.1f && adaptive.avgResponseRate > 0) {
        uint16_t adjusted = (uint16_t)(requestedSec * 1.3f);
        // Giới hạn tối đa = MAX_PUMP_TIME_SEC
        adjusted = min(adjusted, (uint16_t)MAX_PUMP_TIME_SEC);
        Serial.printf("[THICH UNG] Dat phan hoi kem → tang %ds → %ds\n", 
                      requestedSec, adjusted);
        return adjusted;
    }

    // Phản hồi bình thường → giữ nguyên
    return requestedSec;
}

bool AdaptiveEngine::updateDryTrend(int soilPercent, AdaptiveData &adaptive) {
    if (_lastSoilPercent < 0) {
        // Lần đo đầu tiên → chưa có gì để so sánh
        _lastSoilPercent = soilPercent;
        return false;
    }

    if (soilPercent < _lastSoilPercent) {
        // Đất giảm ẩm so với lần trước → tăng counter
        adaptive.dryTrendCounter++;
        Serial.printf("[THICH UNG] Xu huong kho: %d%% → %d%% (counter=%d)\n",
                      _lastSoilPercent, soilPercent, adaptive.dryTrendCounter);
    } else {
        // Đất tăng hoặc giữ nguyên → reset counter
        if (adaptive.dryTrendCounter > 0) {
            Serial.printf("[THICH UNG] Xu huong kho reset (dat tang %d%% → %d%%)\n",
                          _lastSoilPercent, soilPercent);
        }
        adaptive.dryTrendCounter = 0;
    }

    _lastSoilPercent = soilPercent;

    // Cảnh báo nếu xu hướng khô kéo dài > 5 lần đo liên tiếp
    bool dryAlert = (adaptive.dryTrendCounter > 5);
    if (dryAlert) {
        Serial.println(F("[THICH UNG] CANH BAO: Xu huong kho keo dai!"));
    }
    return dryAlert;
}

uint16_t AdaptiveEngine::checkBudget(uint16_t todayUsed, uint16_t dailyBudget) {
    if (todayUsed >= dailyBudget) {
        Serial.println(F("[THICH UNG] Het ngan sach nuoc hom nay!"));
        return 0;
    }
    return dailyBudget - todayUsed;
}

void AdaptiveEngine::resetDailyBudget(RuntimeData &runtime, uint8_t currentDay) {
    if (runtime.todayDate != currentDay) {
        Serial.printf("[THICH UNG] Ngay moi (%d → %d) - reset ngan sach nuoc\n",
                      runtime.todayDate, currentDay);
        runtime.todayDate = currentDay;
        runtime.todayWateringSec = 0;
    }
}
