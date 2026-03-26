#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// WiFi config
const char* WIFI_NAME = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// Telegram config
// #include "secret.h"
// Khai báo Bot Telegram
#include "secret.h"

WiFiClientSecure secureClient;
UniversalTelegramBot telegramBot(BOT_TOKEN, secureClient);

// Hardware config
#define PIN_LED 23
#define PIN_SENSOR 27

// System flags
bool isLedOn = false;
bool isAutoEnabled = false;
bool prevSensorState = false;

// =====================
// LED control
// =====================
void updateLed(bool state) {
  digitalWrite(PIN_LED, state ? HIGH : LOW);
  isLedOn = state;
}

// =====================
// Message builders
// =====================
String getSystemStatus() {
  String result = "";

  if (isLedOn) {
    result = "Đèn đang sáng";
  } else {
    result = "Đèn đang tắt";
  }

  return result;
}

// =====================
// Telegram actions
// =====================
void sendIntro() {
  telegramBot.sendMessage(
    CHAT_ID,
    "Hello Tấn Phát.\n"
    "Các lệnh điều khiển:\n\n"
    "/led_on\n"
    "/led_off\n"
    "/get_state",
    ""
  );
}

void turnLedOn() {
  isAutoEnabled = false;
  updateLed(true);
  telegramBot.sendMessage(CHAT_ID, "Bật LED", "");
}

void turnLedOff() {
  isAutoEnabled = false;
  updateLed(false);
  telegramBot.sendMessage(CHAT_ID, "Tắt LED", "");
}

void sendStatus() {
  telegramBot.sendMessage(CHAT_ID, getSystemStatus(), "");
}
// Command router
// =====================
void executeCommand(String cmd) {
  if (cmd == "/start") sendIntro();
  else if (cmd == "/led_on") turnLedOn();
  else if (cmd == "/led_off") turnLedOff();
  else if (cmd == "/get_state") sendStatus();

}

// =====================
// Telegram check
// =====================
void readTelegram() {
  int count = telegramBot.getUpdates(telegramBot.last_message_received + 1);

  while (count) {
    for (int i = 0; i < count; i++) {
      String cmd = telegramBot.messages[i].text;
      executeCommand(cmd);
    }
    count = telegramBot.getUpdates(telegramBot.last_message_received + 1);
  }
}

// =====================
// PIR sensor logic
// =====================
void processMotion() {
  if (!isAutoEnabled) return;

  bool currentState = digitalRead(PIN_SENSOR);

  if (currentState && !prevSensorState) {
    Serial.println("Motion detected!");
    updateLed(true);
    telegramBot.sendMessage(CHAT_ID, "⚠️ Có chuyển động!", "");
  }

  if (!currentState && prevSensorState) {
    Serial.println("No motion");
    updateLed(false);
  }

  prevSensorState = currentState;
}

// =====================
// Setup helpers
// =====================
void initPins() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SENSOR, INPUT);
}

void connectNetwork() {
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
}

void initTelegram() {
  secureClient.setInsecure();
}

void setup() {
  Serial.begin(115200);

  initPins();
  connectNetwork();
  initTelegram();
}

void loop() {
  readTelegram();
  processMotion();
  delay(500);
}