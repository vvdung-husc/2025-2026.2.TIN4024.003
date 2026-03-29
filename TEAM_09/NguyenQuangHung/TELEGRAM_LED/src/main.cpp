#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram
#define BOTtoken "8627766447:AAGNDjia4TwqvJGMV3RUz_Nfv7A0Tmz8tD8"
#define CHAT_ID "-5270792686"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// LED pin
#define LED 2
void handleMessages(int numNewMessages);
unsigned long lastTime = 0;
const unsigned long delayTime = 1000;

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");

  client.setInsecure(); // bỏ SSL check

  bot.sendMessage(CHAT_ID, 
    "Xin chào!\n"
    "/led_on\n"
    "/led_off\n"
    "/get_state", 
    ""
  );
}

void loop() {
  if (millis() - lastTime > delayTime) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTime = millis();
  }
}

void handleMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      digitalWrite(LED, HIGH);
      bot.sendMessage(CHAT_ID, "LED bật", "");
    }

    else if (text == "/led_off") {
      digitalWrite(LED, LOW);
      bot.sendMessage(CHAT_ID, "LED tắt", "");
    }

    else if (text == "/get_state") {
      if (digitalRead(LED)) {
        bot.sendMessage(CHAT_ID, "LED is ON", "");
      } else {
        bot.sendMessage(CHAT_ID, "LED is OFF", "");
      }
    }
  }
}