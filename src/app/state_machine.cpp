/**
 * @file state_machine.cpp
 * @brief Triển khai máy trạng thái chính - điều phối toàn bộ hệ thống
 * 
 * Luồng chính (AUTO mode):
 *   IDLE ──(2s)──→ READ_SENSOR ──→ ANALYZE ──→ DECIDE ──→ WATERING ──→ LOGGING ──→ IDLE
 * 
 * MANUAL mode:
 *   - State machine vẫn chạy IDLE → READ_SENSOR → ANALYZE → LOGGING
 *   - Bơm được điều khiển bằng nút nhấn (short press)
 *   - Safety checks vẫn hoạt động (timeout, sensor fail, soil wet)
 * 
 * Progressive watering:
 *   WATERING ──→ [BƠM 30s] ──→ [TẮT, ĐỌC ĐẤT] ──→ [ĐẤT ĐỦ?] ──→ LOGGING
 *                    ↑                                    │ CHƯA
 *                    └────────────────────────────────────┘
 */

#include "app/state_machine.h"

// ============================================================
// Tên trạng thái (cho debug log)
// ============================================================
static const char* STATE_NAMES[] = {
    "IDLE", "READ_SENSOR", "ANALYZE", "DECIDE", "WATERING", "LOGGING", "ERROR"
};

// ============================================================
// Khởi tạo toàn bộ hệ thống
// ============================================================
bool StateMachine::init() {
    Serial.println(F("\n========================================"));
    Serial.println(F("  SMART IRRIGATION SYSTEM v1.0"));
    Serial.println(F("  He thong tuoi tieu thong minh"));
    Serial.println(F("========================================\n"));

    // --- Khởi tạo I2C ---
    Wire.begin(PIN_SDA, PIN_SCL);
    Serial.println(F("[INIT] I2C da khoi tao"));

    // --- Khởi tạo Drivers ---
    _sensorDriver.init();
    _relayDriver.init();
    _buttonDriver.init();
    _lcdDriver.init();

    // LED onboard de test nhanh nut nhan
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    // RTC
    if (!_rtcDriver.init()) {
        Serial.println(F("[INIT] CANH BAO: RTC loi - tiep tuc khong co RTC"));
    }

    // EEPROM
    if (!_eepromDriver.init()) {
        Serial.println(F("[INIT] CANH BAO: EEPROM loi - dung mac dinh"));
    }

    // --- Khởi tạo Services ---
    _configManager.init(_eepromDriver);
    _displayManager.init(_lcdDriver);

    // Đọc chế độ từ config đã lưu
    _mode = (OperationMode)_configManager.getConfig().mode;
    _pendingMode = _mode;

    // --- Reset sensor data ---
    memset(&_sensorData, 0, sizeof(SensorData));

    // --- Khởi tạo timing ---
    _lastSensorRead = millis();
    _lastEepromSave = millis();
    _lastDay = 0;

    // --- Trạng thái ban đầu ---
    _state = SystemState::IDLE;
    _currentError = ErrorCode::NONE;

    Serial.println(F("\n[INIT] He thong san sang!\n"));
    Serial.println(F("[INIT] LED onboard san sang de test nut nhan"));

    // Hiển thị trên LCD sau 2 giây
    delay(1500);
    _lcdDriver.clear();

    return true;
}

