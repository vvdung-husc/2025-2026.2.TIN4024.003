# ESP8266 Blynk + Telegram (NodeMCU)

Hướng dẫn nhanh để chạy project trên NodeMCU (ESP8266).

Nội dung chính:

- Nối dây (theo `diagram.json`):
  - OLED (SH1106/SSD1306) I2C: SDA -> D2, SCL -> D1, VCC -> 3V3, GND -> GND
  - DHT22: DATA -> D3, VCC -> 3V3, GND -> GND
  - LED (3): qua trở lần lượt tới D5 (green), D6 (red), D7 (yellow) và các chân GND
  - MQ2 (nếu có): analog -> A0

- Cấu hình secrets: tạo hoặc sửa `src/secrets.h` (file mẫu đã có trong repo). Thêm thông tin WiFi, Blynk token, Telegram bot token.
  - Lưu ý: không commit token thật lên Git.

- Build & nạp firmware (PlatformIO):
```powershell
pio run -e nodemcuv2
pio run -e nodemcuv2 -t upload
```

- Mô phỏng Wokwi:
  - Mở project Wokwi trong trình duyệt; `wokwi.toml` và `diagram.json` đã cập nhật để dùng `nodemcuv2`.

- Ghi chú thiết kế:
  - Code tách thành `main.cpp`, `blynk_service.*`, `telegram_service.*`.
  - `src/secrets.h` được thêm vào `.gitignore` để tránh lộ token.

Nếu muốn, tôi có thể chạy build (`pio run`) và gửi log lỗi nếu có.

---
Trả lời ngắn: muốn tôi chạy build kiểm tra bây giờ không?
