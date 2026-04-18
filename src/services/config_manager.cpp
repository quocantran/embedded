/**
 * @file config_manager.cpp
 * @brief Triển khai quản lý cấu hình hệ thống qua EEPROM AT24C32
 * 
 * Cấu trúc EEPROM:
 * [0x0000] Magic byte (0xAB) - xác nhận dữ liệu hợp lệ
 * [0x0001] Version (0x01) - phiên bản cấu trúc
 * [0x0002] CRC8 config - kiểm tra toàn vẹn
 * [0x0010] SystemConfig struct
 * [0x0040] ScheduleEntry[4] 
 * [0x0060] RuntimeData struct
 * [0x0080] AdaptiveData struct
 */

#include "services/config_manager.h"

// ============================================================
// Khởi tạo config manager
// ============================================================
void ConfigManager::init(EepromDriver &eeprom) {
    _eeprom = &eeprom;

    // Kiểm tra EEPROM có dữ liệu hợp lệ không
    if (_isEepromValid()) {
        Serial.println(F("[CONFIG] EEPROM hop le - tai cau hinh..."));
        loadConfig();
        loadSchedules();
        loadRuntime();
        loadAdaptive();
    } else {
        Serial.println(F("[CONFIG] EEPROM trong/loi - dat gia tri mac dinh"));
        resetDefaults();
        // Ghi mặc định vào EEPROM
        _writeHeader();
        saveConfig();
        saveSchedules();
        saveRuntime();
        saveAdaptive();
    }

    // In cấu hình hiện tại
    Serial.printf("[CONFIG] Nguong: %d-%d%%, Nguy hiem: %d%%\n",
                  _config.soilThresholdLow, _config.soilThresholdHigh,
                  _config.dangerThreshold);
    Serial.printf("[CONFIG] Bom max: %d phut, Budget: %d giay/ngay\n",
                  _config.maxPumpTimeMin, _config.dailyWaterBudget);
    Serial.printf("[CONFIG] Cooldown: %d phut, Manual timeout: %d phut\n",
                  _config.cooldownMinutes, _config.manualTimeoutMin);
    Serial.printf("[CONFIG] Che do: %d (0=AUTO, 1=MANUAL, 2=SCHEDULE)\n",
                  _config.mode);
}

// ============================================================
// Tải cấu hình từ EEPROM
// ============================================================
bool ConfigManager::loadConfig() {
    if (!_eeprom) return false;

    _eeprom->readStruct(EEPROM_ADDR_CONFIG, _config);

    // Kiểm tra CRC
    uint8_t storedCrc = _eeprom->readByte(EEPROM_ADDR_CRC_CFG);
    uint8_t calcCrc = EepromDriver::crc8((uint8_t *)&_config, sizeof(SystemConfig));

    if (storedCrc != calcCrc) {
        Serial.println(F("[CONFIG] CANH BAO: CRC config khong khop! Dung mac dinh."));
        resetDefaults();
        return false;
    }

    Serial.println(F("[CONFIG] Tai cau hinh tu EEPROM thanh cong"));
    return true;
}

// ============================================================
// Lưu cấu hình vào EEPROM
// ============================================================
void ConfigManager::saveConfig() {
    if (!_eeprom) return;

    _eeprom->writeStruct(EEPROM_ADDR_CONFIG, _config);

    // Ghi CRC
    uint8_t crc = EepromDriver::crc8((uint8_t *)&_config, sizeof(SystemConfig));
    _eeprom->writeByte(EEPROM_ADDR_CRC_CFG, crc);

    Serial.println(F("[CONFIG] Luu cau hinh vao EEPROM"));
}

// ============================================================
// Reset về mặc định nhà máy
// ============================================================
void ConfigManager::resetDefaults() {
    _config.soilDryRaw = DEFAULT_SOIL_DRY_RAW;
    _config.soilWetRaw = DEFAULT_SOIL_WET_RAW;
    _config.soilThresholdLow = DEFAULT_SOIL_LOW;
    _config.soilThresholdHigh = DEFAULT_SOIL_HIGH;
    _config.dangerThreshold = DEFAULT_DANGER_LEVEL;
    _config.maxPumpTimeMin = MAX_PUMP_TIME_SEC / 60;
    _config.dailyWaterBudget = DEFAULT_WATER_BUDGET_SEC;
    _config.cooldownMinutes = DEFAULT_COOLDOWN_MIN;
    _config.manualTimeoutMin = MANUAL_TIMEOUT_SEC / 60;
    _config.mode = (uint8_t)OperationMode::AUTO;

    // Reset lịch tưới
    for (int i = 0; i < MAX_SCHEDULES; i++) {
        memset(&_schedules[i], 0, sizeof(ScheduleEntry));
    }

    // Reset runtime
    _runtime.lastWateringTime = 0;
    _runtime.todayWateringSec = 0;
    _runtime.todayDate = 0;
    _runtime.crc = 0;

    // Reset adaptive
    _adaptive.avgResponseRate = 0.0f;
    _adaptive.totalSessions = 0;
    _adaptive.dryTrendCounter = 0;
    _adaptive.crc = 0;

    Serial.println(F("[CONFIG] Da reset ve mac dinh"));
}

