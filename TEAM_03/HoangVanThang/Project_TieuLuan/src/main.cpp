#define BLYNK_TEMPLATE_ID "TMPL6A9BGHDC7"
#define BLYNK_TEMPLATE_NAME "HVThangThi"
#define BLYNK_AUTH_TOKEN "hO7tiblIU_R6BucUhCX-qcCJ2Jd-BupK"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <AccelStepper.h>
#include <UniversalTelegramBot.h>

char WIFI_SSID[] = "YOUR_WIFI_SSID";
char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

const char TELEGRAM_BOT_TOKEN[] = "8630424045:AAHOxt-485DussE69t74L0n6sAg9BNAW-wc";
const char TELEGRAM_ALLOWED_CHAT_ID[] = "6135058113";

bool blynkEnabled = true;
bool telegramEnabled = true;

constexpr uint8_t STEPPER_IN1 = 26;
constexpr uint8_t STEPPER_IN2 = 25;
constexpr uint8_t STEPPER_IN3 = 33;
constexpr uint8_t STEPPER_IN4 = 32;
constexpr uint8_t LDR_PIN = 34;

constexpr long FULL_TRAVEL_STEPS = 4000;
constexpr float STEPPER_MAX_SPEED = 900.0f;
constexpr float STEPPER_ACCELERATION = 600.0f;

constexpr int LIGHT_OPEN_THRESHOLD = 2700;
constexpr int LIGHT_CLOSE_THRESHOLD = 1700;
constexpr uint32_t SENSOR_INTERVAL_MS = 1000;
constexpr uint32_t AUTO_EVAL_INTERVAL_MS = 4000;
constexpr uint32_t TELEGRAM_POLL_INTERVAL_MS = 1200;
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000;
constexpr uint32_t BLYNK_RECONNECT_INTERVAL_MS = 5000;
constexpr uint32_t BLYNK_CONNECT_TIMEOUT_MS = 6000;

constexpr uint8_t VPIN_TARGET_POSITION = V0;
constexpr uint8_t VPIN_OPEN = V1;
constexpr uint8_t VPIN_CLOSE = V2;
constexpr uint8_t VPIN_STOP = V3;
constexpr uint8_t VPIN_AUTO_MODE = V4;
constexpr uint8_t VPIN_LIGHT = V5;
constexpr uint8_t VPIN_CURRENT_POSITION = V6;

AccelStepper stepper(AccelStepper::FULL4WIRE, STEPPER_IN1, STEPPER_IN3, STEPPER_IN2, STEPPER_IN4);
BlynkTimer timer;
WiFiClientSecure securedClient;
UniversalTelegramBot telegramBot(TELEGRAM_BOT_TOKEN, securedClient);

bool autoModeEnabled = true;
int targetPercent = 0;
int filteredLight = 0;

uint32_t lastSensorReadMs = 0;
uint32_t lastAutoEvalMs = 0;
uint32_t lastTelegramPollMs = 0;
uint32_t lastWiFiReconnectMs = 0;
uint32_t lastBlynkReconnectMs = 0;

bool blynkControlStateSynced = false;
uint8_t blynkSyncMask = 0;
constexpr uint8_t BLYNK_SYNC_TARGET_BIT = 0x01;
constexpr uint8_t BLYNK_SYNC_AUTO_BIT = 0x02;

bool isConfiguredValue(const char *value) {
  if (value == nullptr || strlen(value) == 0) {
    return false;
  }
  return strncmp(value, "YOUR_", 5) != 0;
}

long percentToSteps(int percent) {
  percent = constrain(percent, 0, 100);
  return map(percent, 0, 100, 0, FULL_TRAVEL_STEPS);
}

int stepsToPercent(long steps) {
  if (steps < 0) {
    steps = 0;
  }
  if (steps > FULL_TRAVEL_STEPS) {
    steps = FULL_TRAVEL_STEPS;
  }
  return static_cast<int>((steps * 100L) / FULL_TRAVEL_STEPS);
}

bool isTelegramChatAllowed(const String &chatId) {
  if (strlen(TELEGRAM_ALLOWED_CHAT_ID) == 0) {
    return true;
  }
  return chatId == TELEGRAM_ALLOWED_CHAT_ID;
}

void markBlynkSyncDone(uint8_t bitMask) {
  if (!Blynk.connected()) {
    return;
  }

  blynkSyncMask |= bitMask;
  if (!blynkControlStateSynced &&
      (blynkSyncMask & (BLYNK_SYNC_TARGET_BIT | BLYNK_SYNC_AUTO_BIT)) ==
          (BLYNK_SYNC_TARGET_BIT | BLYNK_SYNC_AUTO_BIT)) {
    blynkControlStateSynced = true;
    Serial.println("[Blynk] Control state synced from cloud.");
  }
}

void syncUiState() {
  if (!Blynk.connected()) {
    return;
  }

  if (blynkControlStateSynced) {
    Blynk.virtualWrite(VPIN_AUTO_MODE, autoModeEnabled ? 1 : 0);
    Blynk.virtualWrite(VPIN_TARGET_POSITION, targetPercent);
  }
  Blynk.virtualWrite(VPIN_CURRENT_POSITION, stepsToPercent(stepper.currentPosition()));
  Blynk.virtualWrite(VPIN_LIGHT, filteredLight);
}

