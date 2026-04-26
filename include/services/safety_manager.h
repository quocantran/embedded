
#ifndef SAFETY_MANAGER_H
#define SAFETY_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

class SafetyManager {
public:
    ErrorCode checkSensors(uint8_t dhtFailCount, uint8_t soilFailCount);

    bool checkPumpTimeout(uint32_t pumpOnSec, uint8_t maxPumpTimeMin);

    bool checkPumpEffectiveness(int soilBefore, int soilAfter, uint32_t pumpOnSec);

    bool checkManualTimeout(unsigned long manualStartTime, uint8_t timeoutMin);

    bool isSoilWetEnough(int soilPercent, uint8_t highThreshold);

    ErrorCode getLastError() const;

    void clearError();

private:
    ErrorCode _lastError = ErrorCode::NONE;  // Lỗi gần nhất
};

#endif // SAFETY_MANAGER_H
