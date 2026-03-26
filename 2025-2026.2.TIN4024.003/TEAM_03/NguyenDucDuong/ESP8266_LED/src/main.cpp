#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

// Cấu hình chân kết nối
#define LED_PIN LED_BUILTIN  // LED built-in trên ESP8266 (GPIO2 = D4)
#define DHT_PIN D3           // Chân dữ liệu của cảm biến DHT (GPIO0 = D3)
#define DHT_TYPE DHT22       // Cảm biến trên PCB là DHT22 (AM2302)

// Khởi tạo DHT sensor
DHT dht(DHT_PIN, DHT_TYPE);

// Khởi tạo OLED SH1106 sử dụng I2C
// SCL -> D1 (GPIO5), SDA -> D2 (GPIO4)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Biến lưu trữ
float temperature = 0;
float humidity = 0;
unsigned long previousMillis = 0;
unsigned long ledMillis = 0;
const long interval = 2000;  // Đọc cảm biến mỗi 2 giây
bool ledState = false;

void setup() {
  // Khởi tạo Serial Monitor
  Serial.begin(115200);
  Serial.println("\n=== ESP8266 Nguyen Duc Duong ===");
  Serial.println("LED + DHT11 + OLED SH1106");
  
  // Khởi tạo LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Tắt LED (LOW = ON trên ESP8266)
  
  // Khởi tạo DHT sensor
  dht.begin();
  Serial.println("DHT sensor initialized");
  
  // Khởi tạo OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 15, "ESP8266 Ready!");
  u8g2.drawStr(0, 30, "Initializing...");
  u8g2.sendBuffer();
  
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Nhấp nháy LED (dùng timer riêng)
  if (currentMillis - ledMillis >= 500) {
    ledMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? LOW : HIGH); // LOW = ON
  }
  
  // Đọc cảm biến DHT mỗi 2 giây
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Đọc nhiệt độ và độ ẩm
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    
    // Kiểm tra dữ liệu hợp lệ
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      temperature = 0;
      humidity = 0;
    } else {
      // In ra Serial Monitor
      Serial.println("--- Reading Sensor ---");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
    }
    
    // Hiển thị lên OLED
    u8g2.clearBuffer();
    
    // Tiêu đề
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(15, 12, "ESP8266 Duong");
    
    // Vẽ đường kẻ
    u8g2.drawLine(0, 15, 128, 15);
    
    // Hiển thị nhiệt độ
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5, 30, "Temp:");
    char tempStr[10];
    dtostrf(temperature, 4, 1, tempStr);
    strcat(tempStr, " C");
    u8g2.drawStr(50, 30, tempStr);
    
    // Hiển thị độ ẩm
    u8g2.drawStr(5, 45, "Humid:");
    char humStr[10];
    dtostrf(humidity, 4, 1, humStr);
    strcat(humStr, " %");
    u8g2.drawStr(50, 45, humStr);
    
    // Hiển thị trạng thái LED
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(5, 60, ledState ? "LED: ON" : "LED: OFF");
    
    u8g2.sendBuffer();
  }
}