// ============================================================
// Chạy 1 chu kỳ state machine (gọi mỗi loop)
// ============================================================
void StateMachine::update() {
    // Test truc quan nhu code mau: nhan nut (LOW) thi LED onboard sang
    bool isButtonPressed = (digitalRead(PIN_BUTTON) == LOW);
    digitalWrite(PIN_STATUS_LED, isButtonPressed ? HIGH : LOW);

    // ─── Luôn cập nhật nút nhấn ───
    _buttonDriver.update();
    ButtonEvent event = _buttonDriver.getEvent();
    if (event != ButtonEvent::NONE) {
        Serial.printf("[SM] Nhan duoc ButtonEvent=%s, mode hien tai=%d\n",
                      event == ButtonEvent::LONG_PRESS ? "LONG_PRESS" : "SHORT_PRESS",
                      (int)_mode);
        handleButtonEvent(event);
    }

    // ─── Xử lý yêu cầu đổi chế độ từ web ───
    if (_modeChangeRequested) {
        _modeChangeRequested = false;
        if (_pendingMode != _mode) {
            Serial.printf("[SM] Web yeu cau doi che do: %d → %d\n", 
                         (int)_mode, (int)_pendingMode);
            
            // Tắt bơm nếu đang chạy khi đổi chế độ
            if (_relayDriver.isOn()) {
                _relayDriver.off();
            }

            _mode = _pendingMode;
            _configManager.getConfigMutable().mode = (uint8_t)_mode;
            _configManager.saveConfig();

            if (_mode == OperationMode::MANUAL) {
                _manualStartTime = millis();
                _manualDangerStartTime = 0;
            } else {
                _manualDangerStartTime = 0;
            }
        }
    }

    // ─── Kiểm tra an toàn MANUAL mode ───
    if (_mode == OperationMode::MANUAL) {
        _checkManualSafety();
    }

    // ─── Cập nhật LCD ───
    _displayManager.update(_sensorData, _mode, _state, _relayDriver.isOn(),
                           _currentError, _ipAddress,
                           _configManager.getConfig().dangerThreshold);

    // ─── Chạy state machine ───
    switch (_state) {
        case SystemState::IDLE:         _handleIdle();          break;
        case SystemState::READ_SENSOR:  _handleReadSensor();    break;
        case SystemState::ANALYZE:      _handleAnalyze();       break;
        case SystemState::DECIDE:       _handleDecide();        break;
        case SystemState::WATERING:     _handleWatering();      break;
        case SystemState::LOGGING:      _handleLogging();       break;
        case SystemState::ERROR:        _handleError();         break;
    }
}

// ============================================================
// Xử lý sự kiện nút nhấn
// ============================================================
void StateMachine::handleButtonEvent(ButtonEvent event) {
    if (event == ButtonEvent::LONG_PRESS) {
        // ─── LONG PRESS: Chuyển MANUAL ↔ AUTO ───
        if (_mode == OperationMode::MANUAL) {
            // Đang MANUAL → về AUTO
            Serial.println(F("[SM] Nut giu: MANUAL → AUTO"));
            _relayDriver.off(); // Tắt bơm an toàn
            _mode = OperationMode::AUTO;
            _manualDangerStartTime = 0;
            _displayManager.showTemporaryMessage("MODE CHANGED", "MANUAL -> AUTO");
        } else {
            // Đang AUTO/SCHEDULE → vào MANUAL
            Serial.println(F("[SM] Nut giu: → MANUAL"));
            _mode = OperationMode::MANUAL;
            _manualStartTime = millis();
            _manualDangerStartTime = 0;
            _displayManager.showTemporaryMessage("MODE CHANGED", "TO MANUAL");
        }
        // Lưu chế độ vào config
        _configManager.getConfigMutable().mode = (uint8_t)_mode;
        _configManager.saveConfig();
    }
    else if (event == ButtonEvent::SHORT_PRESS) {
        if (_mode == OperationMode::MANUAL) {
            // ─── SHORT PRESS trong MANUAL: Toggle bơm ───
            Serial.println(F("[SM] Nut ngan: Toggle bom (MANUAL)"));
            _relayDriver.toggle();
            _displayManager.showTemporaryMessage("BUTTON SHORT", _relayDriver.isOn() ? "PUMP: ON" : "PUMP: OFF");
        } else {
            // ─── SHORT PRESS ngoài MANUAL: Không làm gì (LCD 1 trang) ───
            Serial.println(F("[SM] Nut ngan: Khong o MANUAL - bo qua"));
            _displayManager.showTemporaryMessage("BUTTON SHORT", "HOLD 3S MANUAL");
        }
    }
}

