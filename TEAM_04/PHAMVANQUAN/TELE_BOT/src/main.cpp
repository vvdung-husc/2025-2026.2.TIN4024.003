

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// THAY TOKEN MOI CUA BAN VAO DAY
#define BOTtoken "8287871719:AAE3mu_NGaaMyCvEDjr-o3tdvM5sJQotCOQ"

// CHAT ID NHOM CUA BAN
#define CHAT_ID "-5134488074"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan = 0;
const int botRequestDelay = 1000;

const int ledPin = 23;
const int pirPin = 27;

bool ledState = false;
int lastPirState = LOW;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println("Tin nhan moi:");
    Serial.println("chat_id: " + chat_id);
    Serial.println("text: " + text);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    if (text == "/start" || text == "/start@esp32_pvq_bot") {
      String welcome = "Xin chao, " + from_name + ".\n";
      welcome += "Su dung cac lenh sau de dieu khien den LED.\n\n";
      welcome += "Gui /led_on bat sang den\n";
      welcome += "Gui /led_off de tat den\n";
      welcome += "Gui /get_state de yeu cau trang thai den hien tai\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on" || text == "/led_on@esp32_pvq_bot") {
      digitalWrite(ledPin, HIGH);
      ledState = true;
      bot.sendMessage(chat_id, "LED bat sang", "");
    }

    if (text == "/led_off" || text == "/led_off@esp32_pvq_bot") {
      digitalWrite(ledPin, LOW);
      ledState = false;
      bot.sendMessage(chat_id, "LED is OFF", "");
    }

    if (text == "/get_state" || text == "/get_state@esp32_pvq_bot") {
      if (ledState) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi da ket noi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();

  bot.sendMessage(CHAT_ID, "ESP32 da ket noi thanh cong!", "");
}

void loop() {
  int pirState = digitalRead(pirPin);

  if (pirState == HIGH && lastPirState == LOW) {
    bot.sendMessage(CHAT_ID, "Phat hien chuyen dong!", "");
    lastPirState = HIGH;
  }

  if (pirState == LOW && lastPirState == HIGH) {
    lastPirState = LOW;
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