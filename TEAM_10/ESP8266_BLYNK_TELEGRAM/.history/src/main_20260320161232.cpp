/*
  THÔNG TIN NHÓM 10
  1. Đinh Tuấn Anh
  2. Phan Thanh Vũ
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===== WIFI =====
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===== TELEGRAM =====
#define BOTtoken "8333356314:AAHgJ3eQvr2rghJ-i9YwQWU0YcJiqhMnbxw"
#define GROUP_ID "-5116051271"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== DHT =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== CHÂN =====
const int gasDigital = 25;
const int gasAnalog  = 34;
const int ledPin     = 5;

// ===== BIẾN =====
bool gasDetected = false;
bool blinkMode = false;
bool ledState = false;

unsigned long previousMillis = 0;
const long blinkInterval = 200;

unsigned long lastTimeBotRan;
const int botRequestDelay = 1000;

// ===== THEO DÕI THAY ĐỔI =====
float lastTemp = -100;
float lastHum  = -100;

// ===== INTERRUPT =====
void IRAM_ATTR gasDetectedISR() {
  gasDetected = true;
}

// ===== TELEGRAM =====
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/start") {
      bot.sendMessage(chat_id,
        "TEAM 10 SYSTEM\n"
        "/led_on\n"
        "/led_off\n"
        "/gas\n"
        "/get_weather",
        "");
    }

    if (text == "/led_on") {
      blinkMode = true;
      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (text == "/led_off") {
      blinkMode = false;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    if (text == "/gas") {
      int gasValue = analogRead(gasAnalog);
      bot.sendMessage(chat_id, "Gas: " + String(gasValue), "");
    }

    if (text == "/get_weather") {
      float temp = dht.readTemperature();
      float hum  = dht.readHumidity();

      String msg = "🌡 Temperature: " + String(temp) + " C\n";
      msg += "💧 Humidity: " + String(hum) + " %";

      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(gasDigital, INPUT);
  pinMode(ledPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(gasDigital), gasDetectedISR, RISING);

  // OLED dùng chân 13,12
  Wire.begin(13, 12);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED lỗi!");
    while(true);
  }

  display.clearDisplay();
  dht.begin();

  WiFi.begin(ssid, password);
  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  bot.sendMessage(GROUP_ID, "Bot đã online!", "");
}

// ===== LOOP =====
void loop() {

  // ===== ĐỌC DỮ LIỆU =====
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  int gasValue = analogRead(gasAnalog);

  // ===== GỬI KHI THAY ĐỔI =====
  if (!isnan(temp) && !isnan(hum)) {
    if (abs(temp - lastTemp) >= 0.5 || abs(hum - lastHum) >= 1) {

      String msg = "📢 WEATHER UPDATE\n";
      msg += "🌡 Temp: " + String(temp) + " C\n";
      msg += "💧 Humi: " + String(hum) + " %";

      bot.sendMessage(GROUP_ID, msg);

      lastTemp = temp;
      lastHum = hum;
    }
  }

  // ===== THỜI GIAN =====
  unsigned long seconds = millis() / 1000;
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int secs = seconds % 60;

  // ===== OLED =====
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("TEAM 10");

  display.setCursor(0, 10);
  display.printf("Time: %02d:%02d:%02d", hours, minutes, secs);

  display.setCursor(0, 20);
  display.printf("Temp: %.1f C", temp);

  display.setCursor(0, 30);
  display.printf("Humi: %.1f %%", hum);

  display.setCursor(0, 40);
  display.printf("Gas: %d", gasValue);

  display.setCursor(0, 50);
  if (gasValue > 2000) {
    display.println("WARNING GAS!");
  } else {
    display.println("Normal");
  }

  display.display();

  // ===== GAS ALERT =====
  if (gasDetected) {

    String msg = "⚠️ GAS DETECTED!\nValue: " + String(gasValue);
    bot.sendMessage(GROUP_ID, msg);

    gasDetected = false;
    blinkMode = true;
  }

  // ===== LED =====
  if (blinkMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
    }
  }

  // ===== TELEGRAM =====
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}