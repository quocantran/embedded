/**
 * @file irrigation_service.h
 * @brief Dịch vụ quyết định tưới đa yếu tố (Decision Engine)
 * 
 * KHÔNG dùng logic ngưỡng đơn giản. Thay vào đó, đánh giá tổng hợp:
 * - Độ ẩm đất (yếu tố chính)
 * - Nhiệt độ không khí
 * - Độ ẩm không khí
 * - Giờ trong ngày (tránh tưới giữa trưa)
 * 
 * Tính năng:
 * - Hysteresis: chỉ bật nếu đất < ngưỡng thấp, tắt nếu đất > ngưỡng cao
 * - Cooldown: chờ tối thiểu giữa 2 lần tưới
 * - Water budget: giới hạn tổng thời gian tưới/ngày
 * - Progressive: tưới từng xung 30s, không liên tục
 */

#ifndef IRRIGATION_SERVICE_H
#define IRRIGATION_SERVICE_H

#include <Arduino.h>
#include "types.h"

class IrrigationService {
public:
    /**
     * @brief Đánh giá tình huống và ra quyết định tưới
     * @param sensors Dữ liệu cảm biến hiện tại
     * @param config Cấu hình hệ thống
     * @param lastWateringTime Unix timestamp lần tưới cuối
     * @param todayWaterUsed Thời gian đã tưới hôm nay (giây)
     * @return Quyết định tưới (có/không, mức độ, thời gian, lý do)
     */
    IrrigationDecision evaluate(
        const SensorData &sensors,
        const SystemConfig &config,
        uint32_t lastWateringTime,
        uint16_t todayWaterUsed,
        uint32_t currentUnixTime
    );

    /**
     * @brief Kiểm tra xem có đang trong vùng hysteresis không
     * 
     * Hysteresis ngăn bơm bật/tắt liên tục quanh ngưỡng:
     * - Đất < ngưỡng thấp → BẮT ĐẦU tưới
     * - Đất > ngưỡng cao → DỪNG tưới  
     * - Giữa 2 ngưỡng → GIỮ NGUYÊN trạng thái hiện tại
     * 
     * @param soilPercent Độ ẩm đất hiện tại (%)
     * @param lowThresh Ngưỡng thấp (%)
     * @param highThresh Ngưỡng cao (%)
     * @param currentlyWatering Đang tưới hay không
     * @return true nếu NÊN tưới (dựa trên hysteresis)
     */
    bool checkHysteresis(int soilPercent, uint8_t lowThresh, 
                         uint8_t highThresh, bool currentlyWatering);

private:
    bool _wasWatering = false;  // Trạng thái trước đó (cho hysteresis)

    /**
     * @brief Kiểm tra giờ có nằm trong khung tránh tưới (giữa trưa)
     * @param hour Giờ hiện tại (0-23)
     * @return true nếu KHÔNG NÊN tưới vào giờ này
     */
    bool _isNoonPeriod(uint8_t hour);

    /**
     * @brief Kiểm tra cooldown: đã đủ thời gian chờ sau lần tưới cuối?
     * @param lastWateringTime Unix timestamp lần tưới cuối
     * @param cooldownMin Thời gian chờ tối thiểu (phút)
     * @param currentTime Unix timestamp hiện tại (từ RTC)
     * @return true nếu ĐÃ QUA cooldown, có thể tưới
     */
    bool _cooldownPassed(uint32_t lastWateringTime, uint8_t cooldownMin,
                         uint32_t currentTime);

    /**
     * @brief Tính mức độ tưới dựa trên đa yếu tố
     * @param soilPercent Độ ẩm đất (%)
     * @param temperature Nhiệt độ (°C)
     * @param humidity Độ ẩm KK (%)
     * @param hour Giờ hiện tại
     * @return Mức độ tưới và số xung tương ứng
     */
    WateringLevel _calculateLevel(int soilPercent, float temperature, 
                                  float humidity, uint8_t hour);

    /**
     * @brief Tính số xung progressive từ mức độ tưới
     * @param level Mức độ tưới
     * @return Số xung (mỗi xung = WATERING_PULSE_SEC giây)
     */
    uint8_t _levelToPulses(WateringLevel level);
};

#endif // IRRIGATION_SERVICE_H
