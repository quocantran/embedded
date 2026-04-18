# 🌱 Hệ Thống Tưới Tiêu Thông Minh ESP32

## Tổng Quan

Đây là hệ thống tưới thông minh chạy trên ESP32, được thiết kế theo kiến trúc phân lớp và máy trạng thái. Hệ thống không chỉ bật/tắt bơm theo 1 ngưỡng đơn giản mà kết hợp nhiều cơ chế:

- Quyết định tưới đa yếu tố (đất, nhiệt độ, ẩm không khí, thời điểm trong ngày)
- Tưới theo xung (progressive watering) để tránh quá tay
- Lịch tưới thông minh có điều chỉnh theo độ ẩm thực tế
- Tự học phản hồi đất để tinh chỉnh thời gian tưới
- Nhiều lớp an toàn: timeout bơm, lỗi cảm biến, hiệu quả bơm, ngân sách nước/ngày

## Điểm Nổi Bật "Thông Minh"

1. AUTO không tưới mù quáng:
- Chỉ tưới khi cảm biến đất hợp lệ
- Có hysteresis (ngưỡng thấp/ngưỡng cao)
- Có cooldown giữa 2 phiên tưới
- Có water budget mỗi ngày
- Có noon protection (11h-14h)
- Nếu đất ở mức nguy hiểm thì cho phép bỏ qua cooldown để cứu cây

2. Progressive watering:
- Mỗi xung bơm 30 giây
- Sau mỗi xung: tắt bơm, đợi 5 giây, đọc lại đất
- Dừng sớm nếu đủ ẩm hoặc dừng theo số xung mục tiêu

3. Adaptive engine:
- Học phản hồi đất sau mỗi phiên tưới (% tăng / giây tưới)
- EMA để làm mượt dữ liệu
- Đất phản hồi tốt thì giảm thời gian tưới
- Đất phản hồi kém thì tăng thời gian tưới trong giới hạn an toàn

4. Scheduler thông minh:
- Tối đa 4 lịch
- Kích hoạt theo giờ + phút + ngày trong tuần
- Điều chỉnh thời lượng theo độ ẩm đất ngay tại thời điểm kích hoạt
- Mỗi slot chỉ kích hoạt 1 lần/ngày

---

## Phần Cứng

| Linh kiện | Mô tả | Ghi chú |
|---|---|---|
| ESP32 DevKit V1 | Vi điều khiển chính | Board mục tiêu `esp32doit-devkit-v1` |
| DHT11 | Cảm biến nhiệt độ và ẩm không khí | Dùng để hỗ trợ quyết định AUTO |
| Cảm biến đất analog | Đo độ ẩm đất qua ADC | Khuyến nghị loại capacitive |
| DS1307 + AT24C32 | RTC + EEPROM ngoài | RTC cho thời gian, EEPROM cho lưu bền |
| LCD I2C 16x2 | Hiển thị tại chỗ | Địa chỉ mặc định `0x27` |
| Nút nhấn | Điều khiển MANUAL | Dùng internal pull-up |
| Relay 1 kênh | Điều khiển bơm | Logic HIGH = bật |
| Bơm nước DC | Bơm tưới | Dùng nguồn phù hợp công suất |

### Sơ Đồ Chân Kết Nối ESP32

| Chân ESP32 | Thiết bị | Ghi chú |
|---|---|---|
| GPIO21 | SDA LCD + SDA DS1307/AT24C32 | Bus I2C |
| GPIO22 | SCL LCD + SCL DS1307/AT24C32 | Bus I2C |
| GPIO15 | DATA DHT11 | Pull-up 10K lên 3.3V |
| GPIO34 | AO cảm biến đất | ADC1_CH6 |
| GPIO18 | IN relay | Điều khiển bơm |
| GPIO4 | Nút nhấn | Chân còn lại nối GND |
| 3.3V | DHT11, cảm biến đất | Nguồn 3.3V |
| 5V | LCD, DS1307, relay | Nguồn 5V |
| GND | Tất cả module | Chung mass |

