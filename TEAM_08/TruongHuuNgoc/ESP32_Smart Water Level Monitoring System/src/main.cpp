#define BLYNK_TEMPLATE_ID "TMPL6nC4yA98e"
#define BLYNK_TEMPLATE_NAME "ESP32 Smart Water Level Monitoring System"
#define BLYNK_AUTH_TOKEN "GU7kY_6n5IvjQjCV_gOZc95wQld0A-Qq"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// ===== Cấu hình WiFi =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== Cấu hình Telegram =====
#define BOT_TOKEN "8621124778:AAGkuf8ptUKbxWczkcp4zHpeqPIbhkmGijg"
#define CHAT_ID "-1003885166476"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== Chân cắm (Pins) =====
#define WATER_SENSOR 34
#define LED_GREEN 16
#define LED_YELLOW 17
#define LED_RED 18
#define BUTTON_PIN 33

bool systemEnabled = true;
int lastButtonState = HIGH;

// ===== Timer điều hướng =====
unsigned long previousMillis = 0;
const unsigned long interval = 2000;
unsigned long botLastTime = 0;
const unsigned long botInterval = 1000;

String waterState = "Normal";
String lastWaterState = "Normal";

void turnOffAllLEDs()
{
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V1, 0);
  Blynk.virtualWrite(V2, 0);
}

// Đồng bộ từ nút nhấn trên App Blynk
BLYNK_WRITE(V4)
{
  systemEnabled = param.asInt();
  if (!systemEnabled)
  {
    turnOffAllLEDs();
    bot.sendMessage(CHAT_ID, "🚫 Hệ thống đèn đã tắt qua Blynk", "");
  }
  else
  {
    bot.sendMessage(CHAT_ID, "✅ Hệ thống đèn đã bật qua Blynk", "");
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.begin(ssid, pass);
  client.setInsecure(); // Cần thiết cho Telegram trên ESP32

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  bot.sendMessage(CHAT_ID, "🚀 Hệ thống cảnh báo ngập đã khởi động! /start", "");
}

void handleTelegram()
{
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages)
  {
    for (int i = 0; i < numNewMessages; i++)
    {
      String text = bot.messages[i].text;
      if (text == "/start")
      {
        bot.sendMessage(CHAT_ID, "Lệnh: /led_on, /led_off, /water_level, /status", "");
      }
      if (text == "/led_off")
      {
        systemEnabled = false;
        Blynk.virtualWrite(V4, 0);
        turnOffAllLEDs();
        bot.sendMessage(CHAT_ID, "Đã tắt đèn.", "");
      }
      if (text == "/led_on")
      {
        systemEnabled = true;
        Blynk.virtualWrite(V4, 1);
        bot.sendMessage(CHAT_ID, "Đã bật đèn.", "");
      }
      if (text == "/water_level")
      {
        int level = analogRead(WATER_SENSOR);
        bot.sendMessage(CHAT_ID, "Mực nước hiện tại: " + String(level) + " (" + waterState + ")", "");
      }

      if (text == "/status")
      {
        bot.sendMessage(CHAT_ID, "LED: " + String(systemEnabled ? "Đang hoạt động" : "Đã tắt"), "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void loop()
{
  Blynk.run();

  // 1. Kiểm tra nút nhấn vật lý (Wokwi)
  int currentButtonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW)
  { // Nhấn nút
    systemEnabled = !systemEnabled;
    Blynk.virtualWrite(V4, systemEnabled); // Cập nhật trạng thái lên Blynk
    if (!systemEnabled)
      turnOffAllLEDs();
    bot.sendMessage(CHAT_ID, systemEnabled ? "🔔 Đã bật đèn bằng nút nhấn" : "🔕 Đã tắt đèn bằng nút nhấn", "");
    delay(200); // Chống dội nút nhấn
  }
  lastButtonState = currentButtonState;

  unsigned long currentMillis = millis();

  // 2. Logic đọc cảm biến và điều khiển LED (mỗi 2s)
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    int waterLevel = analogRead(WATER_SENSOR);
    Blynk.virtualWrite(V0, waterLevel); // Gửi giá trị lên Widget Gauge/Chart trên Blynk

    if (systemEnabled)
    {
      if (waterLevel < 1500)
      {
        waterState = "Bình thường";
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);

        Blynk.virtualWrite(V3, 1);
        Blynk.virtualWrite(V1, 0);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(V5, waterState);
      }
      else if (waterLevel < 3000)
      {
        waterState = "Cảnh báo";
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_RED, LOW);

        Blynk.virtualWrite(V3, 0);
        Blynk.virtualWrite(V1, 1);
        Blynk.virtualWrite(V2, 0);
        Blynk.virtualWrite(V5, waterState);
      }
      else
      {
        waterState = "Nguy hiểm";
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, HIGH);

        Blynk.virtualWrite(V3, 0);
        Blynk.virtualWrite(V1, 0);
        Blynk.virtualWrite(V2, 1);
        Blynk.virtualWrite(V5, waterState);
      }
    }

    // Gửi thông báo tự động khi đổi trạng thái mức nước
    if (waterState != lastWaterState)
    {
      bot.sendMessage(CHAT_ID, "⚠️ Thay đổi mức nước: " + String(analogRead(WATER_SENSOR)) + " (" + waterState + ")", "");
      Blynk.logEvent("water_alert", "Mức nước hiện tại: " + waterState); // Nếu bạn có setup Event trên Blynk
      lastWaterState = waterState;
    }
  }

  // 3. Kiểm tra Telegram (mỗi 1s)
  if (millis() - botLastTime > botInterval)
  {
    handleTelegram();
    botLastTime = millis();
  }
}