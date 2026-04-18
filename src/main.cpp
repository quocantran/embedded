/**
 * @file main.cpp
 * @brief Entry point - Hệ thống tưới tiêu thông minh ESP32
 * 
 * Thứ tự khởi tạo:
 * 1. State Machine (drivers + services)
 * 2. WiFi (kết nối vào WiFi nhà)
 * 3. NTP (đồng bộ giờ thực từ internet → cập nhật RTC chip)
 * 4. Web Server
 * 5. Vòng lặp chính (non-blocking)
 */

#include <Arduino.h>
#include "app/state_machine.h"
#include "app/web_server.h"

// ============================================================
// Đối tượng toàn cục
// ============================================================
StateMachine stateMachine;
WebServerManager webServer;

// ============================================================
// SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println(F("\n\n"));
    Serial.println(F("========================================"));
    Serial.println(F("  SMART IRRIGATION SYSTEM v1.0"));
    Serial.println(F("  He thong tuoi tieu thong minh"));
    Serial.println(F("  ESP32 + DHT11 + Soil + RTC + LCD"));
    Serial.println(F("========================================"));
    Serial.println();

    // --- Bước 1: Khởi tạo State Machine ---
    if (!stateMachine.init()) {
        Serial.println(F("[MAIN] LOI NGHIEM TRONG: Khong the khoi tao he thong!"));
    }

    // Giữ màn hình khởi động trong lúc chờ WiFi/NTP
    stateMachine.showBootScreen();

    // --- Bước 2: Kết nối WiFi + Web Server ---
    String ip = webServer.init(stateMachine);

    if (ip.length() > 0) {
        // WiFi kết nối OK → đồng bộ giờ NTP
        Serial.printf("[MAIN] Web: http://%s\n", ip.c_str());

        // --- Bước 3: Đồng bộ NTP → cập nhật RTC chip ---
        if (stateMachine.getRtcDriver().syncFromNTP()) {
            Serial.println(F("[MAIN] Gio da dong bo tu internet (NTP)"));
        } else {
            Serial.println(F("[MAIN] NTP loi - dung gio RTC cu"));
        }
    } else {
        Serial.println(F("[MAIN] WiFi loi - chay OFFLINE (khong co web, dung gio RTC cu)"));
    }

    Serial.println(F("\n[MAIN] ========== HE THONG SAN SANG ==========\n"));
}

// ============================================================
// LOOP
// ============================================================
void loop() {
    stateMachine.update();
    webServer.update();
    yield();
}