
#include "drivers/eeprom_driver.h"

bool EepromDriver::init() {
    if (_probeAddress()) {
        Serial.printf("[EEPROM] Tim thay AT24C32 tai dia chi 0x%02X\n", _i2cAddr);
        return true;
    }

    Serial.println(F("[EEPROM] LOI: Khong tim thay EEPROM tren cac dia chi 0x50..0x57"));
    Serial.println(F("[EEPROM] Goi y: Kiem tra jumper A0/A1/A2, day SDA/SCL, nguon module DS1307"));
    return false;
}

bool EepromDriver::_probeAddress() {
    const uint8_t candidates[] = {EEPROM_I2C_ADDR, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57};

    for (uint8_t i = 0; i < sizeof(candidates); i++) {
        uint8_t addr = candidates[i];

        // Bỏ qua địa chỉ trùng lặp để tránh dò lại
        bool duplicate = false;
        for (uint8_t j = 0; j < i; j++) {
            if (candidates[j] == addr) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            _i2cAddr = addr;
            return true;
        }
    }

    return false;
}

void EepromDriver::writeByte(uint16_t addr, uint8_t data) {
    Wire.beginTransmission(_i2cAddr);
    Wire.write((uint8_t)(addr >> 8));       // Byte cao của địa chỉ
    Wire.write((uint8_t)(addr & 0xFF));     // Byte thấp của địa chỉ
    Wire.write(data);                        // Dữ liệu
    Wire.endTransmission();

    // Chờ EEPROM hoàn thành chu kỳ ghi
    delay(EEPROM_WRITE_DELAY_MS);
}

uint8_t EepromDriver::readByte(uint16_t addr) {
    // Bước 1: Gửi địa chỉ cần đọc (dummy write)
    Wire.beginTransmission(_i2cAddr);
    Wire.write((uint8_t)(addr >> 8));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.endTransmission();

    // Bước 2: Yêu cầu đọc 1 byte
    Wire.requestFrom(_i2cAddr, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF; // Giá trị mặc định nếu đọc lỗi
}

void EepromDriver::writeBlock(uint16_t addr, const uint8_t *data, uint16_t len) {
    uint16_t written = 0;

    while (written < len) {
        // Tính số byte có thể ghi trong page hiện tại
        // (không được vượt ranh giới page)
        uint8_t pageOffset = addr % EEPROM_PAGE_SIZE;
        uint8_t pageRemain = EEPROM_PAGE_SIZE - pageOffset;
        uint8_t toWrite = min((uint16_t)pageRemain, (uint16_t)(len - written));

        // Ghi 1 page (hoặc phần page)
        _writePage(addr, data + written, toWrite);

        // Cập nhật con trỏ
        addr += toWrite;
        written += toWrite;
    }
}

void EepromDriver::readBlock(uint16_t addr, uint8_t *data, uint16_t len) {
    uint16_t read = 0;

    while (read < len) {
        // Wire buffer tối đa 32 bytes, đọc từng chunk
        uint8_t chunkSize = min((uint16_t)32, (uint16_t)(len - read));

        // Gửi địa chỉ bắt đầu
        Wire.beginTransmission(_i2cAddr);
        Wire.write((uint8_t)(addr >> 8));
        Wire.write((uint8_t)(addr & 0xFF));
        Wire.endTransmission();

        // Yêu cầu đọc chunk
        Wire.requestFrom(_i2cAddr, chunkSize);
        for (uint8_t i = 0; i < chunkSize && Wire.available(); i++) {
            data[read + i] = Wire.read();
        }

        addr += chunkSize;
        read += chunkSize;
    }
}

uint8_t EepromDriver::crc8(const uint8_t *data, uint16_t len) {
    uint8_t crc = 0x00;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void EepromDriver::_writePage(uint16_t addr, const uint8_t *data, uint8_t len) {
    Wire.beginTransmission(_i2cAddr);
    Wire.write((uint8_t)(addr >> 8));
    Wire.write((uint8_t)(addr & 0xFF));

    for (uint8_t i = 0; i < len; i++) {
        Wire.write(data[i]);
    }

    Wire.endTransmission();

    // Chờ EEPROM hoàn thành chu kỳ ghi trang
    delay(EEPROM_WRITE_DELAY_MS);
}
