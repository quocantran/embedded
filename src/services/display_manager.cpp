
#include "services/display_manager.h"

void DisplayManager::init(LcdDriver &lcd) {
    _lcd = &lcd;
    _lastUpdateTime = 0;
}

void DisplayManager::update(const SensorData &sensors, OperationMode mode,
                            SystemState state, bool pumpOn, ErrorCode error,
                            const String &ipAddress, uint8_t dangerThreshold) {
    if (!_lcd) return;

    unsigned long now = millis();

    // Hien thi thong bao tam thoi (uu tien cao) de thay doi ro rang khi bam nut
    if (now < _tempMsgUntil) {
        _lcd->stopBlink();
        _lcd->printLine(0, _tempLine0);
        _lcd->printLine(1, _tempLine1);
        _lcd->updateBlink();
        return;
    }

    // Giới hạn tốc độ cập nhật LCD
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

void DisplayManager::_showDroughtAlert() {
    _lcd->printLine(0, "TUOI NUOC NGAY!");
    _lcd->printLine(1, "DAT QUA KHO!");
}

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

void DisplayManager::showTemporaryMessage(const String &line0, const String &line1,
                                          unsigned long durationMs) {
    if (!_lcd) return;

    _tempLine0 = line0;
    _tempLine1 = line1;
    _tempMsgUntil = millis() + durationMs;
}
