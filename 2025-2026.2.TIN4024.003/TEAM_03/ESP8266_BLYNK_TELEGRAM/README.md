# ESP8266 IoT Project - Blynk + Telegram + Sensors

## Features (Tính năng)

### 1. OLED Display (màn hình OLED)
- **Uptime**: Hiển thị thời gian hoạt động của thiết bị (HH:MM:SS)
- **LED Status**: Trạng thái LED (ON/OFF)
- **Temperature**: Nhiệt độ từ cảm biến DHT22 (°C)
- **Humidity**: Độ ẩm từ cảm biến DHT22 (%)
- **Gas Sensor**: Giá trị khí ga từ cảm biến MQ2 (ppm)

### 2. Blynk Integration (Tích hợp Blynk)
- **V0 (Uptime)**: Hiển thị thời gian hoạt động (read-only)
- **V1 (LED Switch)**: Điều khiển bật/tắt LED (read/write)
- **V2 (Temperature)**: Hiển thị nhiệt độ (read-only)
- **V3 (Humidity)**: Hiển thị độ ẩm (read-only)
- **V4 (Gas)**: Hiển thị giá trị khí ga (read-only)

### 3. Telegram Bot (Bot Telegram)
Commands (Lệnh):
- **on**: Bật LED
- **off**: Tắt LED
- **status**: Xem trạng thái thiết bị
- **/start**: Xem hướng dẫn

Auto Notifications (Thông báo tự động):
- Gửi thông tin nhiệt độ, độ ẩm khi có sự thay đổi ≥1°C hoặc ≥5%

## Hardware Setup (Kết nối phần cứng)

### GPIO Pins
```
LED_PIN    = D8 (GPIO15)      - Điều khiển LED
DHT_PIN    = D3 (GPIO0)       - Cảm biến DHT22
GAS_PIN    = A0               - Cảm biến MQ2

OLED I2C:
- SDA      = D2 (GPIO4)
- SCL      = D1 (GPIO5)
```

### Sensors
1. **DHT22**: Cảm biến nhiệt độ và độ ẩm
   - VCC → 3.3V
   - GND → GND
   - DATA → D3 (GPIO0)
   - Pull-up 10K Ω trên chân DATA

2. **OLED SH1106 128x64**: Màn hình I2C
   - VCC → 3.3V/5V
   - GND → GND
   - SDA → D2 (GPIO4)
   - SCL → D1 (GPIO5)

3. **MQ2 Gas Sensor**: Cảm biến khí ga (cho demo, có thể sinh ngẫu nhiên)
   - VCC → 5V
   - GND → GND
   - AOut → A0 (Analog Input)

4. **LED**: 
   - Anode → D8 (GPIO15)
   - Cathode → GND (với resistor ~330Ω)

## Software Setup (Cấu hình phần mềm)

### Prerequisites (Yêu cầu)
- PlatformIO IDE hoặc Arduino IDE
- ESP8266 board package

### Installation Steps (Các bước cài đặt)

1. **Clone hoặc download project này**

2. **Cấu hình WiFi** (tại `src/main.cpp`):
```cpp
char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PASSWORD";
```

3. **Cấu hình Blynk Token** (tại `src/main.cpp`):
```cpp
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
```

4. **Cấu hình Telegram** (tại `src/main.cpp`):
```cpp
#define BOT_TOKEN "YOUR_BOT_TOKEN"
#define CHAT_ID "YOUR_CHAT_ID"
```

5. **Build and Upload**:
```bash
platformio run -t upload
```

### Blynk Setup (Cấu hình Blynk)

1. Tạo project mới trên Blynk app
2. Thêm widgets:
   - **Value Display (V0)**: Uptime
   - **Switch (V1)**: LED Control
   - **Gauge (V2)**: Temperature (0-50°C)
   - **Gauge (V3)**: Humidity (0-100%)
   - **Gauge (V4)**: Gas (0-1000 ppm)
3. Copy Auth Token vào code
4. Nhấn Play để kết nối

### Telegram Setup (Cấu hình Telegram)

1. Tìm @BotFather trên Telegram
2. Tạo bot: `/newbot`
3. Copy Bot Token vào code
4. Tìm Chat ID của bạn hoặc nhóm
5. Cập nhật CHAT_ID trong code

## Pinout Diagram (Sơ đồ chân)

```
ESP8266 NodeMCU
┌─────────────────┐
│  D0(GPIO16)     ├─→ RST
│  D1(GPIO5)  SCL ├─→ OLED SCL
│  D2(GPIO4)  SDA ├─→ OLED SDA
│  D3(GPIO0)  DHT ├─→ DHT22 Data
│  D4(GPIO2)      │
│  D5(GPIO14)     │
│  D6(GPIO12)     │
│  D7(GPIO13)     │
│  D8(GPIO15) LED ├─→ LED Anode (+)
│  3.3V           ├─→ VCC
│  GND            ├─→ GND
│  A0             ├─→ MQ2 AOut
│  Vin            ├─→ 5V input
└─────────────────┘
```

## Troubleshooting (Khắc phục sự cố)

### OLED không hiển thị
- Kiểm tra cáp I2C (SDA, SCL)
- Kiểm tra nguồn điện (3.3V)
- Kiểm tra địa chỉ I2C (mặc định 0x3C)
- Giảm tốc độ I2C nếu cần

### DHT22 không đọc giá trị
- Kiểm tra cáp DATA vào D3
- Kiểm tra resistor pull-up 10K Ω
- Chờ 2 giây sau khi khởi động (DHT cần thời gian khởi tạo)

### Blynk không kết nối
- Kiểm tra WiFi SSID và password
- Kiểm tra Auth Token có chính xác không
- Đảm bảo board có internet

### Telegram Bot không phản hồi
- Kiểm tra Bot Token có chính xác không
- Kiểm tra CHAT_ID chính xác
- Đảm bảo board có internet

## Libraries Used (Thư viện sử dụng)

- **U8g2** (v2.35.30): OLED display control
- **DHT** (v1.4.6): DHT temperature/humidity sensor
- **Blynk** (v1.1.2): IoT platform
- **UniversalTelegramBot** (v1.3.0): Telegram bot
- **ArduinoJson** (v6.21.3): JSON parsing

## Code Structure (Cấu trúc code)

```
main.cpp
├── Configuration
│   ├── WiFi
│   ├── Blynk
│   ├── Telegram
│   └── Hardware pins
├── Blynk Virtual Pin Handlers
│   ├── BLYNK_READ(V0-V4)
│   └── BLYNK_WRITE(V1)
├── Functions
│   ├── connectWiFi()
│   ├── connectBlynk()
│   ├── readSensors()
│   ├── updateOledDisplay()
│   ├── sendTelegramMessage()
│   ├── handleTelegramMessages()
│   └── checkTempHumidityChange()
├── setup()
└── loop()
```

## Updates (Cập nhật)

- **Uptime**: Cập nhật mỗi 2 giây trên OLED, mỗi 10 giây trên Blynk
- **Sensors**: Đọc mỗi 5 giây
- **Telegram**: Kiểm tra lệnh mỗi 1 giây
- **OLED**: Cập nhật mỗi 2 giây

## License

Educational Purpose Only

## Support (Hỗ trợ)

Liên hệ với team hoặc kiểm tra Serial Monitor ở 115200 baud để debug.
