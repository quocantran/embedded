
#ifndef CONFIG_H
#define CONFIG_H


// --- Chân I2C (dùng chung cho LCD, RTC, EEPROM) ---
#define PIN_SDA             21      // GPIO21 - Chân dữ liệu I2C
#define PIN_SCL             22      // GPIO22 - Chân xung nhịp I2C

// --- Chân cảm biến ---
#define PIN_DHT             15      // GPIO15 - Chân dữ liệu DHT11
#define PIN_SOIL            34      // GPIO34 - Chân analog cảm biến đất (ADC1_CH6)

// --- Chân điều khiển ---
#define PIN_RELAY           18      // GPIO18 - Chân điều khiển relay bơm nước
#define PIN_BUTTON_D4       4       // GPIO4  - Nút nhấn ngoài (dang bi nhieu tren board hien tai)
#define PIN_BUTTON_BOOT     0       // GPIO0  - Nút BOOT onboard (debug on dinh)
#define USE_BOOT_BUTTON_DBG 1       // 1: dung BOOT de test, 0: dung D4
#if USE_BOOT_BUTTON_DBG
#define PIN_BUTTON          PIN_BUTTON_BOOT
#else
#define PIN_BUTTON          PIN_BUTTON_D4
#endif
#define PIN_STATUS_LED      2       // GPIO2  - LED onboard de test nhanh nut nhan

// --- Địa chỉ I2C các thiết bị ---
#define LCD_I2C_ADDR        0x27    // Địa chỉ LCD I2C 16x2
#define EEPROM_I2C_ADDR     0x57    // Địa chỉ AT24C32 (trên module DS1307)

#define SENSOR_READ_INTERVAL_MS 2000    // Chu kỳ đọc cảm biến: 2 giây
#define LCD_UPDATE_INTERVAL_MS  500     // Chu kỳ cập nhật LCD: 500ms
#define LCD_BLINK_INTERVAL_MS   300     // Tốc độ nhấp nháy cảnh báo LCD: 300ms
#define BUTTON_DEBOUNCE_MS      20      // Chống rung nút nhấn: 20ms (de nhan nut thuc te hon)
#define BUTTON_LONG_PRESS_MS    3000    // Nhấn giữ >= 3 giây = long press

#define WATERING_PULSE_SEC      30      // Mỗi xung tưới progressive = 30 giây
#define WATERING_CHECK_DELAY_SEC 5      // Đợi 5 giây sau mỗi xung để đọc lại đất
#define MAX_PUMP_TIME_SEC       300     // Giới hạn bơm liên tục tối đa = 5 phút
#define DEFAULT_COOLDOWN_MIN    30      // Chờ tối thiểu 30 phút giữa 2 lần tưới
#define DEFAULT_WATER_BUDGET_SEC 600    // Ngân sách nước mỗi ngày = 600 giây (10 phút)
#define MANUAL_TIMEOUT_SEC      60     // Manual mode tự tắt sau 1 phút

#define DHT_TYPE                DHT11   // Loại cảm biến nhiệt độ/ẩm
#define SOIL_SAMPLE_COUNT       5       // Số mẫu trung bình cảm biến đất (lọc nhiễu)
#define DEFAULT_SOIL_DRY_RAW    4095    // Giá trị ADC khi đất hoàn toàn khô
#define DEFAULT_SOIL_WET_RAW    1500    // Giá trị ADC khi đất ướt nhất (nhúng nước)
#define DEFAULT_SOIL_LOW        30      // Ngưỡng thấp (%) - bắt đầu xét tưới
#define DEFAULT_SOIL_HIGH       70      // Ngưỡng cao (%) - dừng tưới (hysteresis)
#define DEFAULT_DANGER_LEVEL    15      // Ngưỡng nguy hiểm (%) - cảnh báo khẩn
#define SENSOR_FAIL_MAX_COUNT   5       // Số lần đọc lỗi liên tiếp → chuyển ERROR

#define EEPROM_MAGIC_BYTE       0xAB    // Byte ma thuật xác nhận dữ liệu hợp lệ
#define EEPROM_DATA_VERSION     0x01    // Phiên bản cấu trúc dữ liệu
#define EEPROM_PAGE_SIZE        32      // Kích thước trang AT24C32 (bytes)
#define EEPROM_WRITE_DELAY_MS   5       // Thời gian chờ ghi trang EEPROM
#define MAX_SCHEDULES           4       // Số lịch tưới tối đa hỗ trợ

// --- Bảng địa chỉ EEPROM ---
#define EEPROM_ADDR_MAGIC       0x0000  // 1 byte  - Magic byte
#define EEPROM_ADDR_VERSION     0x0001  // 1 byte  - Phiên bản dữ liệu
#define EEPROM_ADDR_CRC_CFG     0x0002  // 1 byte  - CRC8 cho config block
#define EEPROM_ADDR_CONFIG      0x0010  // ~16 bytes - Khối cấu hình chính
#define EEPROM_ADDR_SCHEDULE    0x0040  // 32 bytes - 4 lịch tưới × 8 bytes
#define EEPROM_ADDR_RUNTIME     0x0060  // 8 bytes  - Dữ liệu runtime
#define EEPROM_ADDR_ADAPTIVE    0x0080  // 8 bytes  - Dữ liệu thích ứng

#define LCD_COLS                16      // Số cột LCD
#define LCD_ROWS                2       // Số hàng LCD

#define WIFI_STA_SSID           "Minh An"       // Tên WiFi nhà
#define WIFI_STA_PASS           "88888888"       // Mật khẩu WiFi
#define WIFI_CONNECT_TIMEOUT_MS 15000            // Timeout kết nối WiFi: 15 giây
#define WEB_SERVER_PORT         80               // Cổng web server

#define NTP_SERVER              "pool.ntp.org"   // Server NTP
#define NTP_GMT_OFFSET_SEC      25200            // GMT+7 (Việt Nam) = 7*3600
#define NTP_DAYLIGHT_OFFSET_SEC 0                // Không có DST

#define PUMP_FAIL_CHECK_SEC     60      // Sau 60s tưới mà đất không tăng → cảnh báo
#define SOIL_RESPONSE_MIN_PCT   5       // Đất phải tăng ít nhất 5% sau kiểm tra bơm

#endif // CONFIG_H
