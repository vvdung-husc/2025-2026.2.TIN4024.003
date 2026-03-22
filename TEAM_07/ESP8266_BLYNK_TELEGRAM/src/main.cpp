/*
  THÔNG TIN NHÓM 7
  1. Trần Hưng Trường Vủ
  2. Nguyễn Trọng Quý
  3. Trần Quang Tiến
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>

// WIFI
const char* ssid = "CNTT-MMT";
const char* password = "13572468";

// TELEGRAM
#define BOTtoken "8339212597:AAFFy_Wh8ayLi_m3vHcv_TZjklBveeJk5iQ"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// DHT
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LED
#define LED_PIN D4

float oldTemp = -100;
float oldHum = -100;

unsigned long bot_lasttime;
const unsigned long BOT_MTBS = 1000;

// xử lý lệnh telegram
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;   // LẤY CHAT ID TỪ TIN NHẮN

    if (text == "/start") {

      String welcome = "ESP8266 Control\n";
      welcome += "/led_on : Bat den\n";
      welcome += "/led_off : Tat den\n";
      welcome += "/status : Trang thai den";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "LED is ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }

    if (text == "/status") {

      if (digitalRead(LED_PIN) == LOW) {
        bot.sendMessage(chat_id, "LED dang BAT", "");
      } else {
        bot.sendMessage(chat_id, "LED dang TAT", "");
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  WiFi.begin(ssid, password);

  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  client.setInsecure();

  dht.begin();
}

void loop() {

  if (millis() > bot_lasttime + BOT_MTBS) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Loi doc DHT11");
    delay(2000);
    return;
  }

  if (temp != oldTemp || hum != oldHum) {

    String message = "Nhiet do: ";
    message += String(temp);
    message += " C\n";

    message += "Do am: ";
    message += String(hum);
    message += " %";

    bot.sendMessage(bot.messages[0].chat_id, message, "");

    oldTemp = temp;
    oldHum = hum;
  }

  delay(2000);
}