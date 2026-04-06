#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// ================= WIFI =================
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// ================= TELEGRAM =================
#define BOT_TOKEN "8708566986:AAEXLFpjgin7jFjMp-azSgD-dFJUkKv7kx4"
#define CHAT_ID "5394372469"

// ================= PIN =================
#define LDR_PIN 36

#define MOTOR_A_MINUS 19
#define MOTOR_A_PLUS  18
#define MOTOR_B_PLUS  17
#define MOTOR_B_MINUS 16

// ================= OBJECTS =================
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
Preferences prefs;

// ================= SYSTEM =================
int lightThreshold = 2000;
bool curtainOpen = false;
bool autoMode = true;

unsigned long lastBotCheck = 0;
const unsigned long botInterval = 1200;

unsigned long lastActionTime = 0;
const unsigned long actionCooldown = 3000;

unsigned long lastStatusPrint = 0;
const unsigned long statusPrintInterval = 1000;

int tooBrightThreshold = 3500;
int tooDarkThreshold   = 500;
String lastAlertState = "NORMAL";

// ================= STEPPER =================
int stepSequence[4][4] = {
  {1, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 1},
  {1, 0, 0, 1}
};

// ================= UI TELEGRAM =================
const String keyboardJson =
  "["
    "[{\"text\":\"/open\"},{\"text\":\"/close\"}],"
    "[{\"text\":\"/status\"}],"
    "[{\"text\":\"/auto\"},{\"text\":\"/manual\"}],"
    "[{\"text\":\"/setthreshold 1500\"}]"
  "]";

// ================= FUNCTIONS =================
void saveThreshold() {
  prefs.putInt("threshold", lightThreshold);
}

void loadThreshold() {
  lightThreshold = prefs.getInt("threshold", 2000);
}

void setStep(int aMinus, int aPlus, int bPlus, int bMinus) {
  digitalWrite(MOTOR_A_MINUS, aMinus);
  digitalWrite(MOTOR_A_PLUS,  aPlus);
  digitalWrite(MOTOR_B_PLUS,  bPlus);
  digitalWrite(MOTOR_B_MINUS, bMinus);
}

void releaseMotor() {
  digitalWrite(MOTOR_A_MINUS, LOW);
  digitalWrite(MOTOR_A_PLUS, LOW);
  digitalWrite(MOTOR_B_PLUS, LOW);
  digitalWrite(MOTOR_B_MINUS, LOW);
}

void stepMotor(int steps, bool direction) {
  for (int i = 0; i < steps; i++) {
    for (int j = 0; j < 4; j++) {
      int idx = direction ? j : (3 - j);
      setStep(
        stepSequence[idx][0],
        stepSequence[idx][1],
        stepSequence[idx][2],
        stepSequence[idx][3]
      );
      delay(3);
    }
  }
  releaseMotor();
}

bool canActNow() {
  return millis() - lastActionTime >= actionCooldown;
}

void openCurtain() {
  if (!canActNow()) return;

  if (!curtainOpen) {
    Serial.println("Dang mo rem...");
    stepMotor(200, true);
    curtainOpen = true;
    lastActionTime = millis();
    Serial.println("Rem da mo.");
  } else {
    Serial.println("Rem dang mo.");
  }
}

void closeCurtain() {
  if (!canActNow()) return;

  if (curtainOpen) {
    Serial.println("Dang dong rem...");
    stepMotor(200, false);
    curtainOpen = false;
    lastActionTime = millis();
    Serial.println("Rem da dong.");
  } else {
    Serial.println("Rem dang dong.");
  }
}

int readLightValue() {
  return analogRead(LDR_PIN);
}

String getStatusMessage() {
  int lightValue = readLightValue();

  String msg = "Trang thai he thong:\n";
  msg += "- Anh sang: " + String(lightValue) + "\n";
  msg += "- Nguong: " + String(lightThreshold) + "\n";
  msg += "- Rem: " + String(curtainOpen ? "Dang mo" : "Dang dong") + "\n";
  msg += "- Che do: " + String(autoMode ? "Tu dong" : "Thu cong") + "\n";
  msg += "- IP: " + WiFi.localIP().toString();

  return msg;
}

void sendMainMenu(const String &chat_id, const String &title = "Menu dieu khien:") {
  bot.sendMessageWithReplyKeyboard(chat_id, title, "", keyboardJson, true);
}

void sendHelpMessage(const String &chat_id, const String &from_name) {
  String msg = "Chao " + from_name + "\n";
  msg += "Danh sach lenh:\n";
  msg += "/start - Hien menu\n";
  msg += "/open - Mo rem\n";
  msg += "/close - Dong rem\n";
  msg += "/status - Xem trang thai\n";
  msg += "/auto - Bat che do tu dong\n";
  msg += "/manual - Bat che do thu cong\n";
  msg += "/setthreshold <so> - Dat nguong anh sang\n";
  msg += "/menu - Hien nut dieu khien";

  sendMainMenu(chat_id, msg);
}

