/*
 * PROJECT: ESP32 BLYNK SMART HOME
 * NHÓM 11: Trần Quốc Tiến
 * BOARD: ESP32
 * CONNECTIVITY: WiFi (Blynk IoT Cloud)
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

#define BOT_TOKEN "8643001862:AAEVaMnyx0cHiHrkf3RcYKS6jNeKwnBr-zw"
#define CHAT_ID "-5170429956"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
String chat_id = CHAT_ID;
// --- 2. CẤU HÌNH CHÂN (HARDWARE PINS) ---
#define DHTPIN      4
#define DHTTYPE     DHT22
#define LED_PIN     23
#define GAS_PIN     34
#define BTN_PIN     22 

void updateUptime(); 
void updateSensors(); 
void checkButton();

// --- 3. KHỞI TẠO ĐỐI TƯỢNG & BIẾN TOÀN CỤC ---
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool ledStatus = LOW;
bool lastBtnState = HIGH;
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; // Tên WiFi
char pass[] = "";            // Mật khẩu WiFi

// --- ĐỒNG BỘ KHI VỪA KẾT NỐI --- 
BLYNK_CONNECTED() { // Đồng bộ trạng thái nút nhấn V0 từ Web xuống ESP32 ngay lập tức 
  Blynk.syncVirtual(V0);
// Gửi thời gian hoạt động (V4)


// Ép hệ thống gửi dữ liệu ngay lần đầu để Web không bị "đơ" 
  updateUptime(); 
  updateSensors(); 
  Serial.println(">>> System Synchronized with Blynk Cloud <<<"); 
// --- 4. CÁC HÀM HỖ TRỢ (WORKER FUNCTIONS) ---
}
void updateUptime() {
  Blynk.virtualWrite(V4, millis() / 1000);
}
// Gửi dữ liệu cảm biến lên Blynk (V1, V2, V3)
float lastTemp = 0;
float lastHum = 0;

void updateSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gas = analogRead(GAS_PIN);
  bool gasAlertSent = false;
  if (gas > 2000 && !gasAlertSent) {
  bot.sendMessage(CHAT_ID, "⚠️ CẢNH BÁO KHÍ GAS CAO!", "");
  gasAlertSent = true;
}

if (gas < 1500) { 
  gasAlertSent = false; // reset khi an toàn lại
}

  if (abs(t - lastTemp) > 0.5 || abs(h - lastHum) > 1) {
    String msg = "📢 Sensor Update\n";
    msg += "Temp: " + String(t) + "C\n";
    msg += "Humidity: " + String(h) + "%";

    bot.sendMessage(CHAT_ID, msg, "");
    lastTemp = t;
    lastHum = h;
  }
}


// Kiểm tra nút nhấn vật lý
void checkButton() {
  bool currentBtnState = digitalRead(BTN_PIN);
  if (currentBtnState == LOW && lastBtnState == HIGH) {
    delay(50); // Chống nhiễu (Debounce)
    if (digitalRead(BTN_PIN) == LOW) {
      ledStatus = !ledStatus;
      digitalWrite(LED_PIN, ledStatus);
      
      // Đồng bộ trạng thái ngược lên nút nhấn ảo trên App/Web
      Blynk.virtualWrite(V0, ledStatus);
      
      Serial.print("Physical Button -> LED: ");
      Serial.println(ledStatus ? "ON" : "OFF");
    }
  }
  lastBtnState = currentBtnState;
}

// --- 5. BLYNK CALLBACKS ---



// Nhận lệnh từ nút nhấn trên App/Web (V0)
BLYNK_WRITE(V0) {
  ledStatus = param.asInt();
  digitalWrite(LED_PIN, ledStatus);
  Serial.print("Web/App Command -> LED: ");
  Serial.println(ledStatus ? "ON" : "OFF");
}
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;
      if (text == "/start") {
        String welcome = "Xin chào, thông tin nhóm:\n";
        welcome += "1. Trần Quốc Tiến\n";
        welcome += "2. Trần Đức Quốc Chí\n";
        welcome += "3. Lê Tấn Toàn\n";
        welcome += "4. Đặng Tấn Phát\n";
        welcome += "5. Hồ Văn Thạnh \n\n";
        welcome += "Sử dụng các lệnh sau để điều khiển đèn LED:\n\n";
        welcome += "/led_on : Bật đèn\n";
        welcome += "/led_off : Tắt đèn\n";
        welcome += "/led_status : Kiểm tra trạng thái\n";
        welcome += "/get_weather : Xem nhiệt độ, độ ẩm";
        
        bot.sendMessage(chat_id, welcome, "");
      }
      if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        ledStatus = HIGH;
        bot.sendMessage(chat_id, "LED is ON", "");
      }

      if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        ledStatus = LOW;
        bot.sendMessage(chat_id, "LED is OFF", "");
      }

      if (text == "/led_status") {
        bot.sendMessage(chat_id, ledStatus ? "STATUS: LED ON" : "STATUS: LED OFF", "");
      }

      if (text == "/get_weather") {
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        String msg = "🌡 Temp: " + String(t) + " C\n";
        msg += "💧 Humidity: " + String(h) + " %";

        bot.sendMessage(chat_id, msg, "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
// --- 6. SETUP & LOOP ---

void setup() {
  Serial.begin(115200);

  // Cấu hình chân IO
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP); 
  pinMode(GAS_PIN, INPUT);

  // Khởi động cảm biến & Blynk
  dht.begin();
  Blynk.begin(auth, ssid, pass);

  // Thiết lập các khoảng thời gian gửi dữ liệu
  timer.setInterval(1000L, updateUptime);  // Mỗi 1 giây cập nhật Uptime
  timer.setInterval(2000L, updateSensors); // Mỗi 2 giây cập nhật Cảm biến
  client.setInsecure();
}

void loop() {
  Blynk.run();
  timer.run();
  checkButton(); // Kiểm tra nút nhấn liên tục trong loop
  handleTelegram();
}