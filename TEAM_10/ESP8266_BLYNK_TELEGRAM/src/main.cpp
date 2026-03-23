/*
  THÔNG TIN NHÓM 10
  1. Đinh Tuấn Anh - telegram : Tuấn Anh Đinh
  2. Phan Thanh Vũ
  3. Lê Trần Hải Đạt
  4. Đặng Văn Phi
  
*/
#define BLYNK_TEMPLATE_ID "TMPL6uZReKclM"
#define BLYNK_TEMPLATE_NAME "Nhom10iot"
#define BLYNK_AUTH_TOKEN "b0SAhSpKUXQk1oh2w5TvDhiZOvhH1IYf"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
#define BOTtoken "8333356314:AAHgJ3eQvr2rghJ-i9YwQWU0YcJiqhMnbxw"
#define GROUP_ID "-5116051271"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
BlynkTimer timer;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int gasDigital = 25;
const int gasAnalog  = 34;
const int ledPin     = 5;

bool gasDetected = false;
bool blinkMode = false;
bool ledState = false;
unsigned long previousMillis = 0;
const long blinkInterval = 200;
unsigned long lastTimeBotRan;
const int botRequestDelay = 3000; 

float lastTemp = -100, lastHum = -100;

void IRAM_ATTR gasDetectedISR() {
  gasDetected = true;
}

void sendSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int g = analogRead(gasAnalog);

  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V11, t);
    Blynk.virtualWrite(V10, h);
  }
  Blynk.virtualWrite(V12, g);
  Blynk.virtualWrite(V6, "Nhom 10 - Bai Thi IoT");
  Blynk.virtualWrite(V7, millis() / 1000);
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/start") {
      bot.sendMessage(chat_id, "TEAM 10 SYSTEM\n/led_on\n/led_off\n/gas\n/get_weather\n/get_status", "");
    }
    else if (text == "/led_on") {
      blinkMode = true;
      Blynk.virtualWrite(V1, 1); 
      bot.sendMessage(chat_id, "LED ON", "");
    }
    else if (text == "/led_off") {
      blinkMode = false;
      digitalWrite(ledPin, LOW);
      Blynk.virtualWrite(V1, 0);
      bot.sendMessage(chat_id, "LED OFF", "");
    }
    else if (text == "/gas") {
      bot.sendMessage(chat_id, "Giá trị Gas hiện tại: " + String(analogRead(gasAnalog)), "");
    }
    else if (text == "/get_weather") {
      bot.sendMessage(chat_id, "Nhiệt độ: " + String(dht.readTemperature()) + "°C\nĐộ ẩm: " + String(dht.readHumidity()) + "%", "");
    }
    else if (text == "/led_status") {
      String statusMsg = "Trạng thái LED: ";
      
      if (blinkMode) {
        statusMsg += "ĐANG BẬT (BLINK)";
      } else {
        statusMsg += "ĐANG TẮT";
      }

      bot.sendMessage(chat_id, statusMsg, "");
    }
  }
}

BLYNK_WRITE(V1) {
  blinkMode = param.asInt();
  if (!blinkMode) digitalWrite(ledPin, LOW);
  Serial.println(blinkMode ? "Blynk: LED Blink ON" : "Blynk: LED OFF");
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}

void setup() {
  Serial.begin(115200);
  pinMode(gasDigital, INPUT);
  pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(gasDigital), gasDetectedISR, RISING);

  Wire.begin(13, 12); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED lỗi!");
    while(true);
  }

  dht.begin();
  WiFi.begin(ssid, password);
  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(2000L, sendSensorData);
  
  bot.sendMessage(GROUP_ID, "Hệ thống Nhóm 10 đã trực tuyến!", "");
}

void loop() {
  Blynk.run();
  timer.run();

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int g = analogRead(gasAnalog);

  if (gasDetected) {
    bot.sendMessage(GROUP_ID, "CẢNH BÁO: PHÁT HIỆN KHÍ GAS!\nGiá trị: " + String(g));
    blinkMode = true;
    Blynk.virtualWrite(V1, 1); 
    gasDetected = false;
  }

  if (!isnan(t) && !isnan(h)) {
    if (abs(t - lastTemp) >= 1.0 || abs(h - lastHum) >= 2.0) {
      bot.sendMessage(GROUP_ID, "Cập nhật thời tiết:\n " + String(t) + "°C | " + String(h) + "%");
      lastTemp = t; lastHum = h;
    }
  }

  static unsigned long lastOled = 0;
  if (millis() - lastOled > 500) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0); display.println("TEAM 10 - IOT");
    display.setCursor(0, 15); display.printf("Temp: %.1f C", t);
    display.setCursor(0, 25); display.printf("Humi: %.1f %%", h);
    display.setCursor(0, 35); display.printf("Gas : %d", g);
    if (g > 2000) {
      display.setCursor(0, 50);
      display.println("!! WARNING GAS !!");
    }
    display.display();
    lastOled = millis();
  }

  if (blinkMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
    }
  }

  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}