
#ifndef EEPROM_DRIVER_H
#define EEPROM_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class EepromDriver {
public:
    bool init();

    void writeByte(uint16_t addr, uint8_t data);

    uint8_t readByte(uint16_t addr);

    void writeBlock(uint16_t addr, const uint8_t *data, uint16_t len);

    void readBlock(uint16_t addr, uint8_t *data, uint16_t len);

    template <typename T>
    void writeStruct(uint16_t addr, const T &obj) {
        writeBlock(addr, (const uint8_t *)&obj, sizeof(T));
    }

    template <typename T>
    void readStruct(uint16_t addr, T &obj) {
        readBlock(addr, (uint8_t *)&obj, sizeof(T));
    }

    static uint8_t crc8(const uint8_t *data, uint16_t len);

private:
    uint8_t _i2cAddr = EEPROM_I2C_ADDR;    // Địa chỉ I2C của AT24C32

    bool _probeAddress();

    void _writePage(uint16_t addr, const uint8_t *data, uint8_t len);
};

#endif // EEPROM_DRIVER_H