---

## Cấu Hình Mặc Định Quan Trọng

Các hằng số nằm trong `include/config.h`:

- Chu kỳ đọc cảm biến: 2 giây
- Cập nhật LCD: 500ms
- Long press: 3 giây
- Mỗi xung tưới: 30 giây
- Đợi giữa các xung: 5 giây
- Max pump time: 5 phút
- Cooldown mặc định: 30 phút
- Water budget mặc định: 600 giây/ngày
- Ngưỡng đất mặc định: low=30%, high=70%, danger=15%
- MANUAL danger timeout mặc định: 1 phút
- NTP: `pool.ntp.org`, GMT+7

WiFi hiện tại chạy Station mode với cấu hình trong `config.h`:

- `WIFI_STA_SSID`
- `WIFI_STA_PASS`

---

## Kiến Trúc Phần Mềm

```
APP LAYER
  - StateMachine
  - WebServer

SERVICE LAYER
  - IrrigationService
  - SchedulerService
  - AdaptiveEngine
  - SafetyManager
  - DisplayManager
  - ConfigManager

DRIVER LAYER
  - SensorDriver
  - RtcDriver
  - RelayDriver
  - ButtonDriver
  - LcdDriver
  - EepromDriver
```

### Cấu Trúc Thư Mục

```
embeddeed/
├── platformio.ini
├── README.md
├── include/
│   ├── config.h
│   ├── types.h
│   ├── app/
│   ├── drivers/
│   └── services/
└── src/
    ├── main.cpp
    ├── app/
    ├── drivers/
    └── services/
```

---

## Máy Trạng Thái

Trạng thái chính:

1. IDLE
2. READ_SENSOR
3. ANALYZE
4. DECIDE
5. WATERING
6. LOGGING
7. ERROR

Luồng cơ bản:

```
IDLE -> READ_SENSOR -> ANALYZE -> DECIDE -> WATERING/LOGGING -> IDLE
                         \-> ERROR (khi có lỗi)
```

### Khởi Động (setup)

1. `stateMachine.init()` khởi tạo toàn bộ driver/service
2. LCD hiển thị boot screen: `SMART IRRIGATION` + `DANG KHOI DONG`
3. Kết nối WiFi STA
4. Đồng bộ NTP và cập nhật RTC chip
5. Bắt đầu vòng loop

Lưu ý: Trong lúc chờ WiFi/NTP, LCD vẫn giữ màn hình `DANG KHOI DONG`.

---

## Cơ Chế Tưới Theo Từng Chế Độ

## AUTO

Thứ tự đánh giá trong code:

1. `soilValid` phải hợp lệ
2. Hysteresis:
- `soil >= high` -> không tưới
- `low <= soil < high` và không trong trạng thái đã tưới -> không tưới
3. Cooldown theo `lastWateringTime`
- Nếu chưa qua cooldown -> không tưới
- Ngoại lệ: `soil <= dangerThreshold` -> bỏ qua cooldown
4. Water budget ngày
5. Noon protection (11h-14h) nếu RTC hợp lệ
6. Tính mức tưới (SHORT/MEDIUM/LONG) theo đất + nhiệt + ẩm KK
7. Chuyển sang số xung và thời lượng
8. Giới hạn theo ngân sách còn lại

## SCHEDULE

- Cần RTC hợp lệ
- Chỉ kiểm tra mỗi phút 1 lần (tránh lặp)
- Điều kiện kích hoạt lịch:
  - Lịch bật
  - Chưa kích hoạt trong ngày
  - Giờ/phút trùng
  - Ngày trong tuần khớp bitmask
- Điều chỉnh thời lượng lịch:
  - Đất >= high -> bỏ qua
  - low <= đất < high -> 50%
  - đất < low -> 100%
