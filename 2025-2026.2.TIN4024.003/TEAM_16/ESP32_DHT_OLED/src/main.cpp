/*
THÔNG TIN NHÓM 16
1. Lê Anh Quân
2. Phạm Ngọc Khanh
*/

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ================= DHT ================= */
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/* ================= OLED ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ================= LED ================= */
#define LED_GREEN   16
#define LED_YELLOW  17
#define LED_RED     18

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
}

/* ================= LOOP ================= */
void loop() {
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  if (isnan(temp) || isnan(humi)) {
    Serial.println("Failed to read DHT!");
    return;
  }

  // Tắt toàn bộ LED
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.print("Humi: ");
  display.print(humi);
  display.println(" %");

  display.println("----------------");

  // ===== Ngưỡng nhiệt độ =====
  if (temp < 13) {
    display.println("TOO COLD");
    digitalWrite(LED_GREEN, HIGH);
  } 
  else if (temp < 20) {
    display.println("COLD");
    digitalWrite(LED_GREEN, HIGH);
  } 
  else if (temp < 25) {
    display.println("COOL");
    digitalWrite(LED_YELLOW, HIGH);
  } 
  else if (temp < 30) {
    display.println("WARM");
    digitalWrite(LED_YELLOW, HIGH);
  } 
  else if (temp < 35) {
    display.println("HOT");
    digitalWrite(LED_RED, HIGH);
  } 
  else {
    display.println("TOO HOT");
    digitalWrite(LED_RED, HIGH);
  }

  display.display();

  // LED nhấp nháy
  delay(500);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  delay(500);
}

