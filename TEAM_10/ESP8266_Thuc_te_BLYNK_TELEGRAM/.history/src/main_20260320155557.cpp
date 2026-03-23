/*
	THÔNG TIN NHÓM 10
	1. Đinh Tuấn Anh
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
        "/gas",
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
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(gasDigital, INPUT);
  pinMode(ledPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(gasDigital), gasDetectedISR, RISING);

  // OLED dùng chân  (13,12)
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

  // ===== THỜI GIAN HOẠT ĐỘNG =====
  unsigned long seconds = millis() / 1000;
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int secs = seconds % 60;

  // ===== HIỂN THỊ OLED =====
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Dòng 1: Team
  display.setCursor(0, 0);
  display.println("TEAM 10");

  // Dòng 2: Time
  display.setCursor(0, 10);
  display.printf("Time: %02d:%02d:%02d", hours, minutes, secs);

  // Dòng 3: Temp
  display.setCursor(0, 20);
  display.printf("Temp: %.1f C", temp);

  // Dòng 4: Humidity
  display.setCursor(0, 30);
  display.printf("Humi: %.1f %%", hum);

  // Dòng 5: Gas
  display.setCursor(0, 40);
  display.printf("Gas: %d", gasValue);

  // Dòng 6: Status
  display.setCursor(0, 50);
  if (gasValue > 2000) {
    display.println("WARNING GAS!");
  } else {
    display.println("Normal");
  }

  display.display();

  // ===== PHÁT HIỆN GAS =====
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