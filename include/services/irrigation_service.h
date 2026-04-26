
#ifndef IRRIGATION_SERVICE_H
#define IRRIGATION_SERVICE_H

#include <Arduino.h>
#include "types.h"

class IrrigationService {
public:
    IrrigationDecision evaluate(
        const SensorData &sensors,
        const SystemConfig &config,
        uint32_t lastWateringTime,
        uint16_t todayWaterUsed,
        uint32_t currentUnixTime
    );

    bool checkHysteresis(int soilPercent, uint8_t lowThresh, 
                         uint8_t highThresh, bool currentlyWatering);

private:
    bool _wasWatering = false;  // Trạng thái trước đó (cho hysteresis)

    bool _isNoonPeriod(uint8_t hour);

    bool _cooldownPassed(uint32_t lastWateringTime, uint8_t cooldownMin,
                         uint32_t currentTime);

    WateringLevel _calculateLevel(int soilPercent, float temperature, 
                                  float humidity, uint8_t hour);

    uint8_t _levelToPulses(WateringLevel level);
};

#endif // IRRIGATION_SERVICE_H
