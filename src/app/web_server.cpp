/**
 * @file web_server.cpp
 * @brief Triển khai web server cấu hình - WiFi Station Mode
 * 
 * NGUYÊN TẮC QUAN TRỌNG:
 * Web server CHỈ là giao diện cấu hình. Nó KHÔNG điều khiển trực tiếp relay.
 * Mọi thay đổi từ web được ghi vào config → State machine đọc config → quyết định.
 * 
 * WiFi Station Mode:
 * - Kết nối vào WiFi nhà (SSID/PASS từ config.h)
 * - IP do router cấp (hiện trên LCD + Serial)
 * - Port: 80
 */

#include "app/web_server.h"
#include "app/state_machine.h"
#include "app/web_page.h"
#include <ArduinoJson.h>

// ============================================================
// Khởi tạo WiFi STA và web server
// ============================================================
String WebServerManager::init(StateMachine &sm) {
    _sm = &sm;

    // --- Kết nối vào WiFi nhà (Station Mode) ---
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);

    Serial.println(F("\n[WEB] ================================"));
    Serial.printf("[WEB] Dang ket noi WiFi: %s\n", WIFI_STA_SSID);

    // Chờ kết nối (timeout = WIFI_CONNECT_TIMEOUT_MS)
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= WIFI_CONNECT_TIMEOUT_MS) {
            Serial.println(F("\n[WEB] LOI: Khong the ket noi WiFi!"));
            Serial.println(F("[WEB] Web server SE KHONG hoat dong."));
            Serial.println(F("[WEB] He thong van chay OFFLINE (cam bien + LCD)"));
            Serial.println(F("[WEB] ================================\n"));
            return ""; // Trả về rỗng = không có IP
        }
        delay(500);
        Serial.print(".");
    }

    String ip = WiFi.localIP().toString();
    Serial.println();
    Serial.printf("[WEB] WiFi da ket noi!\n");
    Serial.printf("[WEB] IP: %s\n", ip.c_str());
    Serial.printf("[WEB] Truy cap: http://%s\n", ip.c_str());
    Serial.println(F("[WEB] ================================\n"));

    // --- Đăng ký routes ---
    _setupRoutes();

    // --- Khởi động server ---
    _server.begin();
    Serial.println(F("[WEB] Web server da khoi dong"));

    return ip;
}

// ============================================================
// Cập nhật (kiểm tra WiFi vẫn kết nối)
// ============================================================
void WebServerManager::update() {
    // Kiểm tra WiFi có mất kết nối không
    if (WiFi.status() != WL_CONNECTED) {
        // Thử reconnect tự động (non-blocking, WiFi.begin tự reconnect)
        unsigned long now = millis();
        static unsigned long lastReconnect = 0;
        if (now - lastReconnect > 30000) { // Mỗi 30 giây thử lại
            lastReconnect = now;
            Serial.println(F("[WEB] WiFi mat ket noi - thu ket noi lai..."));
            WiFi.reconnect();
        }
    }
}

// ============================================================
// Đăng ký routes
// ============================================================
void WebServerManager::_setupRoutes() {
    // GET / → Trang HTML chính
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        _handleRoot(request);
    });

    // GET /api/status → Trạng thái JSON
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        _handleGetStatus(request);
    });

    // POST /api/config → Cập nhật cấu hình
    _server.on("/api/config", HTTP_POST, 
        [this](AsyncWebServerRequest *request) {
            _handlePostConfig(request);
        },
        NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, 
               size_t index, size_t total) {
            if (index == 0) {
                request->_tempObject = malloc(total + 1);
            }
            if (request->_tempObject) {
                memcpy((uint8_t*)request->_tempObject + index, data, len);
                if (index + len == total) {
                    ((char*)request->_tempObject)[total] = 0;
                }
            }
        }
    );

    // POST /api/schedule → Cập nhật lịch tưới
    _server.on("/api/schedule", HTTP_POST,
        [this](AsyncWebServerRequest *request) {
            _handlePostSchedule(request);
        },
        NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
               size_t index, size_t total) {
            if (index == 0) {
                request->_tempObject = malloc(total + 1);
            }
            if (request->_tempObject) {
                memcpy((uint8_t*)request->_tempObject + index, data, len);
                if (index + len == total) {
                    ((char*)request->_tempObject)[total] = 0;
                }
            }
        }
    );

    // POST /api/mode → Yêu cầu đổi chế độ
    _server.on("/api/mode", HTTP_POST,
        [this](AsyncWebServerRequest *request) {
            _handlePostMode(request);
        },
        NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
               size_t index, size_t total) {
            if (index == 0) {
                request->_tempObject = malloc(total + 1);
            }
            if (request->_tempObject) {
                memcpy((uint8_t*)request->_tempObject + index, data, len);
                if (index + len == total) {
                    ((char*)request->_tempObject)[total] = 0;
                }
            }
        }
    );

    // POST /api/calibrate → Hiệu chuẩn
    _server.on("/api/calibrate", HTTP_POST,
        [this](AsyncWebServerRequest *request) {
            _handlePostCalibrate(request);
        },
        NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
               size_t index, size_t total) {
            if (index == 0) {
                request->_tempObject = malloc(total + 1);
            }
            if (request->_tempObject) {
                memcpy((uint8_t*)request->_tempObject + index, data, len);
                if (index + len == total) {
                    ((char*)request->_tempObject)[total] = 0;
                }
            }
        }
    );

    Serial.println(F("[WEB] Da dang ky tat ca routes"));
}

