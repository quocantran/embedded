
#include <Arduino.h>
#include "app/state_machine.h"
#include "app/web_server.h"

StateMachine stateMachine;
WebServerManager webServer;

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

void loop() {
    stateMachine.update();
    webServer.update();
    yield();
}