void checkLightAlert() {
  if (WiFi.status() != WL_CONNECTED) return;

  int lightValue = readLightValue();
  String currentState = "NORMAL";

  if (lightValue >= tooBrightThreshold) {
    currentState = "TOO_BRIGHT";
  } else if (lightValue <= tooDarkThreshold) {
    currentState = "TOO_DARK";
  }

  if (currentState != lastAlertState) {
    lastAlertState = currentState;

    if (currentState == "TOO_BRIGHT") {
      bot.sendMessage(CHAT_ID,
        "Canh bao: Anh sang qua cao! Gia tri hien tai = " + String(lightValue),
        "");
    } else if (currentState == "TOO_DARK") {
      bot.sendMessage(CHAT_ID,
        "Canh bao: Anh sang qua thap! Gia tri hien tai = " + String(lightValue),
        "");
    }
  }
}

void handleTelegramCommand(const String &chat_id, const String &text, const String &from_name) {
  Serial.println("=== TIN NHAN MOI ===");
  Serial.println("chat_id: " + chat_id);
  Serial.println("text: " + text);

  if (chat_id != CHAT_ID) {
    bot.sendMessage(chat_id, "Ban khong co quyen dieu khien bot nay.", "");
    return;
  }

  if (text == "/start" || text == "/menu") {
    sendHelpMessage(chat_id, from_name);
  }
  else if (text == "/open") {
    autoMode = false;
    openCurtain();
    bot.sendMessage(chat_id, "Da mo rem. He thong dang o che do thu cong.", "");
  }
  else if (text == "/close") {
    autoMode = false;
    closeCurtain();
    bot.sendMessage(chat_id, "Da dong rem. He thong dang o che do thu cong.", "");
  }
  else if (text == "/status") {
    bot.sendMessage(chat_id, getStatusMessage(), "");
  }
  else if (text == "/auto") {
    autoMode = true;
    bot.sendMessage(chat_id, "Da chuyen sang che do tu dong.", "");
  }
  else if (text == "/manual") {
    autoMode = false;
    bot.sendMessage(chat_id, "Da chuyen sang che do thu cong.", "");
  }
  else if (text.startsWith("/setthreshold ")) {
    String valueStr = text.substring(14);
    valueStr.trim();
    int newThreshold = valueStr.toInt();

    if (newThreshold >= 0 && newThreshold <= 4095) {
      lightThreshold = newThreshold;
      saveThreshold();
      bot.sendMessage(chat_id,
        "Da cap nhat nguong anh sang: " + String(lightThreshold) + "\nGia tri da duoc luu.",
        "");
    } else {
      bot.sendMessage(chat_id, "Gia tri khong hop le. Hay nhap tu 0 den 4095.", "");
    }
  }
  else {
    bot.sendMessage(chat_id, "Lenh khong hop le. Dung /menu hoac /start de xem cac nut.", "");
  }
}

void checkTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = bot.messages[i].chat_id;
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;

      handleTelegramCommand(chat_id, text, from_name);
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void connectWiFi() {
  Serial.print("Dang ket noi WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi da ket noi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Ket noi WiFi THAT BAI!");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_A_MINUS, OUTPUT);
  pinMode(MOTOR_A_PLUS, OUTPUT);
  pinMode(MOTOR_B_PLUS, OUTPUT);
  pinMode(MOTOR_B_MINUS, OUTPUT);

  releaseMotor();

  prefs.begin("curtain-bot", false);
  loadThreshold();

  connectWiFi();
  client.setInsecure();

  Serial.println("Nguong da tai: " + String(lightThreshold));

  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, "ESP32 da khoi dong. He thong rem cua san sang.", "");
    sendMainMenu(CHAT_ID);
    Serial.println("Bot san sang.");
  }
}

void loop() {
  int lightValue = readLightValue();

  if (millis() - lastStatusPrint >= statusPrintInterval) {
    Serial.print("Gia tri anh sang: ");
    Serial.println(lightValue);

    if (autoMode) {
      if (lightValue > lightThreshold) {
        closeCurtain();   // sang cao -> dong rem
      } else {
        openCurtain();    // sang thap -> mo rem
      }
    }

    lastStatusPrint = millis();
  }

  checkLightAlert();

  if (WiFi.status() == WL_CONNECTED && millis() - lastBotCheck >= botInterval) {
    checkTelegram();
    lastBotCheck = millis();
  }

  delay(100);
}