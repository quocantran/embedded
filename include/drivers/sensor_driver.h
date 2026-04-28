
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
    DHT _dht = DHT(PIN_DHT, DHT_TYPE);

    // Bộ lọc trung bình cho cảm biến đất
    int _soilSamples[SOIL_SAMPLE_COUNT];
    uint8_t _sampleIndex = 0;              
    bool _samplesReady = false;

    // Đếm lỗi liên tiếp
    uint8_t _dhtFailCount = 0;
    uint8_t _soilFailCount = 0;

    int _readSoilFiltered();

    int _rawToPercent(int raw, uint16_t dryRaw, uint16_t wetRaw);
};

#endif // SENSOR_DRIVER_H
