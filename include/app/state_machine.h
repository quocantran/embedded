/**
 * @file state_machine.h
 * @brief Máy trạng thái chính điều phối toàn bộ hệ thống
 * 
 * 7 trạng thái: IDLE → READ_SENSOR → ANALYZE → DECIDE → WATERING → LOGGING → IDLE
 *               Bất kỳ → ERROR (khi phát hiện lỗi)
 * 
 * Tính năng:
 * - Non-blocking hoàn toàn (dùng millis)
 * - Progressive watering (tưới từng xung 30s, kiểm tra giữa các xung)
 * - Xử lý nút nhấn: long press → MANUAL, short press → toggle bơm
 * - An toàn: timeout, sensor fail, pump fail → tất cả được giám sát
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "types.h"
#include "drivers/sensor_driver.h"
#include "drivers/rtc_driver.h"
#include "drivers/relay_driver.h"
#include "drivers/button_driver.h"
#include "drivers/lcd_driver.h"
#include "services/irrigation_service.h"
#include "services/scheduler_service.h"
#include "services/config_manager.h"
#include "services/display_manager.h"
#include "services/safety_manager.h"
#include "services/adaptive_engine.h"

class StateMachine {
public:
    /**
     * @brief Khởi tạo state machine và tất cả các subsystem
     * @return true nếu khởi tạo thành công
     */
    bool init();

    /**
     * @brief Chạy 1 chu kỳ state machine (gọi trong loop)
     * 
     * Non-blocking: mỗi lần gọi chỉ xử lý 1 bước nhỏ.
     * Tất cả timing được quản lý bằng millis().
     */
    void update();

    /**
     * @brief Xử lý sự kiện nút nhấn
     * @param event Sự kiện từ ButtonDriver
     */
    void handleButtonEvent(ButtonEvent event);

    /**
     * @brief Lấy trạng thái tổng hợp (cho web API)
     */
    SystemStatus getStatus() const;

    /**
     * @brief Thay đổi chế độ hoạt động (từ web - chỉ set config, SM quyết định)
     * @param mode Chế độ mới
     */
    void requestModeChange(OperationMode mode);

    // ─── Getter cho các subsystem (web server cần truy cập) ───
    ConfigManager& getConfigManager();
    SensorData& getSensorData();
    RtcDriver& getRtcDriver();

    /**
     * @brief Hiển thị màn hình khởi động trên LCD
     */
    void showBootScreen();

private:
    // ─── Trạng thái ───
    SystemState _state = SystemState::IDLE;
    OperationMode _mode = OperationMode::AUTO;
    OperationMode _pendingMode = OperationMode::AUTO;  // Chế độ yêu cầu từ web
    bool _modeChangeRequested = false;

    // ─── Drivers ───
    SensorDriver _sensorDriver;
    RtcDriver _rtcDriver;
    RelayDriver _relayDriver;
    ButtonDriver _buttonDriver;
    LcdDriver _lcdDriver;

    // ─── Services ───
    IrrigationService _irrigationService;
    SchedulerService _schedulerService;
    ConfigManager _configManager;
    DisplayManager _displayManager;
    SafetyManager _safetyManager;
    AdaptiveEngine _adaptiveEngine;

    // ─── EEPROM Driver (dùng bởi ConfigManager) ───
    EepromDriver _eepromDriver;

    // ─── Dữ liệu ───
    SensorData _sensorData;
    IrrigationDecision _currentDecision;
    ErrorCode _currentError = ErrorCode::NONE;
    String _ipAddress = "";

    // ─── Timing ───
    unsigned long _lastSensorRead = 0;      // Thời điểm đọc cảm biến cuối
    unsigned long _manualStartTime = 0;     // Thời điểm bắt đầu MANUAL
    unsigned long _manualDangerStartTime = 0; // Bắt đầu đếm đất nguy hiểm trong MANUAL
    unsigned long _errorStartTime = 0;      // Thời điểm vào ERROR
    unsigned long _lastEepromSave = 0;      // Thời điểm lưu EEPROM cuối

    // ─── Progressive Watering ───
    WateringSubState _waterSubState = WateringSubState::COMPLETE;
    unsigned long _pulseStartTime = 0;      // Thời điểm bắt đầu xung hiện tại
    unsigned long _checkStartTime = 0;      // Thời điểm bắt đầu kiểm tra đất
    uint8_t _targetPulses = 0;              // Số xung cần thực hiện
    uint8_t _currentPulse = 0;             // Xung hiện tại (0-based)
    uint16_t _totalWateringTime = 0;        // Tổng thời gian đã tưới phiên này
    int _soilBeforeWatering = 0;           // Độ ẩm đất trước khi tưới
    uint8_t _lastDay = 0;                  // Ngày cuối (để reset daily budget)

    // ─── Xử lý từng trạng thái ───
    void _handleIdle();
    void _handleReadSensor();
    void _handleAnalyze();
    void _handleDecide();
    void _handleWatering();
    void _handleLogging();
    void _handleError();

    // ─── Bắt đầu/kết thúc progressive watering ───
    void _startWatering(const IrrigationDecision &decision);
    void _stopWatering(const String &reason);

    // ─── An toàn trong manual mode ───
    void _checkManualSafety();

    // ─── Chuyển trạng thái với logging ───
    void _setState(SystemState newState);
};

#endif // STATE_MACHINE_H
