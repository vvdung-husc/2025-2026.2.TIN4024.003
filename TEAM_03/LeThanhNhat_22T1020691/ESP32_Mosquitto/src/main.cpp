#define BLYNK_TEMPLATE_ID "TMPL6tmkLUgW6"
#define BLYNK_TEMPLATE_NAME "ESP32 Mosquitto"
#define BLYNK_AUTH_TOKEN "FnepU871ba24uH51ZVuWClf9wF3KgDfK"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const int LED_PIN = 27;
const bool LED_ACTIVE_HIGH = true;

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define TOPIC "home/led/ThanhNhat"

#define BOT_TOKEN "8732002192:AAGsnGr_zOaQwRALsOPynVT1XNqW5SZge2M"
#define CHAT_ID "6251864754"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

WiFiClientSecure tgClient;
UniversalTelegramBot bot(BOT_TOKEN, tgClient);

bool ledState = false;
uint32_t lastMqttRetry = 0;
uint32_t lastTelegramPoll = 0;
bool teleNotified = false;

// Chặn xung sync ban đầu của Blynk
bool blynkReady = false;
uint32_t blynkReadyAt = 0;

// Anti-loop MQTT/Blynk/Telegram
uint32_t lastCommandMs = 0;
String lastCommandSource = "";

void writeLedRaw(bool on) {
  int level = LED_ACTIVE_HIGH ? (on ? HIGH : LOW) : (on ? LOW : HIGH);
  digitalWrite(LED_PIN, level);
  delay(2);
  Serial.printf("[GPIO] LED pin=%d -> %s (read=%d)\n", LED_PIN, on ? "HIGH" : "LOW", digitalRead(LED_PIN));
}

void setLed(bool st, bool syncBlynk = true) {
  ledState = st;
  writeLedRaw(st);

  if (syncBlynk && Blynk.connected()) {
    Blynk.virtualWrite(V0, st ? 1 : 0);
    Blynk.virtualWrite(V1, st ? 1 : 0);
  }
}

void publishMqttState() {
  if (mqttClient.connected()) {
    mqttClient.publish(TOPIC, ledState ? "ON" : "OFF", true);
  }
}

void applyFromSource(bool st, const char *source) {
  setLed(st, true);

  lastCommandMs = millis();
  lastCommandSource = source;

  // Chỉ publish khi lệnh không đến từ MQTT
  if (strcmp(source, "mqtt") != 0) {
    publishMqttState();
  }

  Serial.printf("[LED] %s by %s\n", st ? "ON" : "OFF", source);
}

String menuText() {
  return "ESP32 MQTT Bot Online\n"
         "/on - Bat den\n"
         "/off - Tat den\n"
         "/status - Trang thai\n"
         "/menu - Hien menu";
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, TOPIC) != 0) return;

  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  // Bỏ qua echo/override trong 1.5s sau lệnh local (telegram/blynk)
  if (millis() - lastCommandMs < 1500 && lastCommandSource != "mqtt") {
    Serial.println("[MQTT] ignore echo/override");
    return;
  }

  if (msg.equalsIgnoreCase("ON")) applyFromSource(true, "mqtt");
  else if (msg.equalsIgnoreCase("OFF")) applyFromSource(false, "mqtt");
}

void ensureMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqttClient.connected()) return;

  uint32_t now = millis();
  if (now - lastMqttRetry < 2500) return;
  lastMqttRetry = now;

  String cid = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  if (mqttClient.connect(cid.c_str())) {
    mqttClient.subscribe(TOPIC);
    publishMqttState();
    Serial.println("[MQTT] Connected");
  } else {
    Serial.printf("[MQTT] fail state=%d\n", mqttClient.state());
  }
}

BLYNK_WRITE(V0) {
  if (!blynkReady) return; // chặn sync ban đầu
  int v = param.asInt();

  // Nếu vừa có lệnh từ telegram/mqtt trong 1.5s thì bỏ qua lệnh blynk ngược
  if (millis() - lastCommandMs < 1500 && lastCommandSource != "blynk") {
    Serial.println("[BLYNK] ignore override");
    return;
  }

  applyFromSource(v == 1, "blynk");
}

void handleTelegramMessages(int n) {
  for (int i = 0; i < n; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized", "");
      continue;
    }

    if (text == "/on") {
      applyFromSource(true, "telegram");
      bot.sendMessage(chat_id, "LED ON", "");
    } else if (text == "/off") {
      applyFromSource(false, "telegram");
      bot.sendMessage(chat_id, "LED OFF", "");
    } else if (text == "/status") {
      bot.sendMessage(chat_id, ledState ? "LED is ON" : "LED is OFF", "");
    } else if (text == "/start" || text == "/menu" || text == "/help") {
      bot.sendMessage(chat_id, menuText(), "");
    }
  }
}

void telegramLoop() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastTelegramPoll < 2000) return;
  lastTelegramPoll = millis();

  int n = bot.getUpdates(bot.last_message_received + 1);
  while (n) {
    handleTelegramMessages(n);
    n = bot.getUpdates(bot.last_message_received + 1);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Boot test: sáng 3 giây
  writeLedRaw(true);
  delay(3000);
  writeLedRaw(false);
  ledState = false;

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(200);
  Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());

  tgClient.setInsecure(); // Wokwi/demo

  Blynk.config(BLYNK_AUTH_TOKEN);
  if (Blynk.connect(5000)) {
    Blynk.virtualWrite(V0, ledState ? 1 : 0);
    Blynk.virtualWrite(V1, ledState ? 1 : 0);
    blynkReadyAt = millis();
    blynkReady = false;
  }

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  bot.sendMessage(CHAT_ID, menuText(), "");
  teleNotified = true;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!Blynk.connected()) {
      if (Blynk.connect(1000)) {
        Blynk.virtualWrite(V0, ledState ? 1 : 0);
        Blynk.virtualWrite(V1, ledState ? 1 : 0);
        blynkReadyAt = millis();
        blynkReady = false;
      }
    }
    Blynk.run();
  }

  if (!blynkReady && millis() - blynkReadyAt > 2000) {
    blynkReady = true;
  }

  ensureMqtt();
  if (mqttClient.connected()) mqttClient.loop();

  if (!teleNotified && WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, menuText(), "");
    teleNotified = true;
  }

  telegramLoop();
  delay(10);
}