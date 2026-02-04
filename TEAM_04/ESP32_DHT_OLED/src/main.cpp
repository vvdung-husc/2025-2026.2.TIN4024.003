/*
THÔNG TIN NHÓM 4
1. Hồ Gia Long
2. Hồ Trọng Nghĩa
3. Phạm Văn Quân
4. Nguyễn Thắng
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

/* ================== KHAI BÁO CHÂN ================== */
// DHT22
#define DHTPIN 16
#define DHTTYPE DHT22

// LED
#define LED_RED    4
#define LED_YELLOW 2
#define LED_BLUE   15   // LED màu cyan

// OLED I2C
#define OLED_SDA 13
#define OLED_SCL 12
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/* ================== KHỞI TẠO THIẾT BỊ ================== */
DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

/* ================== HÀM SETUP ================== */
void setup() {
  Serial.begin(115200);
  delay(500);

  // Khởi tạo LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE, LOW);

  // Khởi tạo DHT22
  dht.begin();

  // Khởi tạo I2C cho OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // Khởi tạo OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ Không tìm thấy OLED SSD1306");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ESP32 DHT22 OLED");
  display.println("System Ready...");
  display.display();

  Serial.println("✅ System initialized");
}

/* ================== HÀM LOOP ================== */
void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius

  // Kiểm tra lỗi DHT
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ Loi doc DHT22");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("DHT22 ERROR!");
    display.display();

    delay(2000);
    return;
  }

  /* ======= IN SERIAL ======= */
  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" *C | Do am: ");
  Serial.print(humidity);
  Serial.println(" %");

  /* ======= HIỂN THỊ OLED ======= */
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("ESP32 - DHT22");

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(temperature, 1);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(0, 42);
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.print(" %");

  display.display();

  /* ======= LOGIC LED ======= */
  /*
    - LED XANH: Nhiệt độ < 25°C
    - LED VÀNG: 25°C – 30°C
    - LED ĐỎ : > 30°C
  */

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE, LOW);

  if (temperature < 25) {
    digitalWrite(LED_BLUE, HIGH);
  }
  else if (temperature < 30) {
    digitalWrite(LED_YELLOW, HIGH);
  }
  else {
    digitalWrite(LED_RED, HIGH);
  }

  delay(2000); // đọc mỗi 2 giây
}
