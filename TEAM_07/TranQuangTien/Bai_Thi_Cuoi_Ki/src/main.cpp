#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ==========================
// WIFI + TELEGRAM CONFIG
// ==========================
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* BOT_TOKEN = "8625434219:AAH9nh-EcV7BThwgLQatXFX291iovupO6OM";
const char* CHAT_ID   = "8690982418";

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ==========================
// PIN CONFIG
// ==========================
#define SOIL_PIN   34
#define RELAY_PIN  26
#define LED_PIN    2

// ==========================
// SYSTEM CONFIG
// ==========================
int thresholdDry = 30;                 // nếu độ ẩm < ngưỡng thì bật bơm
bool autoMode = true;                  // chế độ tự động
bool pumpState = false;                // trạng thái bơm hiện tại
bool lastPumpState = false;            // trạng thái bơm trước đó
bool lowMoistureAlertSent = false;     // đã gửi cảnh báo độ ẩm thấp chưa
bool moistureRecoveredSent = false;    // đã gửi thông báo đạt ngưỡng chưa

unsigned long lastTelegramCheck = 0;
unsigned long lastSensorRead = 0;
unsigned long lastNotify = 0;

const unsigned long telegramInterval = 1000;    // kiểm tra telegram mỗi 1 giây
const unsigned long sensorInterval   = 2000;    // đọc cảm biến mỗi 2 giây
const unsigned long notifyInterval   = 600000;  // báo cáo định kỳ mỗi 10 phút

// ==========================
// HELPER FUNCTIONS
// ==========================
void setPump(bool on) {
  pumpState = on;
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);   // nếu relay active LOW thì đảo lại
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

int readMoisturePercent() {
  int raw = analogRead(SOIL_PIN);

  // Map giá trị ADC thành phần trăm độ ẩm
  // Có thể cần chỉnh lại tùy cảm biến thực tế
  int percent = map(raw, 4095, 0, 0, 100);

  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  return percent;
}

String getStatusMessage() {
  int moisture = readMoisturePercent();

  String msg = "📊 *TRANG THAI HE THONG TUOI CAY*\n";
  msg += "• Do am dat: " + String(moisture) + "%\n";
  msg += "• Nguong thiet lap: " + String(thresholdDry) + "%\n";
  msg += "• Trang thai bom: " + String(pumpState ? "BAT" : "TAT") + "\n";
  msg += "• Che do hoat dong: " + String(autoMode ? "Auto" : "Manual") + "\n";
  msg += "• WiFi RSSI: " + String(WiFi.RSSI()) + " dBm";

  return msg;
}

void sendWelcomeMessage(String chat_id, String from_name) {
  String welcome = "Xin chao, " + from_name + "!\n";
  welcome += "Day la bot tuoi cay ESP32.\n\n";
  welcome += "Cac lenh co san:\n";
  welcome += "/status - Xem trang thai he thong\n";
  welcome += "/auto - Bat che do tu dong\n";
  welcome += "/manual - Bat che do thu cong\n";
  welcome += "/pump_on - Bat bom\n";
  welcome += "/pump_off - Tat bom\n";
  welcome += "/set30 - Dat nguong 30%\n";
  welcome += "/set40 - Dat nguong 40%\n";
  welcome += "/help - Xem huong dan";

  bot.sendMessage(chat_id, welcome, "");
  Serial.println("[TELEGRAM] Da gui huong dan su dung");
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.print("[TELEGRAM] Lenh nhan duoc: ");
    Serial.println(text);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Ban khong co quyen truy cap bot nay.", "");
      Serial.println("[SECURITY] Tu choi truy cap do sai CHAT_ID");
      continue;
    }

    if (text == "/start" || text == "/help") {
      sendWelcomeMessage(chat_id, from_name);
    }
    else if (text == "/status") {
      bot.sendMessage(chat_id, getStatusMessage(), "Markdown");
      Serial.println("[STATUS] Da gui trang thai he thong");
    }
    else if (text == "/auto") {
      autoMode = true;
      bot.sendMessage(chat_id, "✅ Da chuyen sang che do TU DONG.", "");
      Serial.println("[MODE] Da chuyen sang AUTO");
    }
    else if (text == "/manual") {
      autoMode = false;
      setPump(false);
      bot.sendMessage(chat_id, "✅ Da chuyen sang che do THU CONG. Bom da tat.", "");
      Serial.println("[MODE] Da chuyen sang MANUAL");
    }
    else if (text == "/pump_on") {
      autoMode = false;
      setPump(true);
      bot.sendMessage(chat_id, "✅ Da BAT bom nuoc o che do thu cong.", "");
      Serial.println("[MANUAL] Da BAT bom thu cong");
    }
    else if (text == "/pump_off") {
      autoMode = false;
      setPump(false);
      bot.sendMessage(chat_id, "✅ Da TAT bom nuoc o che do thu cong.", "");
      Serial.println("[MANUAL] Da TAT bom thu cong");
    }
    else if (text == "/set30") {
      thresholdDry = 30;
      bot.sendMessage(chat_id, "✅ Da dat nguong do am la 30%.", "");
      Serial.println("[SETTING] thresholdDry = 30%");
    }
    else if (text == "/set40") {
      thresholdDry = 40;
      bot.sendMessage(chat_id, "✅ Da dat nguong do am la 40%.", "");
      Serial.println("[SETTING] thresholdDry = 40%");
    }
    else {
      bot.sendMessage(chat_id, "❗ Lenh khong hop le. Gui /help de xem danh sach lenh.", "");
      Serial.println("[ERROR] Lenh khong hop le");
    }
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[WIFI] Dang ket noi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[WIFI] Da ket noi WiFi!");
  Serial.print("[WIFI] IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WIFI] RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void sendStartupMessage() {
  String msg = "🚀 ESP32 da khoi dong thanh cong.\n";
  msg += "He thong tuoi cay da san sang.";
  bot.sendMessage(CHAT_ID, msg, "");
  Serial.println("[SYSTEM] Da gui thong bao khoi dong");
}