// ============================================================
// Lấy trạng thái tổng hợp
// ============================================================
SystemStatus StateMachine::getStatus() const {
    SystemStatus status;
    status.state = _state;
    status.mode = _mode;
    status.sensors = _sensorData;
    status.pumpRunning = _relayDriver.isOn();
    status.pumpOnTimeSec = _relayDriver.getOnDurationSec();
    status.todayWaterUsed = _configManager.getRuntime().todayWateringSec;
    
    uint16_t budget = _configManager.getConfig().dailyWaterBudget;
    uint16_t used = status.todayWaterUsed;
    status.dailyBudgetLeft = (used < budget) ? (budget - used) : 0;
    
    status.lastError = _currentError;
    status.ipAddress = _ipAddress;
    return status;
}

// ============================================================
// Yêu cầu đổi chế độ từ web
// ============================================================
void StateMachine::requestModeChange(OperationMode mode) {
    _pendingMode = mode;
    _modeChangeRequested = true;
}

// ============================================================
// Getter cho subsystems
// ============================================================
ConfigManager& StateMachine::getConfigManager() {
    return _configManager;
}

SensorData& StateMachine::getSensorData() {
    return _sensorData;
}

RtcDriver& StateMachine::getRtcDriver() {
    return _rtcDriver;
}

void StateMachine::showBootScreen() {
    _lcdDriver.clear();
    _lcdDriver.printLine(0, "SMART IRRIGATION");
    _lcdDriver.printLine(1, " DANG KHOI DONG");
}

// ============================================================
// IDLE: Chờ đến chu kỳ đọc cảm biến tiếp theo
// ============================================================
void StateMachine::_handleIdle() {
    unsigned long now = millis();

    // Kiểm tra đã đến lúc đọc cảm biến chưa
    if (now - _lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
        _lastSensorRead = now;
        _setState(SystemState::READ_SENSOR);
    }

    // Reset ngân sách nước cho ngày mới
    if (_sensorData.rtcValid && _sensorData.day != _lastDay) {
        _lastDay = _sensorData.day;
        _adaptiveEngine.resetDailyBudget(_configManager.getRuntime(), _sensorData.day);
        _schedulerService.resetDailyFlags();
        _configManager.saveRuntime();
    }
}

// ============================================================
// READ_SENSOR: Đọc tất cả cảm biến
// ============================================================
void StateMachine::_handleReadSensor() {
    const SystemConfig &cfg = _configManager.getConfig();

    // Đọc DHT11 + cảm biến đất
    _sensorDriver.readAll(_sensorData, cfg.soilDryRaw, cfg.soilWetRaw);

    // Đọc RTC
    _rtcDriver.readTime(_sensorData);

    _setState(SystemState::ANALYZE);
}

// ============================================================
// ANALYZE: Phân tích dữ liệu, kiểm tra an toàn
// ============================================================
void StateMachine::_handleAnalyze() {
    // Kiểm tra an toàn cảm biến
    ErrorCode sensorError = _safetyManager.checkSensors(
        _sensorDriver.getDhtFailCount(),
        _sensorDriver.getSoilFailCount()
    );

    if (sensorError != ErrorCode::NONE) {
        _currentError = sensorError;

        // Nếu đang manual → tắt bơm, chuyển ERROR
        if (_mode == OperationMode::MANUAL && _relayDriver.isOn()) {
            _relayDriver.off();
            Serial.println(F("[SM] Sensor loi trong MANUAL → tat bom!"));
        }

        _setState(SystemState::ERROR);
        return;
    }

    // Theo dõi xu hướng khô
    if (_sensorData.soilValid) {
        _adaptiveEngine.updateDryTrend(_sensorData.soilPercent, 
                                        _configManager.getAdaptive());
    }

    _setState(SystemState::DECIDE);
}

