/**
 * @file display_manager.cpp
 * @brief Triển khai hiển thị LCD đơn giản + cảnh báo khẩn
 * 
 * Bình thường:
 * ┌────────────────┐
 * │ HH:MM  TT°C    │
 * │ D:XX% M:auto    │
 * └────────────────┘
 * 
 * Khi đất < ngưỡng nguy hiểm:
 * ┌────────────────┐
 * │TUOI NUOC NGAY! │
 * │DAT QUA KHO!    │
 * └────────────────┘
 * + đèn nền nhấp nháy
 */

#include "services/display_manager.h"

// ============================================================
// Khởi tạo
// ============================================================
void DisplayManager::init(LcdDriver &lcd) {
    _lcd = &lcd;
    _lastUpdateTime = 0;
}

// ============================================================
// Cập nhật hiển thị LCD
// ============================================================
void DisplayManager::update(const SensorData &sensors, OperationMode mode,
                            SystemState state, bool pumpOn, ErrorCode error,
                            const String &ipAddress, uint8_t dangerThreshold) {
    if (!_lcd) return;

    // Giới hạn tốc độ cập nhật LCD
    unsigned long now = millis();
    if (now - _lastUpdateTime < LCD_UPDATE_INTERVAL_MS) {
        _lcd->updateBlink(); // Vẫn cập nhật blink
        return;
    }
    _lastUpdateTime = now;

    // ─── Kiểm tra lỗi hệ thống ───
    if (state == SystemState::ERROR) {
        _lcd->startBlink();
        _showError(error);
        _lcd->updateBlink();
        return;
    }

    // ─── Kiểm tra đất dưới ngưỡng nguy hiểm → cảnh báo ───
    if (sensors.soilValid && sensors.soilPercent < dangerThreshold) {
        _lcd->startBlink();
        _showDroughtAlert();
        _lcd->updateBlink();
        return;
    }

    // ─── Bình thường → tắt nhấp nháy, hiển thị dữ liệu ───
    _lcd->stopBlink();
    _showMain(sensors, mode);
    _lcd->updateBlink();
}

// ============================================================
// Trang chính: Giờ + Nhiệt độ + Ẩm đất + Mode
// Hàng 0: "HH:MM  TT°C"
// Hàng 1: "D:XX% M:auto/schedule/manual"
// ============================================================
void DisplayManager::_showMain(const SensorData &sensors, OperationMode mode) {
    // ─── Hàng 0: Giờ + Nhiệt độ ───
    String line0 = "";

    if (sensors.rtcValid) {
        if (sensors.hour < 10) line0 += "0";
        line0 += String(sensors.hour) + ":";
        if (sensors.minute < 10) line0 += "0";
        line0 += String(sensors.minute);
    } else {
        line0 += "--:--";
    }

    line0 += "  ";

    if (sensors.dhtValid) {
        line0 += String((int)sensors.temperature) + "\xDF" "C"; // \xDF = ° trên LCD
    } else {
        line0 += "--\xDF" "C";
    }

    _lcd->printLine(0, line0);

    // ─── Hàng 1: Ẩm đất + Mode ───
    String line1 = "";

    if (sensors.soilValid) {
        line1 += "D:" + String(sensors.soilPercent) + "%";
    } else {
        line1 += "D:ERR";
    }

    String modeText = "auto";
    switch (mode) {
        case OperationMode::AUTO:     modeText = "auto"; break;
        case OperationMode::SCHEDULE: modeText = "schedule"; break;
        case OperationMode::MANUAL:   modeText = "manual"; break;
        default:                      modeText = "auto"; break;
    }

    String modePart = "M:" + modeText;
    if (line1.length() + 1 + modePart.length() <= LCD_COLS) {
        line1 += " ";
    }
    line1 += modePart;

    if (line1.length() > LCD_COLS) {
        line1 = line1.substring(0, LCD_COLS);
    }

    _lcd->printLine(1, line1);
}

// ============================================================
// Cảnh báo đất khô
// Hàng 0: "TUOI NUOC NGAY!"
// Hàng 1: "DAT QUA KHO!"
// ============================================================
void DisplayManager::_showDroughtAlert() {
    _lcd->printLine(0, "TUOI NUOC NGAY!");
    _lcd->printLine(1, "DAT QUA KHO!");
}

// ============================================================
// Hiển thị lỗi
// ============================================================
void DisplayManager::_showError(ErrorCode error) {
    _lcd->printLine(0, "LOI HE THONG!");

    String errMsg = "";
    switch (error) {
        case ErrorCode::DHT_FAIL:     errMsg = "Loi DHT11"; break;
        case ErrorCode::SOIL_FAIL:    errMsg = "Loi cam bien dat"; break;
        case ErrorCode::RTC_FAIL:     errMsg = "Loi dong ho RTC"; break;
        case ErrorCode::EEPROM_FAIL:  errMsg = "Loi EEPROM"; break;
        case ErrorCode::PUMP_FAIL:    errMsg = "Loi may bom"; break;
        case ErrorCode::PUMP_TIMEOUT: errMsg = "Bom qua han"; break;
        default:                      errMsg = "Loi khong xac dinh"; break;
    }

    _lcd->printLine(1, errMsg);
}
