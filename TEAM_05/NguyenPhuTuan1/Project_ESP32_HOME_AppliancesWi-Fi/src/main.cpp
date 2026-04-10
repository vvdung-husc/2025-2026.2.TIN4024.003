#define BLYNK_TEMPLATE_ID "TMPL6LXR3wKk8"
#define BLYNK_TEMPLATE_NAME "ESP32 HOME CONTROL"
#define BLYNK_PRINT Serial

#include "secrets.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <UniversalTelegramBot.h>
#include <time.h>

char ssid[] = "Wokwi-GUEST";
char password[] = "";

constexpr uint8_t DHTPIN = 15;
constexpr uint8_t DHTTYPE = DHT22;
constexpr uint8_t LED_PIN = 22;
constexpr uint8_t FAN_PIN = 23;
constexpr uint8_t BTN_LED_PIN = 19;
constexpr uint8_t BTN_FAN_PIN = 18;
constexpr uint8_t LDR_PIN = 34;

#define VPIN_LIGHT_CONTROL V0
#define VPIN_FAN_CONTROL V1
#define VPIN_LIGHT_SENSOR V2
#define VPIN_TEMP_SENSOR V3
#define VPIN_STATUS V4

WiFiClientSecure securedClient;
UniversalTelegramBot bot(BOT_TOKEN, securedClient);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool ledState = false;
bool fanState = false;
bool timeSynced = false;
unsigned long lastTelegramPoll = 0;

void syncOutputs() {
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  digitalWrite(FAN_PIN, fanState ? HIGH : LOW);
}

String buildStatusMessage() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    temp = 0.0f;
  }

  int ldrValue = analogRead(LDR_PIN);
  int brightness = map(ldrValue, 0, 4095, 0, 100);

  if (Blynk.connected()) {
    Blynk.virtualWrite(VPIN_TEMP_SENSOR, temp);
    Blynk.virtualWrite(VPIN_LIGHT_SENSOR, brightness);
  }

  return "Nhiet do: " + String(temp, 1) + " C\n" +
         "Anh sang: " + String(brightness) + "%\n" +
         "Den: " + String(ledState ? "BAT" : "TAT") + "\n" +
         "Quat: " + String(fanState ? "BAT" : "TAT");
}

void sendTelegramMessage(const String& message) {
  if (WiFi.status() == WL_CONNECTED && timeSynced) {
    bot.sendMessage(CHAT_ID, message, "");
  }
}

void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(WiFi.status() == WL_CONNECTED ? "WiFi da ket noi." : "WiFi chua ket noi.");
}

void syncClock() {
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  timeSynced = getLocalTime(&timeinfo, 10000);
  Serial.println(timeSynced ? "Da dong bo thoi gian." : "Khong dong bo duoc thoi gian.");
}

void connectBlynk() {
  if (WiFi.status() != WL_CONNECTED || Blynk.connected()) {
    return;
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  Serial.println("Dang ket noi Blynk...");
  Blynk.connect(5000);
  Serial.println(Blynk.connected() ? "Blynk da ket noi." : "Blynk chua ket noi.");
}

void publishStateToBlynk() {
  if (!Blynk.connected()) {
    return;
  }

  Blynk.virtualWrite(VPIN_LIGHT_CONTROL, ledState ? 1 : 0);
  Blynk.virtualWrite(VPIN_FAN_CONTROL, fanState ? 1 : 0);
}

void sendStatus() {
  String status = buildStatusMessage();
  if (Blynk.connected()) {
    Blynk.virtualWrite(VPIN_STATUS, status);
  }
  sendTelegramMessage(status);
  Serial.println("[STATUS]");
  Serial.println(status);
}

void handleTelegram() {
  if (WiFi.status() != WL_CONNECTED || !timeSynced) {
    return;
  }

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages > 0) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;

      if (text == "/start") {
        sendTelegramMessage("Lenh ho tro: /led_on /led_off /fan_on /fan_off /status");
      } else if (text == "/led_on") {
        ledState = true;
        syncOutputs();
        publishStateToBlynk();
        sendTelegramMessage("Den da BAT tu Telegram.");
      } else if (text == "/led_off") {
        ledState = false;
        syncOutputs();
        publishStateToBlynk();
        sendTelegramMessage("Den da TAT tu Telegram.");
      } else if (text == "/fan_on") {
        fanState = true;
        syncOutputs();
        publishStateToBlynk();
        sendTelegramMessage("Quat da BAT tu Telegram.");
      } else if (text == "/fan_off") {
        fanState = false;
        syncOutputs();
        publishStateToBlynk();
        sendTelegramMessage("Quat da TAT tu Telegram.");
      } else if (text == "/status") {
        sendStatus();
      }
    }

    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void checkButtons() {
  if (digitalRead(BTN_LED_PIN) == LOW) {
    delay(200);
    ledState = !ledState;
    syncOutputs();
    publishStateToBlynk();
    sendTelegramMessage("Den da " + String(ledState ? "BAT" : "TAT") + " bang nut.");
  }

  if (digitalRead(BTN_FAN_PIN) == LOW) {
    delay(200);
    fanState = !fanState;
    syncOutputs();
    publishStateToBlynk();
    sendTelegramMessage("Quat da " + String(fanState ? "BAT" : "TAT") + " bang nut.");
  }
}

BLYNK_WRITE(VPIN_LIGHT_CONTROL) {
  ledState = param.asInt() == 1;
  syncOutputs();
}

BLYNK_WRITE(VPIN_FAN_CONTROL) {
  fanState = param.asInt() == 1;
  syncOutputs();
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_LIGHT_CONTROL, VPIN_FAN_CONTROL);
  publishStateToBlynk();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BTN_LED_PIN, INPUT_PULLUP);
  pinMode(BTN_FAN_PIN, INPUT_PULLUP);
  syncOutputs();

  securedClient.setInsecure();

  connectWifi();
  if (WiFi.status() == WL_CONNECTED) {
    syncClock();
    connectBlynk();
  }

  timer.setInterval(5000L, sendStatus);
  timer.setInterval(10000L, connectBlynk);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    timeSynced = false;
    WiFi.disconnect();
    connectWifi();
    if (WiFi.status() == WL_CONNECTED) {
      syncClock();
      connectBlynk();
    }
    delay(2000);
    return;
  }

  if (Blynk.connected()) {
    Blynk.run();
  }

  timer.run();
  checkButtons();

  if (millis() - lastTelegramPoll >= 2000) {
    lastTelegramPoll = millis();
    handleTelegram();
  }
}
