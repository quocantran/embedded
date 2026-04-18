/**
 * @file eeprom_driver.h
 * @brief Driver cho EEPROM AT24C32 (4KB) trên bus I2C
 * 
 * AT24C32 nằm trên cùng module với DS1307, chia sẻ bus I2C.
 * Địa chỉ mặc định: 0x57 (A0/A1/A2 = 1 trên module thông dụng).
 * 
 * Tính năng:
 * - Đọc/ghi byte đơn lẻ
 * - Đọc/ghi block dữ liệu (tự động chia page 32 bytes)
 * - Đọc/ghi struct với template
 * - Tính CRC8 cho kiểm tra toàn vẹn dữ liệu
 */

#ifndef EEPROM_DRIVER_H
#define EEPROM_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class EepromDriver {
public:
    /**
     * @brief Khởi tạo EEPROM driver
     * @return true nếu EEPROM phản hồi trên I2C
     */
    bool init();

    /**
     * @brief Ghi 1 byte vào địa chỉ EEPROM
     * @param addr Địa chỉ (0x0000 - 0x0FFF)
     * @param data Byte cần ghi
     */
    void writeByte(uint16_t addr, uint8_t data);

    /**
     * @brief Đọc 1 byte từ địa chỉ EEPROM
     * @param addr Địa chỉ (0x0000 - 0x0FFF)
     * @return Byte đọc được
     */
    uint8_t readByte(uint16_t addr);

    /**
     * @brief Ghi block dữ liệu vào EEPROM (tự chia page)
     * @param addr Địa chỉ bắt đầu
     * @param data Con trỏ đến dữ liệu
     * @param len Số byte cần ghi
     */
    void writeBlock(uint16_t addr, const uint8_t *data, uint16_t len);

    /**
     * @brief Đọc block dữ liệu từ EEPROM
     * @param addr Địa chỉ bắt đầu
     * @param data [out] Buffer nhận dữ liệu
     * @param len Số byte cần đọc
     */
    void readBlock(uint16_t addr, uint8_t *data, uint16_t len);

    /**
     * @brief Ghi struct vào EEPROM (template)
     * @tparam T Kiểu struct
     * @param addr Địa chỉ bắt đầu
     * @param obj Tham chiếu đến struct cần ghi
     */
    template <typename T>
    void writeStruct(uint16_t addr, const T &obj) {
        writeBlock(addr, (const uint8_t *)&obj, sizeof(T));
    }

    /**
     * @brief Đọc struct từ EEPROM (template)
     * @tparam T Kiểu struct
     * @param addr Địa chỉ bắt đầu
     * @param obj [out] Tham chiếu đến struct nhận dữ liệu
     */
    template <typename T>
    void readStruct(uint16_t addr, T &obj) {
        readBlock(addr, (uint8_t *)&obj, sizeof(T));
    }

    /**
     * @brief Tính CRC8 cho block dữ liệu
     * @param data Con trỏ đến dữ liệu
     * @param len Số byte
     * @return Giá trị CRC8
     */
    static uint8_t crc8(const uint8_t *data, uint16_t len);

private:
    uint8_t _i2cAddr = EEPROM_I2C_ADDR;    // Địa chỉ I2C của AT24C32

    /**
     * @brief Tìm địa chỉ I2C hợp lệ của EEPROM (0x50..0x57)
     * @return true nếu tìm thấy thiết bị phản hồi
     */
    bool _probeAddress();

    /**
     * @brief Ghi 1 page EEPROM (tối đa 32 bytes, không vượt ranh giới page)
     * @param addr Địa chỉ bắt đầu
     * @param data Con trỏ đến dữ liệu
     * @param len Số byte (tối đa EEPROM_PAGE_SIZE)
     */
    void _writePage(uint16_t addr, const uint8_t *data, uint8_t len);
};

#endif // EEPROM_DRIVER_H
