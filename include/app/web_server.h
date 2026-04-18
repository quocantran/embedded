/**
 * @file web_server.h
 * @brief Web server cấu hình hệ thống qua WiFi Station (kết nối WiFi nhà)
 * 
 * QUAN TRỌNG: Web chỉ để CẤU HÌNH, KHÔNG điều khiển realtime.
 * Web ghi vào config → State machine đọc config và quyết định.
 * 
 * API endpoints:
 * - GET  /              → Trang HTML cấu hình
 * - GET  /api/status    → Trạng thái hệ thống (JSON)
 * - POST /api/config    → Cập nhật cấu hình
 * - POST /api/schedule  → Cập nhật lịch tưới
 * - POST /api/mode      → Yêu cầu đổi chế độ (state machine quyết định)
 * - POST /api/calibrate → Hiệu chuẩn cảm biến đất
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "config.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Forward declaration - tránh include vòng
class StateMachine;

class WebServerManager {
public:
    /**
     * @brief Kết nối WiFi nhà và khởi động web server
     * @param sm Tham chiếu đến StateMachine
     * @return Địa chỉ IP do router cấp (rỗng nếu WiFi lỗi)
     */
    String init(StateMachine &sm);

    /**
     * @brief Kiểm tra WiFi vẫn hoạt động (gọi trong loop nếu cần)
     */
    void update();

private:
    AsyncWebServer _server = AsyncWebServer(WEB_SERVER_PORT);
    StateMachine *_sm = nullptr;

    /**
     * @brief Đăng ký tất cả route handlers
     */
    void _setupRoutes();

    /**
     * @brief Handler cho GET / (trang HTML chính)
     */
    void _handleRoot(AsyncWebServerRequest *request);

    /**
     * @brief Handler cho GET /api/status
     */
    void _handleGetStatus(AsyncWebServerRequest *request);

    /**
     * @brief Handler cho POST /api/config
     */
    void _handlePostConfig(AsyncWebServerRequest *request);

    /**
     * @brief Handler cho POST /api/schedule
     */
    void _handlePostSchedule(AsyncWebServerRequest *request);

    /**
     * @brief Handler cho POST /api/mode
     */
    void _handlePostMode(AsyncWebServerRequest *request);

    /**
     * @brief Handler cho POST /api/calibrate
     */
    void _handlePostCalibrate(AsyncWebServerRequest *request);
};

#endif // WEB_SERVER_H
