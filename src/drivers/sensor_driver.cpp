
#include "drivers/sensor_driver.h"

void SensorDriver::init() {
    _dht.begin();

    // Khởi tạo chân ADC cho cảm biến đất
    pinMode(PIN_SOIL, INPUT);

    // Khởi tạo mảng bộ lọc với giá trị mặc định (đất khô)
    for (int i = 0; i < SOIL_SAMPLE_COUNT; i++) {
        _soilSamples[i] = DEFAULT_SOIL_DRY_RAW;
    }
    _sampleIndex = 0;
    _samplesReady = false;

    _dhtFailCount = 0;
    _soilFailCount = 0;

    Serial.println(F("[SENSOR] Khoi tao cam bien thanh cong"));
}

void SensorDriver::readAll(SensorData &data, uint16_t dryRaw, uint16_t wetRaw) {
    float t = _dht.readTemperature();
    float h = _dht.readHumidity();

    if (isnan(t) || isnan(h)) {
        _dhtFailCount++;
        data.dhtValid = false;
        Serial.printf("[SENSOR] DHT loi lan thu %d\n", _dhtFailCount);
    } else {
        _dhtFailCount = 0;
        data.temperature = t;
        data.humidity = h;
        data.dhtValid = true;
    }

    // --- Đọc cảm biến đất ---
    readSoilOnly(data, dryRaw, wetRaw);
}

void SensorDriver::readSoilOnly(SensorData &data, uint16_t dryRaw, uint16_t wetRaw) {
    int rawFiltered = _readSoilFiltered();

    // Kiểm tra giá trị có hợp lệ không (nằm trong khoảng ADC 0-4095)
    if (rawFiltered < 0 || rawFiltered > 4095) {
        _soilFailCount++;
        data.soilValid = false;
        Serial.printf("[SENSOR] Cam bien dat loi lan thu %d (raw=%d)\n", 
                      _soilFailCount, rawFiltered);
    } else {
        _soilFailCount = 0;
        data.soilRaw = rawFiltered;
        data.soilPercent = _rawToPercent(rawFiltered, dryRaw, wetRaw);
        data.soilValid = true;
    }
}

uint8_t SensorDriver::getDhtFailCount() const {
    return _dhtFailCount;
}

uint8_t SensorDriver::getSoilFailCount() const {
    return _soilFailCount;
}

int SensorDriver::_readSoilFiltered() {
    // Đọc giá trị ADC thô
    int raw = analogRead(PIN_SOIL);

    // Thêm vào mảng mẫu (circular buffer)
    _soilSamples[_sampleIndex] = raw;
    _sampleIndex = (_sampleIndex + 1) % SOIL_SAMPLE_COUNT;

    // Đánh dấu đã đủ mẫu sau vòng đầu tiên
    if (_sampleIndex == 0) {
        _samplesReady = true;
    }

    // Tính trung bình
    int count = _samplesReady ? SOIL_SAMPLE_COUNT : (_sampleIndex);
    if (count == 0) count = 1; // Tránh chia cho 0

    long sum = 0;
    for (int i = 0; i < count; i++) {
        sum += _soilSamples[i];
    }

    return (int)(sum / count);
}

int SensorDriver::_rawToPercent(int raw, uint16_t dryRaw, uint16_t wetRaw) {
    // Map giá trị ADC: đất khô (4095 ADC) = 0%, đất ướt (1500 ADC) = 100%
    int percent = map(raw, (int)dryRaw, (int)wetRaw, 0, 100);

    return constrain(percent, 0, 100);
}
