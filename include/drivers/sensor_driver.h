/**
 * @file sensor_driver.h
 * @brief Driver đọc cảm biến DHT11 (nhiệt độ/ẩm) và cảm biến đất (analog)
 * 
 * Tính năng:
 * - Đọc DHT11 non-blocking (tôn trọng khoảng cách đọc tối thiểu)
 * - Đọc cảm biến đất với bộ lọc trung bình (moving average)
 * - Hiệu chuẩn giá trị đất thô → phần trăm dựa trên điểm khô/ướt
 * - Phát hiện lỗi cảm biến (giá trị NaN, ngoài phạm vi)
 */

#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"
#include "types.h"

class SensorDriver {
public:
    /**
     * @brief Khởi tạo các cảm biến
     * @note Gọi 1 lần trong setup()
     */
    void init();

    /**
     * @brief Đọc tất cả cảm biến và cập nhật vào SensorData
     * @param data [out] Struct chứa dữ liệu cảm biến
     * @param dryRaw Giá trị ADC khi đất khô (từ hiệu chuẩn)
     * @param wetRaw Giá trị ADC khi đất ướt (từ hiệu chuẩn)
     */
    void readAll(SensorData &data, uint16_t dryRaw, uint16_t wetRaw);

    /**
     * @brief Chỉ đọc cảm biến đất (dùng trong WATERING state để kiểm tra nhanh)
     * @param data [out] Cập nhật soilRaw và soilPercent
     * @param dryRaw Giá trị ADC khi đất khô
     * @param wetRaw Giá trị ADC khi đất ướt
     */
    void readSoilOnly(SensorData &data, uint16_t dryRaw, uint16_t wetRaw);

    /**
     * @brief Lấy số lần đọc DHT lỗi liên tiếp
     * @return Số lần lỗi liên tiếp (reset về 0 khi đọc thành công)
     */
    uint8_t getDhtFailCount() const;

    /**
     * @brief Lấy số lần đọc đất lỗi liên tiếp
     * @return Số lần lỗi liên tiếp
     */
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

    /**
     * @brief Đọc cảm biến đất với bộ lọc trung bình
     * @return Giá trị ADC trung bình
     */
    int _readSoilFiltered();

    /**
     * @brief Chuyển giá trị ADC thô → phần trăm dựa trên điểm hiệu chuẩn
     * @param raw Giá trị ADC thô
     * @param dryRaw Giá trị ADC khi đất khô
     * @param wetRaw Giá trị ADC khi đất ướt
     * @return Phần trăm độ ẩm đất (0-100%)
     */
    int _rawToPercent(int raw, uint16_t dryRaw, uint16_t wetRaw);
};

#endif // SENSOR_DRIVER_H
