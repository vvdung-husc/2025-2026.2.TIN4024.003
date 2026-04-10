/*
  THÔNG TIN NHÓM 10 - CODE ESP8266 THỰC TẾ
  1: Đinh Tuấn Anh / Telegram: Tuấn Anh Đinh
  2: Phan Thanh Vũ / Telegram: Phan Thanh Vũ

*/

#define BLYNK_TEMPLATE_ID "TMPL6uZReKclM"
#define BLYNK_TEMPLATE_NAME "Nhom10iot"
#define BLYNK_AUTH_TOKEN "b0SAhSpKUXQk1oh2w5TvDhiZOvhH1IYf"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>
#include <U8g2lib.h>          // Thay Adafruit bằng U8g2
#include <DHT.h>

const char* ssid     = "CNTT-MMT";
const char* password = "13572468";

#define BOTtoken  "8333356314:AAHgJ3eQvr2rghJ-i9YwQWU0YcJiqhMnbxw"
#define GROUP_ID  "-5116051271"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
BlynkTimer timer;


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);

#define DHTPIN 0               // GPIO0 = D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MQ2 Gas
const int gasDigital = 16;     // GPIO16 = D0
const int gasAnalog  = A0;

// LED Blink
const int ledPin     = 12;     // GPIO12 = D6

bool gasDetected = false;
bool blinkMode = false;
bool ledState = false;
unsigned long previousMillis = 0;
const long blinkInterval = 200;
unsigned long lastTimeBotRan;
const int botRequestDelay = 3000;

float lastTemp = -100.0f, lastHum = -100.0f;

// Interrupt
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
      bot.sendMessage(chat_id, "TEAM 10 SYSTEM\n/led_on\n/led_off\n/gas\n/get_weather\n/led_status", "");
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
      if (blinkMode) statusMsg += "ĐANG BẬT (BLINK)";
      else statusMsg += "ĐANG TẮT";
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

  // Khởi tạo OLED U8g2
  Serial.println(F("Khoi dong OLED U8g2..."));
  u8g2.begin();
  
  // Test OLED ngay lập tức
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);  
  u8g2.drawStr(10, 30, "TEAM 10");
  u8g2.drawStr(10, 50, "OLED da len!");
  u8g2.sendBuffer();
  delay(4000);

  Serial.println(F("OLED test hoan tat"));

  dht.begin();

  WiFi.begin(ssid, password);
  client.setInsecure();

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());

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
      lastTemp = t;
      lastHum = h;
    }
  }

  // Cập nhật OLED mỗi 500ms với U8g2
  static unsigned long lastOled = 0;
  if (millis() - lastOled > 500) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr); 
    u8g2.setCursor(0, 10); u8g2.print("TEAM 10 - IOT");
    u8g2.setCursor(0, 25); u8g2.print("Temp: "); u8g2.print(t, 1); u8g2.print(" C");
    u8g2.setCursor(0, 38); u8g2.print("Humi: "); u8g2.print(h, 1); u8g2.print(" %");
    u8g2.setCursor(0, 51); u8g2.print("Gas : "); u8g2.print(g);
    if (g > 2000) {
      u8g2.setCursor(0, 64);
      u8g2.print("!! WARNING GAS !!");
    }
    u8g2.sendBuffer();
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