#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>
#include "secrets.h"
#include <BlynkSimpleEsp32.h>

// =========================
// 1. CẤU HÌNH OLED
// =========================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// =========================
// 2. CẤU HÌNH KẾT NỐI
// =========================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
BlynkTimer timer;

// =========================
// 3. CẤU HÌNH PHẦN CỨNG
// =========================
const int tankHeight = 100;  // chiều cao bể (cm)

const int ledRed = 18;
const int ledYellow = 19;
const int ledGreen = 17;
const int buzzer = 5;

const int TRIG_PIN = 13;
const int ECHO_PIN = 12;

// =========================
// 4. BIẾN TRẠNG THÁI
// =========================
unsigned long lastBuzzerMillis = 0;
bool buzzerState = false;
bool lowLevelAlertSent = false;
bool currentCriticalState = false;

// =========================
// 5. HÀM CẢNH BÁO BUZZER
// =========================
void handleBuzzerAlert(bool isActive) {
  if (isActive) {
    unsigned long currentMillis = millis();

    if (currentMillis - lastBuzzerMillis >= 300) {
      lastBuzzerMillis = currentMillis;
      buzzerState = !buzzerState;

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

// =========================
// 6. HÀM ĐỌC MỰC NƯỚC
// =========================
int getWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  if (duration == 0) return 0;

  int distance = (duration * 0.034 / 2) + 0.5;

  if (distance > tankHeight) distance = tankHeight;

  int actualLevel = tankHeight - distance;
  return (actualLevel < 0) ? 0 : actualLevel;
}

// =========================
// 7. HÀM HIỂN THỊ OLED
// =========================
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

// =========================
// 8. HÀM GỬI TELEGRAM
// =========================
void sendTelegramAlert(int waterLevel, float percentFloat, String status) {
  String msg = "🚨 *CẢNH BÁO MỨC NƯỚC THẤP!* 🚨\n\n";
  msg += "📟 Thiết bị: ESP32\n";
  msg += "💧 Mực nước: " + String(waterLevel) + " cm\n";
  msg += "📊 Phần trăm: " + String(percentFloat, 1) + "%\n";
  msg += "⚠️ Trạng thái: " + status;

  bot.sendMessage(CHAT_ID, msg, "Markdown");
}

// =========================
// 9. HÀM CẬP NHẬT TOÀN HỆ THỐNG
// =========================
void updateSystem() {
  int waterLevel = getWaterLevel();
  float percentFloat = (waterLevel * 100.0) / tankHeight;
  int percent = (int)percentFloat;

  String status;
  bool isCritical = false;

  // Tắt tất cả LED trước khi cập nhật
  digitalWrite(ledRed, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen, LOW);

  // Phân loại mức nước
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

  currentCriticalState = isCritical;

  // Hiển thị OLED
  showOLED(waterLevel, percent, status, isCritical);

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V0, waterLevel);
  Blynk.virtualWrite(V1, percent);
  Blynk.virtualWrite(V2, status);

  // Gửi cảnh báo khi mực nước thấp
  if (isCritical && !lowLevelAlertSent) {
    sendTelegramAlert(waterLevel, percentFloat, status);
    Blynk.logEvent("water_low", String("Muc nuoc thap: ") + waterLevel + " cm");
    lowLevelAlertSent = true;
  } else if (waterLevel >= 25) {
    // reset cảnh báo khi nước lên lại đủ an toàn
    lowLevelAlertSent = false;
  }

  Serial.printf("Water: %d cm | %d%% | Status: %s\n",
                waterLevel, percent, status.c_str());
}

// =========================
// 10. SETUP
// =========================
void setup() {
  Serial.begin(115200);

  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED init failed");
    while (true);
  }

  client.setInsecure(); // dùng cho Telegram trên Wokwi/test nhanh

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Cập nhật hệ thống mỗi 2 giây
  timer.setInterval(2000L, updateSystem);
}

// =========================
// 11. LOOP
// =========================
void loop() {
  Blynk.run();
  timer.run();
  handleBuzzerAlert(currentCriticalState);
}