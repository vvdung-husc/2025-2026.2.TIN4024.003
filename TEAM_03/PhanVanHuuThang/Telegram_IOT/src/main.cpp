#include <Arduino.h>  
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WIFI
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// TELEGRAM
#define BOT_TOKEN "8729067566:AAF92vtaNlMUTBILmI9R2AVH0PC4TX2A6FE"
#define CHAT_ID "5916461003"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// PIN
#define PIR_PIN 27
#define LED_PIN 23

bool ledState = false;
bool motionState = false;

unsigned long lastTimeBotRan;
int botRequestDelay = 1000;

// xử lý tin nhắn telegram
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.println(text);

    if (text == "/start") {
      String welcome = "ESP32 Telegram Control\n";
      welcome += "/on : Bat LED\n";
      welcome += "/off : Tat LED\n";
      welcome += "/status : Trang thai LED\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/on") {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      bot.sendMessage(chat_id, "LED da BAT", "");
    }

    if (text == "/off") {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      bot.sendMessage(chat_id, "LED da TAT", "");
    }

    if (text == "/status") {
      if (ledState) {
        bot.sendMessage(chat_id, "LED dang BAT", "");
      } else {
        bot.sendMessage(chat_id, "LED dang TAT", "");
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, pass);
  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  client.setInsecure(); // cho phép HTTPS
}

void loop() {

  // kiểm tra PIR
  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH && !motionState) {
    Serial.println("Motion detected!");

    bot.sendMessage(CHAT_ID, "Phat hien chuyen dong!", "");

    motionState = true;
  }

  if (motion == LOW) {
    motionState = false;
  }

  // đọc tin nhắn telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}