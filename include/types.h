
#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include "config.h"


enum class SystemState : uint8_t {
    IDLE = 0,
    READ_SENSOR,
    ANALYZE,
    DECIDE,
    WATERING,
    LOGGING,
    ERROR
};

enum class OperationMode : uint8_t {
    AUTO = 0,
    MANUAL = 1,
    SCHEDULE = 2
};

enum class WateringLevel : uint8_t {
    NONE = 0,
    SHORT,
    MEDIUM,
    LONG,
    DEFER
};

enum class WateringSubState : uint8_t {
    PUMP_ON = 0,
    CHECK_SOIL,
    COMPLETE
};

enum class ButtonEvent : uint8_t {
    NONE = 0,
    SHORT_PRESS,
    LONG_PRESS
};

enum class ErrorCode : uint8_t {
    NONE = 0,
    DHT_FAIL,
    SOIL_FAIL,
    RTC_FAIL,
    EEPROM_FAIL,
    PUMP_FAIL,
    PUMP_TIMEOUT
};


struct SensorData {
    float temperature;
    float humidity;

    int soilRaw;
    int soilPercent;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t dayOfWeek;

    bool dhtValid;
    bool soilValid;
    bool rtcValid;
};

struct __attribute__((packed)) SystemConfig {
    uint16_t soilDryRaw;
    uint16_t soilWetRaw;
    uint8_t soilThresholdLow;
    uint8_t soilThresholdHigh;
    uint8_t dangerThreshold;
    uint8_t maxPumpTimeMin;
    uint16_t dailyWaterBudget;
    uint8_t cooldownMinutes;
    uint16_t manualTimeoutSec;
    uint16_t wateringPulseSec;
    uint16_t wateringCheckDelaySec;
    uint8_t mode;
};

struct __attribute__((packed)) ScheduleEntry {
    uint8_t enabled;
    uint8_t hour;
    uint8_t minute;
    uint8_t daysMask;
    uint16_t durationSec;
    uint8_t reserved[2];
};

struct __attribute__((packed)) RuntimeData {
    uint32_t lastWateringTime;
    uint16_t todayWateringSec;
    uint8_t todayDate;
    uint8_t crc;
};

struct __attribute__((packed)) AdaptiveData {
    float avgResponseRate;
    uint16_t totalSessions;
    uint8_t dryTrendCounter;
    uint8_t crc;
};

struct IrrigationDecision {
    bool shouldWater;
    WateringLevel level;
    uint16_t durationSec;
    uint8_t targetPulses;
    String reason;
};

struct SystemStatus {
    SystemState state;
    OperationMode mode;
    SensorData sensors;
    bool pumpRunning;
    uint32_t pumpOnTimeSec;
    uint16_t todayWaterUsed;
    uint16_t dailyBudgetLeft;
    ErrorCode lastError;
    String ipAddress;
};

#endif // TYPES_H
