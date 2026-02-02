/*
Thông tin nhóm 10:
1. Đinh Tuấn Anh.
2. Lê Trần Hải Đạt.
3. Phan Thanh Vũ.

*/
#include <Arduino.h>
#include <DHTesp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== DHT =====
const int DHT_PIN = 16;   // ⚠ ĐÚNG theo sơ đồ mạch
DHTesp dhtSensor;

void showWelcome() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("WELCOME");
  display.display();
  delay(2000);  // Hiển thị 2 giây
}

void setup() {
  Serial.begin(115200);

  // Khởi tạo I2C cho OLED (theo mạch của bạn)
  Wire.begin(13, 12);  // SDA = 13, SCL = 12

  // Khởi tạo OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);
  }

  showWelcome();  // ⭐ HIỂN THỊ LÚC KHỞI ĐỘNG

  // Khởi tạo DHT
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
  Serial.println("---");

  delay(2000);
}