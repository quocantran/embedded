/**
 * @file config_manager.h
 * @brief Quản lý cấu hình hệ thống - đọc/ghi EEPROM AT24C32
 * 
 * Chịu trách nhiệm:
 * - Đọc cấu hình từ EEPROM khi khởi động
 * - Ghi cấu hình khi thay đổi (từ web hoặc mặc định)
 * - Xác minh dữ liệu EEPROM bằng CRC8
 * - Lưu/khôi phục dữ liệu runtime (lần tưới cuối, ngân sách)
 * - Lưu/khôi phục dữ liệu thích ứng (soil response)
 * - Reset về mặc định nếu EEPROM trống hoặc dữ liệu hỏng
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "drivers/eeprom_driver.h"

class ConfigManager {
public:
    /**
     * @brief Khởi tạo config manager với tham chiếu đến EEPROM driver
     * @param eeprom Tham chiếu đến EepromDriver đã khởi tạo
     */
    void init(EepromDriver &eeprom);

    // ─── Cấu hình hệ thống ───

    /**
     * @brief Tải cấu hình từ EEPROM
     * @return true nếu tải thành công (CRC hợp lệ)
     */
    bool loadConfig();

    /**
     * @brief Lưu cấu hình hiện tại vào EEPROM
     */
    void saveConfig();

    /**
     * @brief Đặt cấu hình về giá trị mặc định nhà máy
     */
    void resetDefaults();

    /**
     * @brief Lấy cấu hình hiện tại (read-only)
     */
    const SystemConfig& getConfig() const;

    /**
     * @brief Lấy cấu hình hiện tại (writable - để web server cập nhật)
     */
    SystemConfig& getConfigMutable();

    // ─── Lịch tưới ───

    /**
     * @brief Tải lịch tưới từ EEPROM
     */
    void loadSchedules();

    /**
     * @brief Lưu lịch tưới vào EEPROM
     */
    void saveSchedules();

    /**
     * @brief Lấy mảng lịch tưới
     */
    ScheduleEntry* getSchedules();

    // ─── Dữ liệu Runtime ───

    /**
     * @brief Tải dữ liệu runtime từ EEPROM
     */
    void loadRuntime();

    /**
     * @brief Lưu dữ liệu runtime vào EEPROM
     */
    void saveRuntime();

    /**
     * @brief Lấy dữ liệu runtime (đọc/ghi)
     */
    RuntimeData& getRuntime();

    /**
     * @brief Lấy dữ liệu runtime (chỉ đọc)
     */
    const RuntimeData& getRuntime() const;

    // ─── Dữ liệu Adaptive ───

    /**
     * @brief Tải dữ liệu thích ứng từ EEPROM
     */
    void loadAdaptive();

    /**
     * @brief Lưu dữ liệu thích ứng vào EEPROM
     */
    void saveAdaptive();

    /**
     * @brief Lấy dữ liệu thích ứng (đọc/ghi)
     */
    AdaptiveData& getAdaptive();

    /**
     * @brief Lấy dữ liệu thích ứng (chỉ đọc)
     */
    const AdaptiveData& getAdaptive() const;

private:
    EepromDriver *_eeprom = nullptr;    // Con trỏ đến EEPROM driver

    SystemConfig _config;               // Cấu hình hệ thống
    ScheduleEntry _schedules[MAX_SCHEDULES]; // Mảng lịch tưới
    RuntimeData _runtime;               // Dữ liệu runtime
    AdaptiveData _adaptive;             // Dữ liệu thích ứng

    /**
     * @brief Kiểm tra EEPROM có dữ liệu hợp lệ không (magic byte + version)
     * @return true nếu dữ liệu hợp lệ
     */
    bool _isEepromValid();

    /**
     * @brief Ghi magic byte và version vào EEPROM
     */
    void _writeHeader();
};

#endif // CONFIG_MANAGER_H
