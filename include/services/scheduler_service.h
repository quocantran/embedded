
#ifndef SCHEDULER_SERVICE_H
#define SCHEDULER_SERVICE_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

class SchedulerService {
public:
    IrrigationDecision checkSchedules(
        const SensorData &sensors,
        const ScheduleEntry schedules[MAX_SCHEDULES],
        const SystemConfig &config
    );

    void resetDailyFlags();

private:
    // Cờ đánh dấu lịch đã kích hoạt trong ngày hiện tại
    // Tránh kích hoạt lặp trong cùng 1 phút
    bool _triggered[MAX_SCHEDULES] = {false, false, false, false};
    uint8_t _lastCheckMinute = 255;  // Phút kiểm tra cuối (tránh lặp)

    bool _shouldTrigger(const ScheduleEntry &entry, uint8_t hour, 
                        uint8_t minute, uint8_t dayOfWeek, uint8_t index);

    uint16_t _adjustDuration(uint16_t requestedSec, int soilPercent,
                             const SystemConfig &config);
};

#endif // SCHEDULER_SERVICE_H