// ============================================================
// DECIDE: Ra quyết định tưới
// ============================================================
void StateMachine::_handleDecide() {
    const SystemConfig &cfg = _configManager.getConfig();
    RuntimeData &runtime = _configManager.getRuntime();

    if (_mode == OperationMode::AUTO) {
        // ─── AUTO: Hỏi IrrigationService ───
        _currentDecision = _irrigationService.evaluate(
            _sensorData,
            cfg,
            runtime.lastWateringTime,
            runtime.todayWateringSec,
            _rtcDriver.getUnixTime()
        );

        if (_currentDecision.shouldWater) {
            // Điều chỉnh bởi adaptive engine
            uint16_t adjusted = _adaptiveEngine.adjustDuration(
                _currentDecision.durationSec, _configManager.getAdaptive());
            _currentDecision.durationSec = adjusted;
            _currentDecision.targetPulses = adjusted / WATERING_PULSE_SEC;
            if (_currentDecision.targetPulses == 0) _currentDecision.targetPulses = 1;

            _startWatering(_currentDecision);
            _setState(SystemState::WATERING);
        } else {
            Serial.printf("[SM] Quyet dinh: KHONG tuoi - %s\n", 
                         _currentDecision.reason.c_str());
            _setState(SystemState::LOGGING);
        }

    } else if (_mode == OperationMode::SCHEDULE) {
        // ─── SCHEDULE: Kiểm tra lịch tưới ───
        _currentDecision = _schedulerService.checkSchedules(
            _sensorData, _configManager.getSchedules(), cfg);

        if (_currentDecision.shouldWater) {
            _startWatering(_currentDecision);
            _setState(SystemState::WATERING);
        } else {
            _setState(SystemState::LOGGING);
        }

    } else {
        // ─── MANUAL: Không tự quyết định, chỉ logging ───
        _setState(SystemState::LOGGING);
    }
}

// ============================================================
// WATERING: Progressive watering (tưới từng xung)
// ============================================================
void StateMachine::_handleWatering() {
    const SystemConfig &cfg = _configManager.getConfig();
    unsigned long now = millis();

    switch (_waterSubState) {
        case WateringSubState::PUMP_ON: {
            // ─── Bơm đang chạy - chờ hết xung ───
            uint32_t elapsed = (now - _pulseStartTime) / 1000;

            // Kiểm tra timeout bơm (an toàn)
            if (_safetyManager.checkPumpTimeout(
                    _relayDriver.getOnDurationSec(), cfg.maxPumpTimeMin)) {
                _stopWatering("TIMEOUT BOM - cuong che tat!");
                _currentError = ErrorCode::PUMP_TIMEOUT;
                _setState(SystemState::ERROR);
                return;
            }

            // Kiểm tra xung đã hết thời gian chưa (30s)
            if (elapsed >= WATERING_PULSE_SEC) {
                _relayDriver.off();
                _totalWateringTime += WATERING_PULSE_SEC;
                _currentPulse++;

                Serial.printf("[SM] Xung %d/%d hoan thanh (%ds tong)\n",
                             _currentPulse, _targetPulses, _totalWateringTime);

                // Chuyển sang kiểm tra đất
                _waterSubState = WateringSubState::CHECK_SOIL;
                _checkStartTime = now;
            }
            break;
        }

        case WateringSubState::CHECK_SOIL: {
            // ─── Đợi + đọc lại cảm biến đất ───
            uint32_t waitElapsed = (now - _checkStartTime) / 1000;

            if (waitElapsed >= WATERING_CHECK_DELAY_SEC) {
                // Đọc lại cảm biến đất
                _sensorDriver.readSoilOnly(_sensorData, cfg.soilDryRaw, cfg.soilWetRaw);

                Serial.printf("[SM] Kiem tra dat sau xung: %d%%\n", _sensorData.soilPercent);

                // Điều kiện dừng 1: Đất đã đủ ẩm (hysteresis - ngưỡng CAO)
                if (_sensorData.soilValid && 
                    _sensorData.soilPercent >= cfg.soilThresholdHigh) {
                    _stopWatering("Dat du am " + String(_sensorData.soilPercent) + "%");
                    _setState(SystemState::LOGGING);
                    return;
                }

                // Điều kiện dừng 2: Đã đủ số xung
                if (_currentPulse >= _targetPulses) {
                    _stopWatering("Da du " + String(_targetPulses) + " xung");
                    _setState(SystemState::LOGGING);
                    return;
                }

                // Kiểm tra hiệu quả bơm (sau PUMP_FAIL_CHECK_SEC)
                if (_safetyManager.checkPumpEffectiveness(
                        _soilBeforeWatering, _sensorData.soilPercent, 
                        _totalWateringTime)) {
                    _stopWatering("Bom khong hieu qua - kiem tra may bom!");
                    _currentError = ErrorCode::PUMP_FAIL;
                    _setState(SystemState::ERROR);
                    return;
                }

                // Tiếp tục xung tiếp theo
                _relayDriver.on();
                _pulseStartTime = now;
                _waterSubState = WateringSubState::PUMP_ON;
                Serial.printf("[SM] Bat dau xung %d/%d\n", 
                             _currentPulse + 1, _targetPulses);
            }
            break;
        }

        case WateringSubState::COMPLETE:
            // Đã hoàn thành - chuyển sang LOGGING
            _setState(SystemState::LOGGING);
            break;
    }
}

