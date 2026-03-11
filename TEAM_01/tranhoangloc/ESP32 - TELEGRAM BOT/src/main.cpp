#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// WiFi for Wokwi simulation
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram Bot Token
#define BOT_TOKEN ""

// Group or Chat ID
#define CHAT_ID ""

// Pins
const int ledPin = 23;
const int pirPin = 27;

// Telegram objects
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Motion detection
bool motionState = false;

// Handle Telegram messages
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    Serial.println(text);

    if (text == "/start") {

      String welcome = "ESP32 Telegram Control\n";
      welcome += "/led_on : Turn LED ON\n";
      welcome += "/led_off : Turn LED OFF\n";
      welcome += "/status : Check PIR sensor\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED is ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }

    if (text == "/status") {

      int motion = digitalRead(pirPin);

      if (motion == HIGH) {
        bot.sendMessage(chat_id, "Motion detected!", "");
      } else {
        bot.sendMessage(chat_id, "No motion", "");
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");

  // Required for HTTPS
  client.setInsecure();

  bot.sendMessage(CHAT_ID, "ESP32 is online!", "");
}

void loop() {

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  // PIR motion detection
  int motion = digitalRead(pirPin);

  if (motion == HIGH && !motionState) {

    Serial.println("Motion detected");

    bot.sendMessage(CHAT_ID, "Motion detected by PIR sensor!", "");

    motionState = true;
  }

  if (motion == LOW) {
    motionState = false;
  }

  delay(1000);
}