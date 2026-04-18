/**
 * @file safety_manager.cpp
 * @brief Triển khai quản lý an toàn hệ thống
 * 
 * 5 lớp bảo vệ:
 * 1. Cảm biến lỗi → ERROR state
 * 2. Bơm quá thời gian → tắt bơm cưỡng bức
 * 3. Bơm không hiệu quả → cảnh báo (bơm hỏng/hết nước?)
 * 4. Manual mode timeout → tự về AUTO
 * 5. Đất đủ ẩm → dừng bơm
 */

#include "services/safety_manager.h"

// ============================================================
// Kiểm tra an toàn cảm biến
// ============================================================
ErrorCode SafetyManager::checkSensors(uint8_t dhtFailCount, uint8_t soilFailCount) {
    // DHT lỗi liên tiếp quá giới hạn
    if (dhtFailCount >= SENSOR_FAIL_MAX_COUNT) {
        _lastError = ErrorCode::DHT_FAIL;
        Serial.printf("[AN TOAN] DHT loi %d lan lien tiep!\n", dhtFailCount);
        return ErrorCode::DHT_FAIL;
    }

    // Cảm biến đất lỗi liên tiếp quá giới hạn
    if (soilFailCount >= SENSOR_FAIL_MAX_COUNT) {
        _lastError = ErrorCode::SOIL_FAIL;
        Serial.printf("[AN TOAN] Cam bien dat loi %d lan lien tiep!\n", soilFailCount);
        return ErrorCode::SOIL_FAIL;
    }

    return ErrorCode::NONE;
}

// ============================================================
// Kiểm tra timeout bơm
// ============================================================
bool SafetyManager::checkPumpTimeout(uint32_t pumpOnSec, uint8_t maxPumpTimeMin) {
    uint32_t maxSec = (uint32_t)maxPumpTimeMin * 60;

    if (pumpOnSec >= maxSec) {
        _lastError = ErrorCode::PUMP_TIMEOUT;
        Serial.printf("[AN TOAN] BOM QUA THOI GIAN! (%lu >= %lu giay)\n", 
                      pumpOnSec, maxSec);
        return true; // CẦN TẮT BƠM NGAY
    }

    return false;
}

// ============================================================
// Kiểm tra hiệu quả bơm
// ============================================================
bool SafetyManager::checkPumpEffectiveness(int soilBefore, int soilAfter, 
                                            uint32_t pumpOnSec) {
    // Chỉ kiểm tra sau khi bơm đã chạy đủ lâu (PUMP_FAIL_CHECK_SEC)
    if (pumpOnSec < PUMP_FAIL_CHECK_SEC) {
        return false; // Chưa đủ thời gian để đánh giá
    }

    // Nếu đất không tăng ít nhất SOIL_RESPONSE_MIN_PCT sau 60s tưới
    int increase = soilAfter - soilBefore;
    if (increase < SOIL_RESPONSE_MIN_PCT) {
        _lastError = ErrorCode::PUMP_FAIL;
        Serial.printf("[AN TOAN] BOM CO VAN DE! Dat chi tang %d%% sau %lu giay tuoi\n",
                      increase, pumpOnSec);
        return true; // BƠM CÓ VẤN ĐỀ
    }

    return false;
}

// ============================================================
// Kiểm tra timeout chế độ manual
// ============================================================
bool SafetyManager::checkManualTimeout(unsigned long manualStartTime, 
                                        uint8_t timeoutMin) {
    uint32_t timeoutMs = (uint32_t)timeoutMin * 60 * 1000;
    unsigned long elapsed = millis() - manualStartTime;

    if (elapsed >= timeoutMs) {
        Serial.printf("[AN TOAN] Manual timeout! (%lu ms >= %lu ms)\n", 
                      elapsed, timeoutMs);
        return true; // CẦN VỀ AUTO
    }

    return false;
}

// ============================================================
// Kiểm tra đất đã đủ ẩm chưa
// ============================================================
bool SafetyManager::isSoilWetEnough(int soilPercent, uint8_t highThreshold) {
    if (soilPercent >= highThreshold) {
        Serial.printf("[AN TOAN] Dat du am %d%% >= %d%% → dung bom\n", 
                      soilPercent, highThreshold);
        return true;
    }
    return false;
}

// ============================================================
// Getter/Setter lỗi
// ============================================================
ErrorCode SafetyManager::getLastError() const {
    return _lastError;
}

void SafetyManager::clearError() {
    _lastError = ErrorCode::NONE;
    Serial.println(F("[AN TOAN] Da xoa loi"));
}
