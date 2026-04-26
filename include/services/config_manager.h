
#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "drivers/eeprom_driver.h"

class ConfigManager {
public:
    void init(EepromDriver &eeprom);

    // ─── Cấu hình hệ thống ───

    bool loadConfig();

    void saveConfig();

    void resetDefaults();

    const SystemConfig& getConfig() const;

    SystemConfig& getConfigMutable();

    // ─── Lịch tưới ───

    void loadSchedules();

    void saveSchedules();

    ScheduleEntry* getSchedules();

    // ─── Dữ liệu Runtime ───

    void loadRuntime();

    void saveRuntime();

    RuntimeData& getRuntime();

    const RuntimeData& getRuntime() const;

    // ─── Dữ liệu Adaptive ───

    void loadAdaptive();

    void saveAdaptive();

    AdaptiveData& getAdaptive();

    const AdaptiveData& getAdaptive() const;

private:
    EepromDriver *_eeprom = nullptr;    // Con trỏ đến EEPROM driver

    SystemConfig _config;               // Cấu hình hệ thống
    ScheduleEntry _schedules[MAX_SCHEDULES]; // Mảng lịch tưới
    RuntimeData _runtime;               // Dữ liệu runtime
    AdaptiveData _adaptive;             // Dữ liệu thích ứng

    bool _isEepromValid();

    void _writeHeader();
};

#endif // CONFIG_MANAGER_H