// ============================================================
// GET / → Trang HTML
// ============================================================
void WebServerManager::_handleRoot(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", WEB_PAGE_HTML);
}

// ============================================================
// GET /api/status → Trạng thái JSON
// ============================================================
void WebServerManager::_handleGetStatus(AsyncWebServerRequest *request) {
    if (!_sm) {
        request->send(500, "application/json", "{\"error\":\"SM null\"}");
        return;
    }

    SystemStatus status = _sm->getStatus();
    const SystemConfig &cfg = _sm->getConfigManager().getConfig();
    ScheduleEntry *schedules = _sm->getConfigManager().getSchedules();

    JsonDocument doc;

    if (status.sensors.dhtValid) {
        doc["temp"] = round(status.sensors.temperature * 10.0f) / 10.0f;
        doc["hum"] = round(status.sensors.humidity);
    } else {
        doc["temp"] = nullptr;
        doc["hum"] = nullptr;
    }

    if (status.sensors.soilValid) {
        doc["soil"] = status.sensors.soilPercent;
    } else {
        doc["soil"] = nullptr;
    }

    doc["pump"] = status.pumpRunning;
    doc["mode"] = (int)status.mode;
    doc["state"] = (int)status.state;
    doc["error"] = (int)status.lastError;
    doc["budgetLeft"] = status.dailyBudgetLeft;
    doc["pumpTime"] = status.pumpOnTimeSec;
    doc["todayUsed"] = status.todayWaterUsed;

    doc["low"] = cfg.soilThresholdLow;
    doc["high"] = cfg.soilThresholdHigh;
    doc["danger"] = cfg.dangerThreshold;
    doc["maxPump"] = cfg.maxPumpTimeMin;
    doc["budget"] = cfg.dailyWaterBudget;
    doc["cooldown"] = cfg.cooldownMinutes;
    doc["manTimeout"] = cfg.manualTimeoutMin;
    doc["calDry"] = cfg.soilDryRaw;
    doc["calWet"] = cfg.soilWetRaw;

    JsonArray schArr = doc["schedules"].to<JsonArray>();
    for (int i = 0; i < MAX_SCHEDULES; i++) {
        JsonObject sch = schArr.add<JsonObject>();
        sch["enabled"] = schedules[i].enabled;
        sch["hour"] = schedules[i].hour;
        sch["minute"] = schedules[i].minute;
        sch["days"] = schedules[i].daysMask;
        sch["duration"] = schedules[i].durationSec;
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

// ============================================================
// POST /api/config → Cập nhật cấu hình
// ============================================================
void WebServerManager::_handlePostConfig(AsyncWebServerRequest *request) {
    if (!_sm || !request->_tempObject) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"Không có dữ liệu\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (char*)request->_tempObject);
    free(request->_tempObject);
    request->_tempObject = nullptr;

    if (err) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"JSON lỗi\"}");
        return;
    }

    SystemConfig &cfg = _sm->getConfigManager().getConfigMutable();

    if (doc["low"].is<int>())        cfg.soilThresholdLow = doc["low"];
    if (doc["high"].is<int>())       cfg.soilThresholdHigh = doc["high"];
    if (doc["danger"].is<int>())     cfg.dangerThreshold = doc["danger"];
    if (doc["maxPump"].is<int>())    cfg.maxPumpTimeMin = doc["maxPump"];
    if (doc["budget"].is<int>())     cfg.dailyWaterBudget = doc["budget"];
    if (doc["cooldown"].is<int>())   cfg.cooldownMinutes = doc["cooldown"];
    if (doc["manTimeout"].is<int>()) cfg.manualTimeoutMin = doc["manTimeout"];

    if (cfg.soilThresholdLow >= cfg.soilThresholdHigh) {
        request->send(400, "application/json", 
                      "{\"ok\":false,\"msg\":\"Ngưỡng thấp phải nhỏ hơn ngưỡng cao\"}");
        return;
    }

    _sm->getConfigManager().saveConfig();

    Serial.println(F("[WEB] Cap nhat cau hinh tu web"));
    request->send(200, "application/json", "{\"ok\":true}");
}