// ==========================
// SETUP
// ==========================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("====================================");
  Serial.println("[SYSTEM] ESP32 STARTED");
  Serial.println("====================================");

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  setPump(false);

  connectWiFi();

  // Demo đơn giản
  client.setInsecure();

  sendStartupMessage();
}

// ==========================
// LOOP
// ==========================
void loop() {
  unsigned long now = millis();

  // 1) Đọc cảm biến và điều khiển tự động
  if (now - lastSensorRead >= sensorInterval) {
    lastSensorRead = now;

    int moisture = readMoisturePercent();

    Serial.print("[SENSOR] Do am dat hien tai: ");
    Serial.print(moisture);
    Serial.println("%");

    if (autoMode) {
      // 4.2.2 Cảnh báo độ ẩm thấp
      if (moisture < thresholdDry) {
        setPump(true);

        if (!lowMoistureAlertSent) {
          String msg = "⚠️ *CANH BAO DO AM THAP*\n";
          msg += "• Do am hien tai: " + String(moisture) + "%\n";
          msg += "• Nguong thiet lap: " + String(thresholdDry) + "%\n";
          msg += "• Bom nuoc tu dong da BAT de tuoi cay.";

          bot.sendMessage(CHAT_ID, msg, "Markdown");

          Serial.println("[ALERT] Do am thap hon nguong");
          Serial.println("[ACTION] Da gui canh bao Telegram");
          Serial.println("[ACTION] Bom nuoc tu dong BAT");

          lowMoistureAlertSent = true;
          moistureRecoveredSent = false;
        }
      }
      // 4.2.3 Trạng thái độ ẩm đạt ngưỡng
      else {
        setPump(false);

        if (!moistureRecoveredSent) {
          String msg = "✅ *DO AM DAT NGUONG*\n";
          msg += "• Do am hien tai: " + String(moisture) + "%\n";
          msg += "• Nguong thiet lap: " + String(thresholdDry) + "%\n";
          msg += "• Bom nuoc tu dong da TAT.\n";
          msg += "• He thong dang duy tri trang thai on dinh.";

          bot.sendMessage(CHAT_ID, msg, "Markdown");

          Serial.println("[INFO] Do am da dat hoac vuot nguong");
          Serial.println("[ACTION] Ngung gui canh bao");
          Serial.println("[ACTION] Bom nuoc tu dong TAT");
          Serial.println("[SYSTEM] He thong dang on dinh");

          moistureRecoveredSent = true;
        }

        lowMoistureAlertSent = false;
      }
    }

    // Gửi thông báo khi trạng thái bơm thay đổi
    if (pumpState != lastPumpState) {
      String msg = pumpState ?
        "💧 Bom nuoc vua duoc BAT." :
        "🛑 Bom nuoc vua duoc TAT.";

      bot.sendMessage(CHAT_ID, msg, "");

      Serial.print("[PUMP] Trang thai bom thay doi: ");
      Serial.println(pumpState ? "BAT" : "TAT");

      lastPumpState = pumpState;
    }
  }

  // 2) Kiểm tra lệnh Telegram
  if (now - lastTelegramCheck >= telegramInterval) {
    lastTelegramCheck = now;

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }

  // 3) Gửi báo cáo định kỳ
  if (now - lastNotify >= notifyInterval) {
    lastNotify = now;
    bot.sendMessage(CHAT_ID, getStatusMessage(), "Markdown");
    Serial.println("[REPORT] Da gui bao cao dinh ky");
  }
}