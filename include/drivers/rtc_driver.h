
#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <Arduino.h>
#include <RTClib.h>
#include "types.h"
#include "config.h"

class RtcDriver {
public:
    bool init();

    bool syncFromNTP();

    bool readTime(SensorData &data);

    uint32_t getUnixTime();

    bool isRunning();

    bool isNtpSynced() const;

private:
    RTC_DS1307 _rtc;                // Đối tượng RTC từ thư viện RTClib
    bool _initialized = false;      // Cờ đã khởi tạo thành công
    bool _ntpSynced = false;        // Đã sync NTP thành công
};

#endif // RTC_DRIVER_H