// ============================================================
// POST /api/schedule → Cập nhật lịch tưới
// ============================================================
void WebServerManager::_handlePostSchedule(AsyncWebServerRequest *request) {
    if (!_sm || !request->_tempObject) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"Không có dữ liệu\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (char*)request->_tempObject);
    free(request->_tempObject);
    request->_tempObject = nullptr;

    if (err) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"JSON lỗi\"}");
        return;
    }

    ScheduleEntry *schedules = _sm->getConfigManager().getSchedules();
    JsonArray schArr = doc["schedules"];

    for (int i = 0; i < MAX_SCHEDULES && i < (int)schArr.size(); i++) {
        JsonObject sch = schArr[i];
        int enabled = sch["enabled"] | 0;
        int hour = sch["hour"] | 0;
        int minute = sch["minute"] | 0;
        int days = sch["days"] | 0;
        int duration = sch["duration"] | 60;

        if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
            request->send(400, "application/json",
                          "{\"ok\":false,\"msg\":\"Giờ/phút lịch không hợp lệ\"}");
            return;
        }

        if (duration < 10 || duration > 600) {
            request->send(400, "application/json",
                          "{\"ok\":false,\"msg\":\"Thời lượng lịch phải từ 10 đến 600 giây\"}");
            return;
        }

        if (enabled && ((days & 0x7F) == 0)) {
            request->send(400, "application/json",
                          "{\"ok\":false,\"msg\":\"Lịch bật phải chọn ít nhất 1 ngày\"}");
            return;
        }

        schedules[i].enabled = enabled ? 1 : 0;
        schedules[i].hour = (uint8_t)hour;
        schedules[i].minute = (uint8_t)minute;
        schedules[i].daysMask = (uint8_t)(days & 0x7F);
        schedules[i].durationSec = (uint16_t)duration;
    }

    _sm->getConfigManager().saveSchedules();

    Serial.println(F("[WEB] Cap nhat lich tuoi tu web"));
    request->send(200, "application/json", "{\"ok\":true}");
}

// ============================================================
// POST /api/mode → Yêu cầu đổi chế độ
// ============================================================
void WebServerManager::_handlePostMode(AsyncWebServerRequest *request) {
    if (!_sm || !request->_tempObject) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"Không có dữ liệu\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (char*)request->_tempObject);
    free(request->_tempObject);
    request->_tempObject = nullptr;

    if (err) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"JSON lỗi\"}");
        return;
    }

    int mode = doc["mode"] | -1;
    if (mode < 0 || mode > 2) {
        request->send(400, "application/json", 
                      "{\"ok\":false,\"msg\":\"Chế độ không hợp lệ (0-2)\"}");
        return;
    }

    _sm->requestModeChange((OperationMode)mode);

    Serial.printf("[WEB] Yeu cau doi che do: %d\n", mode);
    request->send(200, "application/json", "{\"ok\":true}");
}

// ============================================================
// POST /api/calibrate → Hiệu chuẩn cảm biến đất
// ============================================================
void WebServerManager::_handlePostCalibrate(AsyncWebServerRequest *request) {
    if (!_sm || !request->_tempObject) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"Không có dữ liệu\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (char*)request->_tempObject);
    free(request->_tempObject);
    request->_tempObject = nullptr;

    if (err) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"JSON lỗi\"}");
        return;
    }

    SystemConfig &cfg = _sm->getConfigManager().getConfigMutable();

    if (doc["dry"].is<int>()) cfg.soilDryRaw = doc["dry"];
    if (doc["wet"].is<int>()) cfg.soilWetRaw = doc["wet"];

    if (cfg.soilDryRaw <= cfg.soilWetRaw) {
        request->send(400, "application/json",
                      "{\"ok\":false,\"msg\":\"ADC khô phải lớn hơn ADC ướt\"}");
        return;
    }

    _sm->getConfigManager().saveConfig();

    Serial.printf("[WEB] Hieu chuan: kho=%d, uot=%d\n", cfg.soilDryRaw, cfg.soilWetRaw);
    request->send(200, "application/json", "{\"ok\":true}");
}