// ============================================================
// LOGGING: Ghi log dữ liệu
// ============================================================
void StateMachine::_handleLogging() {
    // Debug log ra Serial
    Serial.printf("[LOG] [%02d:%02d:%02d] Dat:%d%% T:%.1f°C H:%.0f%% Bom:%s Mode:%s\n",
                  _sensorData.hour, _sensorData.minute, _sensorData.second,
                  _sensorData.soilPercent, _sensorData.temperature, _sensorData.humidity,
                  _relayDriver.isOn() ? "BAT" : "TAT",
                  _mode == OperationMode::AUTO ? "AUTO" : 
                  _mode == OperationMode::MANUAL ? "MANUAL" : "SCHEDULE");

    // Lưu runtime vào EEPROM mỗi 5 phút (tránh ghi quá nhiều)
    unsigned long now = millis();
    if (now - _lastEepromSave >= 300000UL) { // 5 phút = 300000ms
        _lastEepromSave = now;
        _configManager.saveRuntime();
        _configManager.saveAdaptive();
        Serial.println(F("[LOG] Luu runtime vao EEPROM"));
    }

    _setState(SystemState::IDLE);
}

// ============================================================
// ERROR: Xử lý lỗi và thử phục hồi
// ============================================================
void StateMachine::_handleError() {
    // Đảm bảo bơm TẮT khi ERROR
    if (_relayDriver.isOn()) {
        _relayDriver.off();
    }

    // Lần đầu vào ERROR → ghi nhận thời gian
    if (_errorStartTime == 0) {
        _errorStartTime = millis();
        Serial.printf("[SM] === LOI: %d ===\n", (int)_currentError);
    }

    // Thử phục hồi sau 10 giây
    if (millis() - _errorStartTime >= 10000) {
        Serial.println(F("[SM] Thu phuc hoi tu loi..."));

        // Kiểm tra lại cảm biến
        ErrorCode err = _safetyManager.checkSensors(
            _sensorDriver.getDhtFailCount(),
            _sensorDriver.getSoilFailCount()
        );

        if (err == ErrorCode::NONE) {
            // Cảm biến đã hoạt động lại → phục hồi
            Serial.println(F("[SM] Phuc hoi thanh cong! → IDLE"));
            _currentError = ErrorCode::NONE;
            _safetyManager.clearError();
            _errorStartTime = 0;
            _mode = OperationMode::AUTO; // An toàn: về AUTO
            _setState(SystemState::IDLE);
        } else {
            // Vẫn lỗi → reset timer, thử lại
            _errorStartTime = millis();
            Serial.println(F("[SM] Van loi - thu lai sau 10 giay"));
        }
    }
}

// ============================================================
// Bắt đầu progressive watering
// ============================================================
void StateMachine::_startWatering(const IrrigationDecision &decision) {
    _targetPulses = decision.targetPulses;
    _currentPulse = 0;
    _totalWateringTime = 0;
    _soilBeforeWatering = _sensorData.soilPercent;

    // Bắt đầu xung đầu tiên
    _relayDriver.on();
    _pulseStartTime = millis();
    _waterSubState = WateringSubState::PUMP_ON;

    Serial.printf("[SM] === BAT DAU TUOI: %d xung, ly do: %s ===\n",
                  _targetPulses, decision.reason.c_str());
}

