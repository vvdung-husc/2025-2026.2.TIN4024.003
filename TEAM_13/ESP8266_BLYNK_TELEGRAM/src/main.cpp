
/*
 THÔNG TIN NHÓM 13
 1. Thân Hoàng Phước Minh
 2.Nguyễn Tấn Nhân
 3. Nguyễn Thành Trung
*/

#define BLYNK_TEMPLATE_ID "TMPL6iGHfH-M1"
#define BLYNK_TEMPLATE_NAME "IoT Team 13"
#define BLYNK_AUTH_TOKEN "yVi_5-_wYBsZxu3YOIu05e8bPJXfKszz"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <U8g2lib.h> 
#include <Wire.h>
#include <DHT.h>

#define RELAY_ON  LOW   
#define RELAY_OFF HIGH  

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "MinhHome"; 
char pass[] = "12345678";

#define BOTtoken "8701041356:AAHxrUqpLUk6Ez-26wPUSGnWAzgOuwYPapc"
#define GROUP_ID "-5002231155"

#define LED_PIN 2         
#define DHTPIN 0          // Chân D3 trên NodeMCU
#define DHTTYPE DHT22
#define MQ2_PIN A0 

U8G2_SH1106_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ D1, /* data=*/ D2, /* reset=*/ U8X8_PIN_NONE);

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan;
int botRequestDelay = 3000; 

float t, h;
int gasValue;
bool current_led_state = false;

void updateOLED() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "--- SYSTEM STATUS ---");
    u8g2.setCursor(0, 28);
    if (isnan(t)) u8g2.print("Nhiet do: DANG DOC...");
    else { u8g2.print("Nhiet do: "); u8g2.print(t, 1); u8g2.print(" C"); }
    u8g2.setCursor(0, 40);
    u8g2.print("Do am:    "); u8g2.print(h, 1); u8g2.print(" %");
    u8g2.setCursor(0, 52);
    u8g2.print("Khi Gas:  "); u8g2.print(gasValue);
    u8g2.setFont(u8g2_font_profont11_tr);
    u8g2.setCursor(0, 64);
    u8g2.print("LED: "); u8g2.print(current_led_state ? "ON " : "OFF");
    u8g2.drawStr(70, 64, "IoT Team 13");
  } while ( u8g2.nextPage() );
}

void updateSensors() {
  t = dht.readTemperature();
  h = dht.readHumidity();
  gasValue = analogRead(MQ2_PIN);
  if (!isnan(t)) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  }
  Blynk.virtualWrite(V3, gasValue);
  updateOLED(); 
}

// ===== CẬP NHẬT CÁC LỆNH TELEGRAM TẠI ĐÂY =====
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID) continue;
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      digitalWrite(LED_PIN, RELAY_ON);
      current_led_state = true;
      Blynk.virtualWrite(V0, 1);
      bot.sendMessage(chat_id, "✅ LED ĐÃ BẬT", "");
    } 
    else if (text == "/led_off") {
      digitalWrite(LED_PIN, RELAY_OFF);
      current_led_state = false;
      Blynk.virtualWrite(V0, 0);
      bot.sendMessage(chat_id, "❌ LED ĐÃ TẮT", "");
    }
    else if (text == "/led_state") {
      String stateMsg = current_led_state ? "Đèn đang BẬT 🟢" : "Đèn đang TẮT 🔴";
      bot.sendMessage(chat_id, "💡 Trạng thái hệ thống:\n" + stateMsg, "");
    }
    else if (text == "/get_weather") {
      String weatherMsg = "📊 THÔNG SỐ MÔI TRƯỜNG:\n";
      if (isnan(t)) {
        weatherMsg += "⚠️ Lỗi cảm biến DHT22!\n";
      } else {
        weatherMsg += "🌡 Nhiệt độ: " + String(t, 1) + " °C\n";
        weatherMsg += "💧 Độ ẩm: " + String(h, 1) + " %\n";
      }
      weatherMsg += "💨 Khí Gas: " + String(gasValue);
      bot.sendMessage(chat_id, weatherMsg, "");
    }
    else if (text == "/start") {
      String welcome = "Chào mừng Team 13!\nCác lệnh điều khiển:\n";
      welcome += "/led_on : Bật đèn\n";
      welcome += "/led_off : Tắt đèn\n";
      welcome += "/led_state : Kiểm tra đèn\n";
      welcome += "/get_weather : Xem cảm biến";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}

BLYNK_WRITE(V0) {
  int value = param.asInt();
  current_led_state = (value == 1);
  digitalWrite(LED_PIN, current_led_state ? RELAY_ON : RELAY_OFF);
  updateOLED();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, RELAY_OFF); 

  dht.begin();
  u8g2.begin();
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  Blynk.config(auth);
  client.setInsecure();
  client.setBufferSizes(512, 512); 
  
  delay(2000); 
  bot.sendMessage(GROUP_ID, "🚀 Team 13: Hệ thống đã sẵn sàng!\nGõ /start để xem các lệnh.", "");
}

void loop() {
  Blynk.run();
  if (millis() - lastTimeBotRan > botRequestDelay) {
    updateSensors();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}