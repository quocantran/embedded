
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
    String init(StateMachine &sm);

    void update();

private:
    AsyncWebServer _server = AsyncWebServer(WEB_SERVER_PORT);
    StateMachine *_sm = nullptr;

    void _setupRoutes();

    void _handleRoot(AsyncWebServerRequest *request);

    void _handleGetStatus(AsyncWebServerRequest *request);

    void _handlePostConfig(AsyncWebServerRequest *request);

    void _handlePostSchedule(AsyncWebServerRequest *request);

    void _handlePostMode(AsyncWebServerRequest *request);

    void _handlePostCalibrate(AsyncWebServerRequest *request);
};

#endif // WEB_SERVER_H
