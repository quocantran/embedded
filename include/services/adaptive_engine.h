/**
 * @file adaptive_engine.h
 * @brief Engine tưới thích ứng - học phản hồi đất để tối ưu tưới
 * 
 * Tính năng nâng cao:
 * 1. Theo dõi phản hồi đất (soil response): đất tăng bao nhiêu % sau mỗi giây tưới
 * 2. Điều chỉnh thời gian tưới dựa trên lịch sử phản hồi
 * 3. Phát hiện xu hướng khô (dry trend): đất liên tục giảm ẩm qua các lần đo
 * 4. Anti-overwatering: cooldown bắt buộc sau mỗi phiên tưới
 * 5. Water budget hàng ngày: giới hạn tổng thời gian tưới/ngày
 */

#ifndef ADAPTIVE_ENGINE_H
#define ADAPTIVE_ENGINE_H

#include <Arduino.h>
#include "types.h"

class AdaptiveEngine {
public:
    /**
     * @brief Ghi nhận kết quả 1 phiên tưới để học
     * @param soilBefore Độ ẩm đất trước tưới (%)
     * @param soilAfter Độ ẩm đất sau tưới (%)
     * @param durationSec Thời gian đã tưới (giây)
     * @param adaptive [in/out] Dữ liệu thích ứng (sẽ được cập nhật)
     */
    void recordSession(int soilBefore, int soilAfter, 
                       uint16_t durationSec, AdaptiveData &adaptive);

    /**
     * @brief Điều chỉnh thời gian tưới dựa trên dữ liệu thích ứng
     * @param requestedSec Thời gian tưới đề xuất ban đầu (giây)
     * @param adaptive Dữ liệu thích ứng hiện tại
     * @return Thời gian tưới đã điều chỉnh (giây)
     */
    uint16_t adjustDuration(uint16_t requestedSec, const AdaptiveData &adaptive);

    /**
     * @brief Cập nhật theo dõi xu hướng khô
     * @param soilPercent Độ ẩm đất hiện tại (%)
     * @param adaptive [in/out] Dữ liệu thích ứng
     * @return true nếu phát hiện xu hướng khô đáng lo ngại (> 5 lần liên tiếp)
     */
    bool updateDryTrend(int soilPercent, AdaptiveData &adaptive);

    /**
     * @brief Kiểm tra ngân sách nước hàng ngày
     * @param todayUsed Thời gian đã tưới hôm nay (giây)
     * @param dailyBudget Ngân sách tối đa (giây)
     * @return Thời gian còn được phép tưới (giây), 0 nếu hết ngân sách
     */
    uint16_t checkBudget(uint16_t todayUsed, uint16_t dailyBudget);

    /**
     * @brief Reset ngân sách nước cho ngày mới
     * @param runtime [in/out] Dữ liệu runtime
     * @param currentDay Ngày hiện tại (1-31)
     */
    void resetDailyBudget(RuntimeData &runtime, uint8_t currentDay);

private:
    int _lastSoilPercent = -1;  // Giá trị đất lần đo trước (để theo dõi trend)
};

#endif // ADAPTIVE_ENGINE_H
