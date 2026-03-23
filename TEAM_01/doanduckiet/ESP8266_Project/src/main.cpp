#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <U8g2lib.h>

// ================= CẤU HÌNH CHÂN LINH KIỆN =================
#define LED_BUILTIN 2   // Chân LED tích hợp trên board ESP8266
#define DHTPIN 0        // Chân D3 (GPIO0) nối với cảm biến DHT
#define DHTTYPE DHT22   // Loại cảm biến (Nếu bạn dùng cục màu xanh thì đổi thành DHT11)

#define OLED_SDA 4      // Chân D2
#define OLED_SCL 5      // Chân D1

// ================= KHỞI TẠO ĐỐI TƯỢNG =================
DHT dht(DHTPIN, DHTTYPE);

// Khởi tạo màn hình OLED SH1106 dùng thư viện U8g2 của oliver
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ================= BIẾN QUẢN LÝ THỜI GIAN =================
unsigned long previousMillisLED = 0;
unsigned long previousMillisDHT = 0;
int ledState = HIGH;    // Trạng thái ban đầu của LED

void setup() {
  Serial.begin(115200);
 
  // 1. Cấu hình đèn LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, ledState);

  // 2. Khởi tạo cảm biến nhiệt độ
  dht.begin();

  // 3. Khởi tạo màn hình OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.enableUTF8Print(); // Bật chế độ in tiếng Việt có dấu
}

void loop() {
  unsigned long currentMillis = millis();

  // --- YÊU CẦU 1: ĐIỀU KHIỂN ĐÈN NHẤP NHÁY ---
  // Cứ mỗi 1000ms (1 giây) đèn sẽ đảo trạng thái (Sáng <-> Tắt)
  if (currentMillis - previousMillisLED >= 1000) {
    previousMillisLED = currentMillis;
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(LED_BUILTIN, ledState);
  }

  // --- YÊU CẦU 2, 3 & 4: ĐỌC NHIỆT ĐỘ & HIỂN THỊ OLED ---
  // Cứ mỗi 2000ms (2 giây) sẽ cập nhật màn hình 1 lần
  if (currentMillis - previousMillisDHT >= 2000) {
    previousMillisDHT = currentMillis;

    // Đọc thông tin nhiệt độ
    float temperature = dht.readTemperature();

    // Chuẩn bị màn hình
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese1); // Font tiếng Việt

    // >>> THÊM THÔNG TIN NHÓM & SINH VIÊN <<<
    oled.setCursor(0, 15);
    oled.print("Nhóm 1:");  
   
    oled.setCursor(0, 32);
    oled.print("Đoàn Đức Kiệt");    

    // Kẻ một đường ngang phân cách cho đẹp mắt
    oled.drawLine(0, 40, 128, 40);

    // Hiển thị thông tin nhiệt độ
    oled.setCursor(0, 58);
    if (isnan(temperature)) {
      oled.print("Lỗi đọc DHT!");
      Serial.println("Lỗi: Không tìm thấy cảm biến DHT!");
    } else {
      oled.print("Nhiệt độ: ");
      oled.print(temperature, 1); // In nhiệt độ với 1 số lẻ (VD: 25.5)
      oled.print(" °C");
     
    }

    // Đẩy tất cả dữ liệu ra màn hình OLED
    oled.sendBuffer();
  }
}