void setCurtainTarget(int percent, bool manualCommand, const char *source) {
  int nextTargetPercent = constrain(percent, 0, 100);
  bool targetChanged = nextTargetPercent != targetPercent;
  targetPercent = nextTargetPercent;
  stepper.moveTo(percentToSteps(targetPercent));

  if (manualCommand && autoModeEnabled && targetChanged) {
    autoModeEnabled = false;
  }

  Serial.printf("[%s] Target changed to %d%%\n", source, targetPercent);
  syncUiState();
}

void stopCurtain(const char *source) {
  stepper.stop();
  targetPercent = stepsToPercent(stepper.currentPosition());
  Serial.printf("[%s] Curtain stopped at %d%%\n", source, targetPercent);
  syncUiState();
}

String getStatusText() {
  String text = "Trang thai rem\n";
  text += "- Vi tri hien tai: " + String(stepsToPercent(stepper.currentPosition())) + "%\n";
  text += "- Muc tieu: " + String(targetPercent) + "%\n";
  text += "- Cam bien anh sang: " + String(filteredLight) + "\n";
  text += "- Auto mode: ";
  text += autoModeEnabled ? "BAT" : "TAT";
  return text;
}

void sendTelegramHelp(const String &chatId) {
  String help = "Lenh ho tro:\n";
  help += "/id - Lay chat id hien tai\n";
  help += "/open - Mo rem 100%\n";
  help += "/close - Dong rem 0%\n";
  help += "/stop - Dung rem ngay\n";
  help += "/set <0-100> - Dat vi tri rem\n";
  help += "/auto_on - Bat auto mode\n";
  help += "/auto_off - Tat auto mode\n";
  help += "/status - Xem trang thai";
  telegramBot.sendMessage(chatId, help, "");
}

void processTelegramCommand(const String &chatId, String text) {
  if (!isTelegramChatAllowed(chatId)) {
    telegramBot.sendMessage(chatId, "Chat ID khong duoc phep.", "");
    return;
  }

  text.trim();
  text.toLowerCase();

  if (text == "/start") {
    text = "/help";
  }

  if (text == "/help") {
    sendTelegramHelp(chatId);
    return;
  }

  if (text == "/id") {
    telegramBot.sendMessage(chatId, "Chat ID cua ban: " + chatId, "");
    return;
  }

  if (text == "/open") {
    setCurtainTarget(100, true, "Telegram");
    telegramBot.sendMessage(chatId, "Dang mo rem 100%", "");
    return;
  }

  if (text == "/close") {
    setCurtainTarget(0, true, "Telegram");
    telegramBot.sendMessage(chatId, "Dang dong rem 0%", "");
    return;
  }

  if (text == "/stop") {
    stopCurtain("Telegram");
    telegramBot.sendMessage(chatId, "Da dung rem", "");
    return;
  }

  if (text.startsWith("/set ")) {
    int newTarget = text.substring(5).toInt();
    if (newTarget < 0 || newTarget > 100) {
      telegramBot.sendMessage(chatId, "Gia tri khong hop le. Dung /set 0..100", "");
      return;
    }

    setCurtainTarget(newTarget, true, "Telegram");
    telegramBot.sendMessage(chatId, "Da dat vi tri rem: " + String(newTarget) + "%", "");
    return;
  }

  if (text == "/auto_on") {
    autoModeEnabled = true;
    syncUiState();
    telegramBot.sendMessage(chatId, "Da bat auto mode", "");
    return;
  }

  if (text == "/auto_off") {
    autoModeEnabled = false;
    syncUiState();
    telegramBot.sendMessage(chatId, "Da tat auto mode", "");
    return;
  }

  if (text == "/status") {
    telegramBot.sendMessage(chatId, getStatusText(), "");
    return;
  }

  telegramBot.sendMessage(chatId, "Khong nhan dien lenh. Gui /help de xem danh sach lenh.", "");
}

