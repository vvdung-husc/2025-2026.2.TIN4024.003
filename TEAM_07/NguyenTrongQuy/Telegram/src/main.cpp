#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// Bot Telegram
#define BOTtoken "8590092376:AAFfdnmEgMkZmKIiqcGKFvPBJiZZhetGQpw"
#define CHAT_ID "5394372469"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan;
int bot_delay = 1000;

const int ledPin = 23;   // LED ESP32
bool ledState = LOW;

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {

      String welcome = "Welcome " + from_name + "\n";
      welcome += "/led2_on : Bat LED\n";
      welcome += "/led2_off : Tat LED\n";
      welcome += "/get_state : Trang thai LED\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led2_on") {
      digitalWrite(ledPin, HIGH);   // bật LED
      ledState = HIGH;
      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (text == "/led2_off") {
      digitalWrite(ledPin, LOW);   // tắt LED
      ledState = LOW;
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    if (text == "/get_state") {
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

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  client.setInsecure();

  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi da ket noi");
  Serial.println(WiFi.localIP());
}

void loop() {

  if (millis() - lastTimeBotRan > bot_delay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}