// ============================================================
// Getter cấu hình
// ============================================================
const SystemConfig& ConfigManager::getConfig() const {
    return _config;
}

SystemConfig& ConfigManager::getConfigMutable() {
    return _config;
}

// ============================================================
// Quản lý lịch tưới
// ============================================================
void ConfigManager::loadSchedules() {
    if (!_eeprom) return;
    for (int i = 0; i < MAX_SCHEDULES; i++) {
        _eeprom->readStruct(EEPROM_ADDR_SCHEDULE + i * sizeof(ScheduleEntry), 
                            _schedules[i]);
    }
    Serial.println(F("[CONFIG] Tai lich tuoi tu EEPROM"));
}

void ConfigManager::saveSchedules() {
    if (!_eeprom) return;
    for (int i = 0; i < MAX_SCHEDULES; i++) {
        _eeprom->writeStruct(EEPROM_ADDR_SCHEDULE + i * sizeof(ScheduleEntry), 
                             _schedules[i]);
    }
    Serial.println(F("[CONFIG] Luu lich tuoi vao EEPROM"));
}

ScheduleEntry* ConfigManager::getSchedules() {
    return _schedules;
}

// ============================================================
// Quản lý dữ liệu runtime
// ============================================================
void ConfigManager::loadRuntime() {
    if (!_eeprom) return;
    _eeprom->readStruct(EEPROM_ADDR_RUNTIME, _runtime);

    // Kiểm tra CRC
    uint8_t calcCrc = EepromDriver::crc8((uint8_t *)&_runtime, 
                                          sizeof(RuntimeData) - 1); // Trừ byte CRC
    if (_runtime.crc != calcCrc) {
        Serial.println(F("[CONFIG] CANH BAO: CRC runtime loi - reset"));
        _runtime.lastWateringTime = 0;
        _runtime.todayWateringSec = 0;
        _runtime.todayDate = 0;
    }
}

void ConfigManager::saveRuntime() {
    if (!_eeprom) return;
    // Tính CRC trước khi ghi
    _runtime.crc = EepromDriver::crc8((uint8_t *)&_runtime, 
                                       sizeof(RuntimeData) - 1);
    _eeprom->writeStruct(EEPROM_ADDR_RUNTIME, _runtime);
}

RuntimeData& ConfigManager::getRuntime() {
    return _runtime;
}

const RuntimeData& ConfigManager::getRuntime() const {
    return _runtime;
}

// ============================================================
// Quản lý dữ liệu adaptive
// ============================================================
void ConfigManager::loadAdaptive() {
    if (!_eeprom) return;
    _eeprom->readStruct(EEPROM_ADDR_ADAPTIVE, _adaptive);

    // Kiểm tra CRC
    uint8_t calcCrc = EepromDriver::crc8((uint8_t *)&_adaptive, 
                                          sizeof(AdaptiveData) - 1);
    if (_adaptive.crc != calcCrc) {
        Serial.println(F("[CONFIG] CANH BAO: CRC adaptive loi - reset"));
        _adaptive.avgResponseRate = 0.0f;
        _adaptive.totalSessions = 0;
        _adaptive.dryTrendCounter = 0;
    }
}

void ConfigManager::saveAdaptive() {
    if (!_eeprom) return;
    _adaptive.crc = EepromDriver::crc8((uint8_t *)&_adaptive, 
                                        sizeof(AdaptiveData) - 1);
    _eeprom->writeStruct(EEPROM_ADDR_ADAPTIVE, _adaptive);
}

AdaptiveData& ConfigManager::getAdaptive() {
    return _adaptive;
}

const AdaptiveData& ConfigManager::getAdaptive() const {
    return _adaptive;
}

// ============================================================
// Private: Kiểm tra EEPROM có dữ liệu hợp lệ không
// ============================================================
bool ConfigManager::_isEepromValid() {
    if (!_eeprom) return false;

    uint8_t magic = _eeprom->readByte(EEPROM_ADDR_MAGIC);
    uint8_t version = _eeprom->readByte(EEPROM_ADDR_VERSION);

    Serial.printf("[CONFIG] EEPROM magic=0x%02X (can 0x%02X), version=%d (can %d)\n",
                  magic, EEPROM_MAGIC_BYTE, version, EEPROM_DATA_VERSION);

    return (magic == EEPROM_MAGIC_BYTE && version == EEPROM_DATA_VERSION);
}

// ============================================================
// Private: Ghi header vào EEPROM
// ============================================================
void ConfigManager::_writeHeader() {
    if (!_eeprom) return;
    _eeprom->writeByte(EEPROM_ADDR_MAGIC, EEPROM_MAGIC_BYTE);
    _eeprom->writeByte(EEPROM_ADDR_VERSION, EEPROM_DATA_VERSION);
    Serial.println(F("[CONFIG] Ghi header EEPROM"));
}
