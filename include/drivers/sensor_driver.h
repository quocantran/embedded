
#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"
#include "types.h"

class SensorDriver {
public:
    void init();

    void readAll(SensorData &data, uint16_t dryRaw, uint16_t wetRaw);

    void readSoilOnly(SensorData &data, uint16_t dryRaw, uint16_t wetRaw);

    uint8_t getDhtFailCount() const;

    uint8_t getSoilFailCount() const;

private:
    DHT _dht = DHT(PIN_DHT, DHT_TYPE);     // Đối tượng DHT

    // Bộ lọc trung bình cho cảm biến đất
    int _soilSamples[SOIL_SAMPLE_COUNT];    // Mảng mẫu
    uint8_t _sampleIndex = 0;              // Chỉ số mẫu hiện tại
    bool _samplesReady = false;            // Đã đủ mẫu để tính trung bình?

    // Đếm lỗi liên tiếp
    uint8_t _dhtFailCount = 0;             // Số lần DHT lỗi liên tiếp
    uint8_t _soilFailCount = 0;            // Số lần đất lỗi liên tiếp

    int _readSoilFiltered();

    int _rawToPercent(int raw, uint16_t dryRaw, uint16_t wetRaw);
};

#endif // SENSOR_DRIVER_H