- Mỗi slot (Lịch 1..4) chỉ chạy 1 lần/ngày
- Sang ngày mới thì reset cờ chạy lịch

## MANUAL

- Long press 3 giây: AUTO/SCHEDULE <-> MANUAL
- Short press khi ở MANUAL: toggle bơm
- Short press ngoài MANUAL: bỏ qua

An toàn trong MANUAL:

1. Nếu đất <= dangerThreshold liên tục quá `manualTimeoutMin` -> tắt bơm và về AUTO
2. Nếu bơm quá `maxPumpTimeMin` -> tắt bơm, vào ERROR
3. Nếu đất đã đủ ẩm (>= high) và bơm đang chạy -> tắt bơm

---

## Progressive Watering

Khi đã quyết định tưới, hệ thống chạy theo xung:

1. Bật bơm 30 giây
2. Tắt bơm
3. Đợi 5 giây
4. Đọc lại đất
5. Kiểm tra điều kiện dừng:
- Đất đạt ngưỡng cao
- Đủ số xung mục tiêu
- Timeout bơm
- Bơm không hiệu quả (sau 60 giây mà đất tăng < 5%)

---

## Adaptive Engine

Sau mỗi phiên tưới, hệ thống ghi nhận:

- `soilBefore`, `soilAfter`, `durationSec`
- Tính response rate
- Cập nhật EMA (`alpha = 0.3`)

Điều chỉnh:

- `avgResponseRate > 0.5` -> giảm ~20% thời gian tưới
- `0 < avgResponseRate < 0.1` -> tăng ~30% (giới hạn max)

Theo dõi xu hướng khô:

- Đất giảm liên tục -> tăng `dryTrendCounter`
- Counter > 5 -> cảnh báo xu hướng khô

---

## Lưu Trữ EEPROM

Dùng AT24C32, có header và CRC.

Layout chính:

- `0x0000`: Magic
- `0x0001`: Version
- `0x0002`: CRC config
- `0x0010`: `SystemConfig`
- `0x0040`: `ScheduleEntry[4]`
- `0x0060`: `RuntimeData`
- `0x0080`: `AdaptiveData`

Dữ liệu được lưu khi:

- Cập nhật config/schedule từ web
- Kết thúc phiên tưới (runtime/adaptive)
- Chu kỳ logging định kỳ

---

## Hiển Thị LCD

LCD hiện có 1 trang chính và 2 trạng thái ưu tiên:

1. Boot:
- `SMART IRRIGATION`
- `DANG KHOI DONG`

2. Main:
- Dòng 1: `HH:MM  TT°C`
- Dòng 2: `D:XX% M:auto/manual/schedule`

3. Cảnh báo khô:
- `TUOI NUOC NGAY!`
- `DAT QUA KHO!`

4. Lỗi:
- `LOI HE THONG!`
- Mô tả lỗi rút gọn

Backlight nhấp nháy khi cảnh báo khô hoặc ERROR.

---

## Web Server

Hệ thống dùng Async Web Server trên cổng 80.

Truy cập:

1. ESP32 kết nối vào WiFi nhà (STA)
2. Xem Serial để lấy IP do router cấp
3. Mở `http://<ip>`

### Endpoint

- `GET /` -> giao diện web
- `GET /api/status` -> trạng thái + config + lịch
- `POST /api/config` -> cập nhật cấu hình
- `POST /api/schedule` -> cập nhật lịch
- `POST /api/mode` -> yêu cầu đổi mode
- `POST /api/calibrate` -> hiệu chuẩn đất

### Validation web hiện tại

`/api/config`:
- `low < high` bắt buộc

`/api/schedule`:
- `hour` 0..23
- `minute` 0..59
- `duration` 10..600 giây
- Lịch bật phải có ít nhất 1 ngày

`/api/calibrate`:
- `dry > wet`

### Frontend behavior

