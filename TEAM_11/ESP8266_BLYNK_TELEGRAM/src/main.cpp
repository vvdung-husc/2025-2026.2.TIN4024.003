/*
 * PROJECT: ESP32 BLYNK SMART HOME
 * NHÓM 11:
 * Trần Quốc Tiến
 * Đặng Tấn Phát
 * Trần Đức Quốc Chí
 * Hồ Văn Thạnh
 * Lê Tấn Toàn
 */

#define BLYNK_TEMPLATE_ID   "TMPL6c0cqfIJ6"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN    "9gbOkBxTHIn7Iu-tFhjPAKEZDYZjzquO"

// --- 1. THƯ VIỆN ---
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// --- 2. TELEGRAM CONFIG ---
#define BOT_TOKEN "8643001862:AAEVaMnyx0cHiHrkf3RcYKS6jNeKwnBr-zw"
#define CHAT_ID   "-5170429956"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// --- 3. CẤU HÌNH CHÂN ---
#define DHTPIN   4
#define DHTTYPE  DHT22
#define LED_PIN  23
#define GAS_PIN  34
#define BTN_PIN  22

// --- 4. PROTOTYPES ---
void updateUptime();
void updateSensors();
void checkButton();
void handleTelegram();

// --- 5. BIẾN TOÀN CỤC ---
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool ledStatus = LOW;
bool lastBtnState = HIGH;
bool gasAlertSent = false;

float lastTemp = NAN;
float lastHum  = NAN;

String activeChatId = CHAT_ID;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// THÊM BIẾN NÀY ĐỂ CHỐNG SPAM TIN NHẮN
unsigned long lastWeatherAlertTime = 0;

// --- 6. BLYNK CONNECTED ---
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0);
  updateUptime();
  updateSensors();
  Serial.println(">>> System Synchronized with Blynk Cloud <<<");
}

// --- 7. HÀM CẬP NHẬT UPTIME ---
void updateUptime() {
  Blynk.virtualWrite(V4, millis() / 1000);
}

// --- 8. HÀM CẬP NHẬT CẢM BIẾN ---
void updateSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gas = analogRead(GAS_PIN);

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  }
  Blynk.virtualWrite(V3, gas);

  // --- Cảnh báo gas ---
  if (gas > 2000 && !gasAlertSent) {
    String alertMsg = "⚠️ CẢNH BÁO KHÍ GAS CAO!\n";
    alertMsg += "Mức Gas hiện tại: " + String(gas);

    Serial.print("Sending gas alert to: ");
    Serial.println(activeChatId);

    bot.sendMessage(activeChatId, alertMsg, "");
    gasAlertSent = true;
  }

  // Reset khi an toàn trở lại
  if (gas < 1500) {
    gasAlertSent = false;
  }

  // --- Gửi nhiệt độ/độ ẩm khi thay đổi (Đã tối ưu) ---
  if (!isnan(h) && !isnan(t)) {
    // Tăng mức chênh lệch lên: Nhiệt độ lệch >= 1.0 độ HOẶC độ ẩm lệch >= 5.0%
    if (isnan(lastTemp) || isnan(lastHum) || abs(t - lastTemp) >= 1.0 || abs(h - lastHum) >= 5.0) {
      
      // Chống spam: Đảm bảo khoảng cách giữa 2 lần gửi tin nhắn ít nhất là 60 giây (60000 ms)
      if (millis() - lastWeatherAlertTime > 60000) {
        String msg = "📢 Cập nhật thay đổi môi trường:\n";
        msg += "🌡 Nhiệt độ: " + String(t) + " °C\n";
        msg += "💧 Độ ẩm: " + String(h) + " %";

        Serial.print("Sending sensor update to: ");
        Serial.println(activeChatId);

        bot.sendMessage(activeChatId, msg, "");

        // Cập nhật lại mốc so sánh
        lastTemp = t;
        lastHum = h;
        lastWeatherAlertTime = millis();
      }
    }
  }

  Serial.printf("T: %.1fC | H: %.1f%% | Gas: %d\n", t, h, gas);
  Serial.println("--- Information provided by Team 11 ---");
}

// --- 9. XỬ LÝ NÚT NHẤN VẬT LÝ ---
void checkButton() {
  bool currentBtnState = digitalRead(BTN_PIN);

  if (currentBtnState == LOW && lastBtnState == HIGH) {
    delay(50);
    if (digitalRead(BTN_PIN) == LOW) {
      ledStatus = !ledStatus;
      digitalWrite(LED_PIN, ledStatus);
      Blynk.virtualWrite(V0, ledStatus);

      Serial.print("Physical Button -> LED: ");
      Serial.println(ledStatus ? "ON" : "OFF");
    }
  }

  lastBtnState = currentBtnState;
}

// --- 10. NHẬN LỆNH TỪ BLYNK ---
BLYNK_WRITE(V0) {
  ledStatus = param.asInt();
  digitalWrite(LED_PIN, ledStatus);

  Serial.print("Web/App Command -> LED: ");
  Serial.println(ledStatus ? "ON" : "OFF");
}

// --- 11. XỬ LÝ TELEGRAM ---
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String fromChatId = bot.messages[i].chat_id;

      activeChatId = fromChatId;

      Serial.print("fromChatId = ");
      Serial.println(fromChatId);

      if (text == "/start") {
        String welcome = "Xin chào, thông tin nhóm:\n";
        welcome += "1. Trần Quốc Tiến\n";
        welcome += "2. Trần Đức Quốc Chí\n";
        welcome += "3. Lê Tấn Toàn\n";
        welcome += "4. Đặng Tấn Phát\n";
        welcome += "5. Hồ Văn Thạnh\n\n";
        welcome += "Sử dụng các lệnh sau để điều khiển đèn LED:\n";
        welcome += "/led_on : Bật đèn\n";
        welcome += "/led_off : Tắt đèn\n";
        welcome += "/led_status : Kiểm tra trạng thái\n";
        welcome += "/get_weather : Xem nhiệt độ, độ ẩm";

        bot.sendMessage(fromChatId, welcome, "");
      }
      else if (text == "/led_on") {
        ledStatus = HIGH;
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V0, ledStatus);
        bot.sendMessage(fromChatId, "LED is ON", "");
      }
      else if (text == "/led_off") {
        ledStatus = LOW;
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V0, ledStatus);
        bot.sendMessage(fromChatId, "LED is OFF", "");
      }
      else if (text == "/led_status") {
        bot.sendMessage(fromChatId, ledStatus ? "STATUS: LED ON" : "STATUS: LED OFF", "");
      }
      else if (text == "/get_weather") {
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
          bot.sendMessage(fromChatId, "Khong doc duoc du lieu tu DHT22", "");
        } else {
          String msg = "🌡 Temp: " + String(t) + " C\n";
          msg += "💧 Humidity: " + String(h) + " %";
          bot.sendMessage(fromChatId, msg, "");
        }
      }
    }

    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// --- 12. SETUP ---
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(GAS_PIN, INPUT);

  dht.begin();
  client.setInsecure();

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(1000L, updateUptime);
  timer.setInterval(2000L, updateSensors);
}

// --- 13. LOOP ---
void loop() {
  Blynk.run();
  timer.run();
  checkButton();
  handleTelegram();
}