
#ifndef ADAPTIVE_ENGINE_H
#define ADAPTIVE_ENGINE_H

#include <Arduino.h>
#include "types.h"

class AdaptiveEngine {
public:
    void recordSession(int soilBefore, int soilAfter, 
                       uint16_t durationSec, AdaptiveData &adaptive);

    uint16_t adjustDuration(uint16_t requestedSec, const AdaptiveData &adaptive);

    bool updateDryTrend(int soilPercent, AdaptiveData &adaptive);

    uint16_t checkBudget(uint16_t todayUsed, uint16_t dailyBudget);

    void resetDailyBudget(RuntimeData &runtime, uint8_t currentDay);

private:
    int _lastSoilPercent = -1;  // Giá trị đất lần đo trước (để theo dõi trend)
};

#endif // ADAPTIVE_ENGINE_H
