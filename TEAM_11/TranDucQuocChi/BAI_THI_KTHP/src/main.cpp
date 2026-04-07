#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>
#include "secrets.h"

// -------------------------
// THÔNG TIN MÀN HÌNH OLED
// -------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -------------------------
// THÔNG TIN KẾT NỐI
// -------------------------
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// -------------------------
// THÔNG TIN HỆ THỐNG
// -------------------------
const int tankHeight = 100;
const int ledRed = 18;
const int ledYellow = 19;
const int ledGreen = 17;
const int buzzer = 5;

// Chân cảm biến siêu âm
const int TRIG_PIN = 13;
const int ECHO_PIN = 12;

int uploadCount = 0;
bool lowLevelAlertSent = false;

// -------------------------
// HÀM ĐỌC CẢM BIẾN SIÊU ÂM
// -------------------------
int getWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;
  if (distance > tankHeight) distance = tankHeight;
  int actualLevel = tankHeight - distance;
  return (actualLevel < 0) ? 0 : actualLevel;
}

// -------------------------
// HÀM HIỂN THỊ OLED
// -------------------------
void showOLED(int waterLevel, int percent, String status, bool alertLow) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SMART WATER LEVEL");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(waterLevel);
  display.print(" cm");
  display.setTextSize(2);
  display.setCursor(0, 42);
  display.print(percent);
  display.print("% ");
  display.print(status);
  if (alertLow) {
    display.setTextSize(1);
    display.setCursor(92, 0);
    display.print("ALERT");
  }
  display.display();
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// -------------------------
// HÀM GỬI TELEGRAM
// -------------------------
void sendTelegramAlert(int waterLevel, float percentFloat, String status) {
  String msg = "🚨 CẢNH BÁO MỨC NƯỚC THẤP! 🚨\n";
  msg += "📟 Thiết bị: ESP32\n";
  msg += "💧 Mức nước: " + String(waterLevel) + " cm\n";
  msg += "📊 Phần trăm: " + String(percentFloat, 2) + "%\n";
  msg += "⚠️ Trạng thái: " + status + "\n";
  msg += "📉 Ngưỡng cảnh báo: dưới 20 cm";
  bot.sendMessage(CHAT_ID, msg, "");
}

void setup() {
  Serial.begin(115200);

  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  
  connectWiFi();
  client.setInsecure();
  delay(1500);
}

void loop() {
  // Lấy dữ liệu từ cảm biến siêu âm 
  int waterLevel = getWaterLevel(); 
  // ------------------------------------

  float percentFloat = (waterLevel * 100.0) / tankHeight;
  int percent = (int)percentFloat;
  String status;
  bool alertLow = false;

  digitalWrite(ledRed, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen, LOW);
  noTone(buzzer);

  // Logic xử lý
  if (waterLevel < 20) {
    status = "LOW";
    digitalWrite(ledRed, HIGH);
    tone(buzzer, 1000);
    alertLow = true;
  } else if (percent < 30) {
    status = "LOW";
    digitalWrite(ledRed, HIGH);
  } else if (percent < 70) {
    status = "MED";
    digitalWrite(ledYellow, HIGH);
  } else {
    status = "HIGH";
    digitalWrite(ledGreen, HIGH);
  }

  uploadCount++;
  showOLED(waterLevel, percent, status, alertLow);

  if (waterLevel < 20 && !lowLevelAlertSent) {
    sendTelegramAlert(waterLevel, percentFloat, status);
    lowLevelAlertSent = true;
  }
  if (waterLevel >= 20) {
    lowLevelAlertSent = false;
  }

  // Serial Debug
  Serial.print("Water Level: "); Serial.println(waterLevel);

  delay(2000);
  noTone(buzzer);
}