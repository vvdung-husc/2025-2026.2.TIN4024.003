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
/* ================== HÀM LOOP ================== */
void loop() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ Loi doc DHT22");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("DHT22 ERROR!");
    display.display();
    delay(2000);
    return;
  }

  /* ======= XÁC ĐỊNH TRẠNG THÁI ======= */
  String statusText = "";
  int ledColor = 0;

  if (temperature < 13) {
    statusText = "TOO COLD";
    ledColor = 1;
  }
  else if (temperature < 20) {
    statusText = "COLD";
    ledColor = 1;
  }
  else if (temperature < 25) {
    statusText = "COOL";
    ledColor = 2;
  }
  else if (temperature < 30) {
    statusText = "WARM";
    ledColor = 2;
  }
  else if (temperature < 35) {
    statusText = "HOT";
    ledColor = 3;
  }
  else {
    statusText = "TOO HOT";
    ledColor = 3;
  }

  /* ======= SERIAL ======= */
  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" C | Do am: ");
  Serial.print(humidity);
  Serial.println(" %");

  /* ======= OLED ======= */
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("ESP32 - DHT22");

  display.setTextSize(2);
  display.setCursor(0,14);
  display.print(temperature,1);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(0,36);
  display.print("Humidity: ");
  display.print(humidity,1);
  display.print("%");

  display.setTextSize(2);
  display.setCursor(0,48);
  display.println(statusText);

  display.display();

  /* ======= LED BLINK ======= */
  for(int i=0;i<5;i++)
  {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_BLUE, LOW);

    if(ledColor==1) digitalWrite(LED_BLUE, HIGH);
    if(ledColor==2) digitalWrite(LED_YELLOW, HIGH);
    if(ledColor==3) digitalWrite(LED_RED, HIGH);

    delay(300);

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_BLUE, LOW);

    delay(300);
  }
  delay(2000);

}