// ============================================================
// Dừng watering
// ============================================================
void StateMachine::_stopWatering(const String &reason) {
    _relayDriver.off();
    _waterSubState = WateringSubState::COMPLETE;

    Serial.printf("[SM] === KET THUC TUOI: %s (tong %ds) ===\n",
                  reason.c_str(), _totalWateringTime);

    // Cập nhật runtime data
    RuntimeData &runtime = _configManager.getRuntime();
    runtime.todayWateringSec += _totalWateringTime;
    runtime.lastWateringTime = _rtcDriver.getUnixTime();

    // Ghi nhận phiên tưới cho adaptive engine
    _adaptiveEngine.recordSession(
        _soilBeforeWatering, _sensorData.soilPercent,
        _totalWateringTime, _configManager.getAdaptive());

    // Cập nhật trạng thái tưới cho irrigation service (hysteresis)
    // → _wasWatering sẽ được set false trong evaluate() khi đất đủ ẩm

    // Lưu ngay vào EEPROM
    _configManager.saveRuntime();
    _configManager.saveAdaptive();
}

// ============================================================
// Kiểm tra an toàn MANUAL mode
// ============================================================
void StateMachine::_checkManualSafety() {
    const SystemConfig &cfg = _configManager.getConfig();
    unsigned long now = millis();

    // An toàn 1: Đất dưới ngưỡng nguy hiểm liên tục quá thời gian cấu hình → về AUTO
    if (_sensorData.soilValid && _sensorData.soilPercent <= cfg.dangerThreshold) {
        if (_manualDangerStartTime == 0) {
            _manualDangerStartTime = now;
            Serial.printf("[SM] MANUAL: Dat nguy hiem %d%%, bat dau dem %d phut\n",
                          _sensorData.soilPercent, cfg.manualTimeoutMin);
        } else {
            uint32_t dangerMs = (uint32_t)cfg.manualTimeoutMin * 60UL * 1000UL;
            if (now - _manualDangerStartTime >= dangerMs) {
                Serial.printf("[SM] MANUAL: Dat nguy hiem lien tuc > %d phut → AUTO\n",
                              cfg.manualTimeoutMin);
                _relayDriver.off();
                _mode = OperationMode::AUTO;
                _manualDangerStartTime = 0;
                _configManager.getConfigMutable().mode = (uint8_t)OperationMode::AUTO;
                _configManager.saveConfig();
                return;
            }
        }
    } else {
        _manualDangerStartTime = 0;
    }

    // An toàn 2: Bơm đang chạy → kiểm tra timeout bơm
    if (_relayDriver.isOn()) {
        if (_safetyManager.checkPumpTimeout(
                _relayDriver.getOnDurationSec(), cfg.maxPumpTimeMin)) {
            Serial.println(F("[SM] Pump timeout trong MANUAL! Tat bom."));
            _relayDriver.off();
            _currentError = ErrorCode::PUMP_TIMEOUT;
            _mode = OperationMode::AUTO;
            _setState(SystemState::ERROR);
            return;
        }

        // An toàn 3: Đất đã đủ ẩm → tắt bơm
        if (_sensorData.soilValid && 
            _safetyManager.isSoilWetEnough(_sensorData.soilPercent, 
                                            cfg.soilThresholdHigh)) {
            Serial.println(F("[SM] Dat du am trong MANUAL → tat bom"));
            _relayDriver.off();
        }
    }
}

// ============================================================
// Chuyển trạng thái với debug logging
// ============================================================
void StateMachine::_setState(SystemState newState) {
    if (newState != _state) {
        Serial.printf("[SM] %s → %s\n", STATE_NAMES[(int)_state], 
                      STATE_NAMES[(int)newState]);
        _state = newState;
    }
}
