#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>

// --- Cấu hình OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Cấu hình Telegram Bot ---
#include "secrets.h"

// --- Cấu hình Kết nối ---
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- Pin & Tham số ---
const int tankHeight = 100;
const int ledRed = 18, ledYellow = 19, ledGreen = 17, buzzer = 5;
const int TRIG_PIN = 13, ECHO_PIN = 12;

// --- Biến trạng thái ---
unsigned long lastBuzzerMillis = 0;
bool buzzerState = false;

int uploadCount = 0;
bool lowLevelAlertSent = false;

// -------------------------
// 1. HÀM CẢNH BÁO LOA
// -------------------------
void handleBuzzerAlert(bool isActive) {
  if (isActive) {
    unsigned long currentMillis = millis();
    // Cứ mỗi 300ms thì đảo trạng thái loa một lần
    if (currentMillis - lastBuzzerMillis >= 300) {
      lastBuzzerMillis = currentMillis;
      buzzerState = !buzzerState; // Đảo trạng thái (Tắt -> Bật -> Tắt)
      
      if (buzzerState) {
        tone(buzzer, 1000);
      } else {
        noTone(buzzer);
      }
    }
  } else {
    noTone(buzzer);
    buzzerState = false;
  }
}

// -------------------------
// 2. HÀM ĐỌC CẢM BIẾN
// -------------------------
int getWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = (duration * 0.034 / 2) + 0.5;
  
  // Chống trường hợp đo được khoảng cách lớn hơn chiều cao bồn
  if (distance > tankHeight) distance = tankHeight;
  int actualLevel = tankHeight - distance;
  return (actualLevel < 0) ? 0 : actualLevel;
}

// -------------------------
// 3. HÀM HIỂN THỊ OLED
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
  display.printf("%d cm", waterLevel);
  
  display.setCursor(0, 42);
  display.printf("%d%% %s", percent, status.c_str());

  if (alertLow) {
    display.setTextSize(1);
    display.setCursor(95, 0);
    display.print("WARN!");
  }
  display.display();
}

// -------------------------
// 4. HÀM KẾT NỐI WIFI
// ------------------------- 
void connectWiFi() {
  Serial.print("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

// -------------------------
// 5. HÀM GỬI CẢNH BÁO TELEGRAM
// -------------------------
void sendTelegramAlert(int waterLevel, float percentFloat, String status) {
  String msg = "🚨 *CẢNH BÁO MỨC NƯỚC THẤP!* 🚨\n\n";
  msg += "📟 Thiết bị: ESP32\n";
  msg += "💧 Mực nước: " + String(waterLevel) + " cm\n";
  msg += "📊 Phần trăm: " + String(percentFloat, 1) + "%\n";
  msg += "⚠️ Trạng thái: " + status;
  bot.sendMessage(CHAT_ID, msg, "Markdown"); // Dùng Markdown để in đậm
}

void setup() {
  Serial.begin(115200);
  pinMode(ledRed, OUTPUT); pinMode(ledYellow, OUTPUT); 
  pinMode(ledGreen, OUTPUT); pinMode(buzzer, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  
  connectWiFi();
  client.setInsecure();
}

void loop() {
  int waterLevel = getWaterLevel();
  float percentFloat = (waterLevel * 100.0) / tankHeight;
  int percent = (int)percentFloat;
  
  String status;
  bool isCritical = false;

  // Tắt tất cả LED trước khi cập nhật
  digitalWrite(ledRed, LOW); digitalWrite(ledYellow, LOW); digitalWrite(ledGreen, LOW);

  // --- Logic Phân Cấp ---
  if (waterLevel < 20) {
    status = "LOW";
    digitalWrite(ledRed, HIGH);
    isCritical = true;
  } else if (percent < 40) {
    status = "MED";
    digitalWrite(ledYellow, HIGH);
  } else {
    status = "HIGH";
    digitalWrite(ledGreen, HIGH);
  }

  // Cập nhật các hàm chức năng
  showOLED(waterLevel, percent, status, isCritical);
  handleBuzzerAlert(isCritical);

  // --- Logic Gửi Telegram (Chống spam) ---
  if (isCritical && !lowLevelAlertSent) {
    sendTelegramAlert(waterLevel, percentFloat, status);
    lowLevelAlertSent = true;
  } else if (waterLevel >= 25) { 
    // Chỉ reset trạng thái gửi tin khi nước dâng lên hẳn 25cm
    lowLevelAlertSent = false;
  }

  Serial.printf("Water: %d cm | Status: %s\n", waterLevel, status.c_str());
  // Thay vì delay(2000), ta chia nhỏ ra để loa bíp bíp vẫn hoạt động
  for (int i = 0; i < 200; i++) { 
    handleBuzzerAlert(isCritical); // Liên tục cập nhật trạng thái loa
    delay(10); // Tổng cộng vẫn là 200 * 10 = 2000ms (2 giây)
  }
}