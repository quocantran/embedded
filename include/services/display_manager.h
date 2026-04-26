
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "drivers/lcd_driver.h"

class DisplayManager {
public:
    void init(LcdDriver &lcd);

    void update(const SensorData &sensors, OperationMode mode,
                SystemState state, bool pumpOn, ErrorCode error,
                const String &ipAddress, uint8_t dangerThreshold);

    void showTemporaryMessage(const String &line0, const String &line1,
                              unsigned long durationMs = 1200);

private:
    LcdDriver *_lcd = nullptr;
    unsigned long _lastUpdateTime = 0;
    unsigned long _tempMsgUntil = 0;
    String _tempLine0 = "";
    String _tempLine1 = "";

    void _showMain(const SensorData &sensors, OperationMode mode);

    void _showDroughtAlert();

    void _showError(ErrorCode error);
};

#endif // DISPLAY_MANAGER_H
