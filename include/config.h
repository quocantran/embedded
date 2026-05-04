
#ifndef CONFIG_H
#define CONFIG_H

#define PIN_SDA             21
#define PIN_SCL             22

#define PIN_DHT             15
#define PIN_SOIL            34

#define PIN_RELAY           18
#define PIN_BUTTON_D4       4
#define PIN_BUTTON_BOOT     0
#define USE_BOOT_BUTTON_DBG 1
#if USE_BOOT_BUTTON_DBG
#define PIN_BUTTON          PIN_BUTTON_BOOT
#else
#define PIN_BUTTON          PIN_BUTTON_D4
#endif
#define PIN_STATUS_LED      2

#define LCD_I2C_ADDR        0x27
#define EEPROM_I2C_ADDR     0x57

#define SENSOR_READ_INTERVAL_MS 2000
#define LCD_UPDATE_INTERVAL_MS  500
#define LCD_BLINK_INTERVAL_MS   300
#define BUTTON_DEBOUNCE_MS      20
#define BUTTON_LONG_PRESS_MS    3000

#define DEFAULT_WATERING_PULSE_SEC 30
#define MIN_WATERING_PULSE_SEC  5
#define MAX_WATERING_PULSE_SEC  120
#define DEFAULT_WATERING_CHECK_DELAY_SEC 5
#define MIN_WATERING_CHECK_DELAY_SEC 1
#define MAX_WATERING_CHECK_DELAY_SEC 120
#define MAX_PUMP_TIME_SEC       300
#define DEFAULT_COOLDOWN_MIN    30
#define DEFAULT_WATER_BUDGET_SEC 600
#define MANUAL_TIMEOUT_SEC      60

#define DHT_TYPE                DHT11
#define SOIL_SAMPLE_COUNT       5
#define DEFAULT_SOIL_DRY_RAW    4095
#define DEFAULT_SOIL_WET_RAW    1500
#define DEFAULT_SOIL_LOW        30
#define DEFAULT_SOIL_HIGH       70
#define DEFAULT_DANGER_LEVEL    15
#define SENSOR_FAIL_MAX_COUNT   5

#define EEPROM_MAGIC_BYTE       0xAB
#define EEPROM_DATA_VERSION     0x06
#define EEPROM_PAGE_SIZE        32
#define EEPROM_WRITE_DELAY_MS   5
#define MAX_SCHEDULES           4

#define EEPROM_ADDR_MAGIC       0x0000
#define EEPROM_ADDR_VERSION     0x0001
#define EEPROM_ADDR_CRC_CFG     0x0002
#define EEPROM_ADDR_CONFIG      0x0010
#define EEPROM_ADDR_SCHEDULE    0x0120
#define EEPROM_ADDR_RUNTIME     0x0160
#define EEPROM_ADDR_ADAPTIVE    0x0180

#define LCD_COLS                16
#define LCD_ROWS                2

#define WIFI_STA_SSID           "BackCloudy"
#define WIFI_STA_PASS           "17052004"
#define WIFI_CONNECT_TIMEOUT_MS 15000
#define WEB_SERVER_PORT         80

#define NTP_SERVER              "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC      25200
#define NTP_DAYLIGHT_OFFSET_SEC 0

#define PUMP_FAIL_CHECK_SEC     60
#define SOIL_RESPONSE_MIN_PCT   5

#endif
