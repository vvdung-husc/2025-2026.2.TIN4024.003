#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ===== WiFi =====
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===== Telegram =====
#define BOT_TOKEN "8737324658:AAHt2JRgUZW6yLJkBcVmhe8q9kkW88K0FjA"
#define CHAT_ID "8652307048"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== DHT22 =====
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== Relay + LED =====
#define RELAY_PIN 23
#define LED_PIN 2

// ===== Cấu hình =====
float threshold = 25.0;
bool autoMode = true;   // mặc định AUTO
bool fanState = false;  // trạng thái quạt

unsigned long lastTime = 0;
unsigned long delayTime = 2000;

bool alertSent = false; // tránh spam cảnh báo

// ===== Xử lý Telegram =====
void handleMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;

    if (text == "/start") {
      bot.sendMessage(CHAT_ID,
        "LENH:\n"
        "/temp - xem nhiet do\n"
        "/on - bat quat\n"
        "/off - tat quat\n"
        "/auto - che do tu dong\n"
        "/manual - che do tay",
        "");
    }

    if (text == "/temp") {
      float temp = dht.readTemperature();
      bot.sendMessage(CHAT_ID, "Nhiet do: " + String(temp) + " C", "");
    }

    if (text == "/auto") {
      autoMode = true;
      bot.sendMessage(CHAT_ID, "Da chuyen sang AUTO", "");
    }

    if (text == "/manual") {
      autoMode = false;
      bot.sendMessage(CHAT_ID, "Da chuyen sang MANUAL", "");
    }

    if (text == "/on" && !autoMode) {
      fanState = true;
      bot.sendMessage(CHAT_ID, "Quat BAT (Manual)", "");
    }

    if (text == "/off" && !autoMode) {
      fanState = false;
      bot.sendMessage(CHAT_ID, "Quat TAT (Manual)", "");
    }
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  WiFi.begin(ssid, password);
  client.setInsecure();

  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" OK");

  bot.sendMessage(CHAT_ID, "ESP32 SAN SANG!", "");
}

// ===== Loop =====
void loop() {
  if (millis() - lastTime > delayTime) {

    // đọc tin nhắn Telegram
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    float temp = dht.readTemperature();

    if (!isnan(temp)) {
      Serial.print("Nhiet do: ");
      Serial.println(temp);

      // ===== AUTO MODE =====
      if (autoMode) {
        if (temp > threshold) {
          fanState = true;
        } else {
          fanState = false;
        }
      }

      // ===== Điều khiển relay + LED =====
      digitalWrite(RELAY_PIN, fanState ? HIGH : LOW);
      digitalWrite(LED_PIN, fanState ? HIGH : LOW);

      // ===== Cảnh báo =====
      if (temp > 35 && !alertSent) {
        bot.sendMessage(CHAT_ID, "⚠️ Nhiet do cao! Quat da bat!", "");
        alertSent = true;
      }

      if (temp <= 35) {
        bot.sendMessage(CHAT_ID, "✅ Nhiet do thap. Quat da tat!", "");
        alertSent = false;
      }
    }

    lastTime = millis();
  }
}