/**
 * @file safety_manager.h
 * @brief Quản lý an toàn hệ thống - bảo vệ phần cứng và phát hiện lỗi
 * 
 * Các cơ chế bảo vệ:
 * 1. Sensor failure: N lần đọc lỗi liên tiếp → chuyển ERROR
 * 2. Pump timeout: bơm chạy quá MAX_PUMP_TIME_SEC → tắt bơm
 * 3. Pump effectiveness: tưới 60s mà đất không tăng → cảnh báo
 * 4. Manual timeout: manual mode quá MANUAL_TIMEOUT_SEC → về AUTO
 * 5. Soil too wet: đất vượt ngưỡng cao → dừng tưới
 */

#ifndef SAFETY_MANAGER_H
#define SAFETY_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

class SafetyManager {
public:
    /**
     * @brief Kiểm tra an toàn cảm biến
     * @param dhtFailCount Số lần DHT lỗi liên tiếp
     * @param soilFailCount Số lần đất lỗi liên tiếp
     * @return ErrorCode khác NONE nếu có lỗi
     */
    ErrorCode checkSensors(uint8_t dhtFailCount, uint8_t soilFailCount);

    /**
     * @brief Kiểm tra timeout bơm (bảo vệ phần cứng)
     * @param pumpOnSec Thời gian bơm đã chạy liên tục (giây)
     * @param maxPumpTimeMin Giới hạn tối đa (phút, từ config)
     * @return true nếu bơm ĐÃ VƯỢT QUÁ giới hạn → cần TẮT
     */
    bool checkPumpTimeout(uint32_t pumpOnSec, uint8_t maxPumpTimeMin);

    /**
     * @brief Kiểm tra hiệu quả bơm (bơm có thực sự tưới được không?)
     * 
     * So sánh độ ẩm đất trước và sau khi tưới 60s.
     * Nếu đất không tăng → bơm hỏng hoặc hết nước.
     * 
     * @param soilBefore Độ ẩm đất trước khi bắt đầu tưới (%)
     * @param soilAfter Độ ẩm đất sau khi tưới (%)
     * @param pumpOnSec Thời gian đã tưới (giây)
     * @return true nếu bơm CÓ VẤN ĐỀ
     */
    bool checkPumpEffectiveness(int soilBefore, int soilAfter, uint32_t pumpOnSec);

    /**
     * @brief Kiểm tra timeout chế độ manual
     * @param manualStartTime Thời điểm bắt đầu manual (millis)
     * @param timeoutMin Timeout (phút, từ config)
     * @return true nếu ĐÃ VƯỢT QUÁ timeout → cần về AUTO
     */
    bool checkManualTimeout(unsigned long manualStartTime, uint8_t timeoutMin);

    /**
     * @brief Kiểm tra đất đã đủ ẩm chưa (dùng trong manual mode)
     * @param soilPercent Độ ẩm đất hiện tại (%)
     * @param highThreshold Ngưỡng cao (%)
     * @return true nếu đất ĐÃ ĐỦ ẨM → nên tắt bơm
     */
    bool isSoilWetEnough(int soilPercent, uint8_t highThreshold);

    /**
     * @brief Lấy mã lỗi gần nhất
     */
    ErrorCode getLastError() const;

    /**
     * @brief Xóa lỗi (sau khi phục hồi)
     */
    void clearError();

private:
    ErrorCode _lastError = ErrorCode::NONE;  // Lỗi gần nhất
};

#endif // SAFETY_MANAGER_H