void pollTelegram() {
  if (!telegramEnabled || WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (millis() - lastTelegramPollMs < TELEGRAM_POLL_INTERVAL_MS) {
    return;
  }
  lastTelegramPollMs = millis();

  int newMessages = telegramBot.getUpdates(telegramBot.last_message_received + 1);

  while (newMessages) {
    for (int i = 0; i < newMessages; i++) {
      processTelegramCommand(telegramBot.messages[i].chat_id, telegramBot.messages[i].text);
    }
    newMessages = telegramBot.getUpdates(telegramBot.last_message_received + 1);
  }
}

void updateLightSensor() {
  if (millis() - lastSensorReadMs < SENSOR_INTERVAL_MS) {
    return;
  }
  lastSensorReadMs = millis();

  int raw = analogRead(LDR_PIN);
  if (filteredLight == 0) {
    filteredLight = raw;
  } else {
    filteredLight = (filteredLight * 7 + raw) / 8;
  }

  if (Blynk.connected()) {
    Blynk.virtualWrite(VPIN_LIGHT, filteredLight);
  }
}

void handleAutoModeByLight() {
  if (!autoModeEnabled) {
    return;
  }

  if (millis() - lastAutoEvalMs < AUTO_EVAL_INTERVAL_MS) {
    return;
  }
  lastAutoEvalMs = millis();

  int currentPercent = stepsToPercent(stepper.currentPosition());

  if (filteredLight >= LIGHT_OPEN_THRESHOLD && currentPercent < 95) {
    setCurtainTarget(100, false, "AutoLight");
    return;
  }

  if (filteredLight <= LIGHT_CLOSE_THRESHOLD && currentPercent > 5) {
    setCurtainTarget(0, false, "AutoLight");
    return;
  }
}

void maintainConnections() {
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWiFiReconnectMs >= WIFI_RECONNECT_INTERVAL_MS) {
      lastWiFiReconnectMs = millis();
      Serial.println("WiFi disconnected. Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    return;
  }

  if (blynkEnabled && !Blynk.connected() && millis() - lastBlynkReconnectMs >= BLYNK_RECONNECT_INTERVAL_MS) {
    lastBlynkReconnectMs = millis();
    Serial.println("Reconnecting Blynk...");
    bool connected = Blynk.connect(BLYNK_CONNECT_TIMEOUT_MS);
    Serial.printf("Blynk connect result: %s\n", connected ? "OK" : "FAILED");
  }
}

void sendPeriodicStatus() {
  syncUiState();
}

BLYNK_CONNECTED() {
  Serial.println("[Blynk] Connected to cloud.");
  blynkControlStateSynced = false;
  blynkSyncMask = 0;
  Blynk.syncVirtual(VPIN_TARGET_POSITION, VPIN_AUTO_MODE);
}

BLYNK_WRITE(V0) {
  int value = param.asInt();
  Serial.printf("[Blynk] Target slider: %d%%\n", value);
  markBlynkSyncDone(BLYNK_SYNC_TARGET_BIT);
  setCurtainTarget(value, true, "BlynkSlider");
}

BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    Serial.println("[Blynk] Open command");
    setCurtainTarget(100, true, "BlynkOpen");
    Blynk.virtualWrite(VPIN_OPEN, 0);
  }
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    Serial.println("[Blynk] Close command");
    setCurtainTarget(0, true, "BlynkClose");
    Blynk.virtualWrite(VPIN_CLOSE, 0);
  }
}

BLYNK_WRITE(V3) {
  if (param.asInt() == 1) {
    Serial.println("[Blynk] Stop command");
    stopCurtain("BlynkStop");
    Blynk.virtualWrite(VPIN_STOP, 0);
  }
}

BLYNK_WRITE(V4) {
  autoModeEnabled = param.asInt() == 1;
  markBlynkSyncDone(BLYNK_SYNC_AUTO_BIT);
  Serial.printf("[Blynk] Auto mode: %s\n", autoModeEnabled ? "ON" : "OFF");
}

BLYNK_WRITE_DEFAULT() {
  Serial.printf("[Blynk] Unhandled pin V%d, value=%s\n", request.pin, param.asStr());
}

void setup() {
  Serial.begin(115200);
  delay(200);

  if (!isConfiguredValue(WIFI_SSID)) {
    strncpy(WIFI_SSID, "Wokwi-GUEST", sizeof(WIFI_SSID) - 1);
    WIFI_SSID[sizeof(WIFI_SSID) - 1] = '\0';
    strncpy(WIFI_PASSWORD, "", sizeof(WIFI_PASSWORD) - 1);
    WIFI_PASSWORD[sizeof(WIFI_PASSWORD) - 1] = '\0';
  }

  blynkEnabled = isConfiguredValue(BLYNK_AUTH_TOKEN);
  telegramEnabled = isConfiguredValue(TELEGRAM_BOT_TOKEN) && strlen(TELEGRAM_BOT_TOKEN) >= 20;

  analogReadResolution(12);
  pinMode(LDR_PIN, INPUT);

  stepper.setMaxSpeed(STEPPER_MAX_SPEED);
  stepper.setAcceleration(STEPPER_ACCELERATION);
  stepper.setCurrentPosition(0);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  if (blynkEnabled) {
    Blynk.config(BLYNK_AUTH_TOKEN);
  }
  securedClient.setInsecure();

  timer.setInterval(1000L, sendPeriodicStatus);

  Serial.printf("WiFi SSID: %s\n", WIFI_SSID);
  Serial.printf("Blynk: %s\n", blynkEnabled ? "ENABLED" : "DISABLED");
  Serial.printf("Telegram: %s\n", telegramEnabled ? "ENABLED" : "DISABLED");
  Serial.println("System started. Waiting WiFi...");
}

void loop() {
  maintainConnections();

  if (blynkEnabled) {
    Blynk.run();
  }
  timer.run();

  updateLightSensor();
  handleAutoModeByLight();
  pollTelegram();

  stepper.run();
}