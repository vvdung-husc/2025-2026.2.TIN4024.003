# ESP8266_BLYNK_TELEGRAM - Team 11

## Thông tin nhóm
**Nhóm 11**

1. Trần Quốc Tiến  
2. Đặng Tấn Phát  
3. Trần Đức Quốc Chí  
4. Hồ Văn Thạnh  
5. Lê Tấn Toàn  

## Giới thiệu dự án
Dự án **ESP8266_BLYNK_TELEGRAM** mô phỏng hệ thống IoT giám sát và điều khiển thiết bị bằng **ESP32**, **Blynk IoT Cloud** và **Telegram Bot**.

Hệ thống có các chức năng chính:
- Hiển thị thời gian hoạt động của thiết bị (uptime)
- Điều khiển bật/tắt đèn LED bằng Blynk và Telegram
- Đo nhiệt độ và độ ẩm bằng cảm biến **DHT22**
- Đọc giá trị khí gas từ cảm biến **MQ2**
- Gửi cảnh báo khí gas lên Telegram khi vượt ngưỡng
- Gửi thông tin nhiệt độ, độ ẩm khi có thay đổi
- Đồng bộ trạng thái LED giữa nút nhấn vật lý, Blynk và Telegram

## Phần cứng sử dụng
- ESP32
- Cảm biến nhiệt độ và độ ẩm DHT22
- Cảm biến khí gas MQ2
- LED
- Nút nhấn
- Blynk IoT Cloud
- Telegram Bot
- Wokwi Simulator

## Sơ đồ kết nối
- **DHT22**
  - DATA → GPIO 4
- **LED**
  - Anode → GPIO 23
- **MQ2**
  - AO → GPIO 34
- **Button**
  - Signal → GPIO 22

## Chức năng trên Blynk
Hệ thống hiển thị và điều khiển trên Blynk gồm:

- **V0**: Switch điều khiển bật/tắt LED
- **V1**: Nhiệt độ
- **V2**: Độ ẩm
- **V3**: Giá trị khí gas
- **V4**: Uptime

Ngoài ra, giao diện Blynk có hiển thị thông tin **Team 11** ở cuối dashboard.

## Chức năng trên Telegram
Bot Telegram hỗ trợ các lệnh sau:

- `/start` : Hiển thị thông tin nhóm và hướng dẫn sử dụng
- `/led_on` : Bật đèn LED
- `/led_off` : Tắt đèn LED
- `/led_status` : Kiểm tra trạng thái LED
- `/get_weather` : Xem nhiệt độ và độ ẩm hiện tại

Ngoài ra:
- Bot tự động gửi thông báo khi **nhiệt độ/độ ẩm thay đổi**
- Bot tự động gửi **cảnh báo khí gas** khi giá trị gas vượt ngưỡng an toàn

## Cấu trúc thư mục
```text
ESP8266_BLYNK_TELEGRAM/
├── image/
│   ├── blynk.png
│   └── tele.png
├── include/
├── lib/
├── src/
│   └── main.cpp
├── test/
├── diagram.json
├── platformio.ini
├── wokwi.toml
└── README.md