- Poll trạng thái mỗi 5 giây
- Không ghi đè form nếu người dùng đang nhập
- Chỉ render lại danh sách lịch khi dữ liệu lịch thay đổi

---

## An Toàn Hệ Thống

| Cơ chế | Điều kiện | Hành động |
|---|---|---|
| Sensor fail | DHT/Soil lỗi liên tiếp >= 5 | Chuyển ERROR |
| Pump timeout | Bơm chạy quá giới hạn | Tắt bơm, ERROR |
| Pump effectiveness | Sau 60s tưới đất tăng < 5% | Tắt bơm, ERROR |
| Manual danger timeout | Đất nguy hiểm kéo dài trong MANUAL | Về AUTO |
| Soil wet stop (MANUAL) | Đất >= high khi bơm đang chạy | Tắt bơm |
| Water budget | Dùng hết ngân sách ngày | Chặn tưới |
| Cooldown | Chưa qua thời gian chờ | Chặn tưới (trừ đất nguy hiểm) |
| Noon protection | 11h-14h (khi RTC hợp lệ) | Hoãn tưới AUTO |

---

## Hướng Dẫn Vận Hành

### 1) Nạp firmware

```bash
pio run
pio run -t upload
pio device monitor
```

### 2) Cấu hình lần đầu

1. Bật hệ thống, chờ qua màn hình `DANG KHOI DONG`
2. Mở Serial Monitor, lấy IP truy cập web
3. Vào web, hiệu chuẩn cảm biến đất (`dry`, `wet`)
4. Đặt ngưỡng low/high/danger
5. Đặt budget/cooldown/max pump
6. Lưu cấu hình

### 3) Dùng AUTO

- Chọn mode AUTO từ web hoặc long press để về AUTO
- Hệ thống tự quyết định và tưới theo xung

### 4) Dùng SCHEDULE

- Tạo lịch với giờ/phút/thời lượng/ngày
- Chọn mode SCHEDULE
- Lưu ý slot đã chạy trong ngày sẽ không chạy lại trong cùng ngày

### 5) Dùng MANUAL

- Long press vào MANUAL
- Short press để bật/tắt bơm
- Long press để thoát MANUAL
- Nếu đất nguy hiểm kéo dài quá thời gian cấu hình thì hệ thống tự về AUTO

---

## Checklist Kiểm Thử Nhanh

1. Boot screen hiển thị đúng khi chờ WiFi/NTP
2. Web truy cập được bằng IP do router cấp
3. AUTO với đất khô (< low) kích hoạt tưới
4. AUTO với đất nguy hiểm bỏ qua cooldown
5. AUTO trong khung 11h-14h bị hoãn
6. SCHEDULE kích hoạt đúng phút và đúng ngày
7. Slot lịch không chạy lại trong cùng ngày
8. MANUAL short press toggle bơm đúng
9. MANUAL tự thoát chỉ khi đất nguy hiểm kéo dài quá timeout
10. ERROR recovery thử lại sau 10 giây khi lỗi cảm biến được khắc phục

---

## Thư Viện Sử Dụng (platformio.ini)

| Thư viện | Mục đích |
|---|---|
| LiquidCrystal_I2C | LCD I2C |
| DHT sensor library | DHT11 |
| Adafruit Unified Sensor | Dependency DHT |
| RTClib | RTC DS1307 |
| AsyncTCP | Nền tảng TCP bất đồng bộ |
| ESP Async WebServer | Web server bất đồng bộ |
| ArduinoJson | JSON API |

---

## Ghi Chú

- Web chỉ ghi cấu hình. Quyết định relay luôn do state machine.
- Nếu WiFi lỗi, hệ thống vẫn chạy OFFLINE (sensor + LCD + logic tưới).
- Nên theo dõi Serial log khi tinh chỉnh ngưỡng để hiểu quyết định tưới.

## Giấy Phép

Dự án phục vụ học tập và nghiên cứu hệ thống nhúng.
