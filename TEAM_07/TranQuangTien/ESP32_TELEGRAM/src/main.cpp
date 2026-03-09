#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi (Wokwi dùng mạng này)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Token bot từ BotFather
#define BOTtoken "8699608688:AAGrbSK0hPxQKOKEROCkAcJrH92M5GXGe2c"

// Telegram ID của bạn
#define CHAT_ID "8690982418"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan;
int bot_delay = 1000;

const int ledPin = 23;
bool ledState = LOW;

// xử lý tin nhắn mới
void handleNewMessages(int numNewMessages) {

  Serial.println("Handling new messages");

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println(text);

    if (text == "/start") {

      String welcome = "Welcome " + from_name + "\n";
      welcome += "Commands:\n";
      welcome += "/led_on - Turn LED ON\n";
      welcome += "/led_off - Turn LED OFF\n";
      welcome += "/state - Check LED state\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = HIGH;
      bot.sendMessage(chat_id, "LED is ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = LOW;
      bot.sendMessage(chat_id, "LED is OFF", "");
    }

    if (text == "/state") {
      if (digitalRead(ledPin)) {
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
  digitalWrite(ledPin, ledState);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setInsecure();   // tránh lỗi TLS

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void loop() {

  if (millis() > lastTimeBotRan + bot_delay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      Serial.println("Got message");

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}