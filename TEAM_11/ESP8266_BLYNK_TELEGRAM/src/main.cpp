/*
THÔNG TIN NHÓM 11
1. Trần Quốc Tiến
2. Trần Đức Quốc Chí
*/
<<<<<<< HEAD

#define BLYNK_TEMPLATE_ID "TMPL6c0cqfIJ6"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "9gbOkBxTHIn7Iu-tFhjPAKEZDYZjzquO"

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
=======
#include "secrets.h" 
>>>>>>> a914f7b4bf8ef19d8435ad58e0aab96a049f3665
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// telegram
#define BOT_TOKEN "8618113869:AAGGvhkMe7qviT1j3v6K_wjMMBPALFQKN9c"
#define CHAT_ID "7633653308"
// #define CHAT_ID "5401844529"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
String chat_id = CHAT_ID;

// --- Cấu hình chân phần cứng ---
#define DHTPIN 4
#define DHTTYPE DHT22
#define LED_PIN 23
#define GAS_PIN 34
#define BTN_PIN 22 

// --- KHAI BÁO NGUYÊN MẪU HÀM (Prototypes) ---
// Bước này giúp compiler biết sự tồn tại của hàm trước khi chúng được gọi
void updateUptime();
void updateSensors();
void checkButton();

// --- Khởi tạo đối tượng ---
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool ledStatus = LOW;
bool lastBtnState = HIGH;

// --- ĐỒNG BỘ KHI VỪA KẾT NỐI ---
BLYNK_CONNECTED() {
  // Đồng bộ trạng thái nút nhấn V0 từ Web xuống ESP32 ngay lập tức
  Blynk.syncVirtual(V0); 
  
  // Ép hệ thống gửi dữ liệu ngay lần đầu để Web không bị "đơ"
  updateUptime();
  updateSensors();
  
  Serial.println(">>> System Synchronized with Blynk Cloud <<<");
}

// 1. Cập nhật Uptime (V4) mỗi 1 giây
void updateUptime() {
  Blynk.virtualWrite(V4, millis() / 1000);
}

// 2. Cập nhật cảm biến (V1, V2, V3) mỗi 2 giây
float lastTemp = 0;
float lastHum = 0;

void updateSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (abs(t - lastTemp) > 0.5 || abs(h - lastHum) > 1) {
    String msg = " Sensor Update\n";
    msg += "Temp: " + String(t) + "C\n";
    msg += "Humidity: " + String(h) + "%";

    bot.sendMessage(chat_id, msg, "");
    lastTemp = t;
    lastHum = h;
  }
<<<<<<< HEAD
=======
  Blynk.virtualWrite(V3, gas); 

  Serial.printf("T: %.1fC | H: %.1f%% | Gas: %d\n", t, h, gas);
  Serial.println("--- Information provided by Team 11 ---");
>>>>>>> a914f7b4bf8ef19d8435ad58e0aab96a049f3665
}
// 3. Xử lý nút bấm vật lý (GPIO 22) đồng bộ với Switch trên Web (V0)
void checkButton() {
  bool currentBtnState = digitalRead(BTN_PIN);
  if (currentBtnState == LOW && lastBtnState == HIGH) {
    delay(50); // Chống nhiễu
    if (digitalRead(BTN_PIN) == LOW) {
      ledStatus = !ledStatus;
      digitalWrite(LED_PIN, ledStatus);
      
      // Đồng bộ ngược lại Web: Đổi trạng thái Switch V0
      Blynk.virtualWrite(V0, ledStatus);
      
      Serial.print("Physical Button -> LED: ");
      Serial.println(ledStatus ? "ON" : "OFF");
    }
  }
  lastBtnState = currentBtnState;
}
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String current_sender_id = bot.messages[i].chat_id; 
      String text = bot.messages[i].text;

      if (text == "/start") {
        String welcome = "Xin chào, thông tin nhóm:\n";
        welcome += "1. Trần Quốc Tiến\n";
        welcome += "2. Trần Đức Quốc Chí\n";
        welcome += "3. Lê Tấn Toàn\n";
        welcome += "4. Đặng Tấn Phát\n";
        welcome += "5. Hồ Văn Thạnh\n\n";
        welcome += "Sử dụng các lệnh sau để điều khiển đèn LED:\n\n";
        welcome += "/led_on : Bật đèn\n";
        welcome += "/led_off : Tắt đèn\n";
        welcome += "/get_state : Kiểm tra trạng thái\n"; // Hướng dẫn là get_state
        welcome += "/get_weather : Xem nhiệt độ, độ ẩm";
        
        bot.sendMessage(current_sender_id, welcome, "");
      }   

      if (text == "/led_on") {
        ledStatus = HIGH;
        digitalWrite(LED_PIN, ledStatus);
        Blynk.virtualWrite(V0, ledStatus); // Cập nhật trạng thái lên App Blynk
        bot.sendMessage(current_sender_id, "Đèn đã BẬT (ON)", "");
      }

      if (text == "/led_off") {
        ledStatus = LOW;
        digitalWrite(LED_PIN, ledStatus);
        Blynk.virtualWrite(V0, ledStatus); // Cập nhật trạng thái lên App Blynk
        bot.sendMessage(current_sender_id, "Đèn đã TẮT (OFF)", "");
      }

      // Sửa từ /led_status thành /get_state để khớp với menu /start
      if (text == "/get_state") { 
        String statusMsg = "Trạng thái hiện tại: ";
        statusMsg += (ledStatus == HIGH) ? "ĐANG BẬT 💡" : "ĐANG TẮT 🌑";
        bot.sendMessage(current_sender_id, statusMsg, "");
      }

      if (text == "/get_weather") {
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (isnan(h) || isnan(t)) {
          bot.sendMessage(current_sender_id, "Lỗi: Không đọc được cảm biến!", "");
        } else {
          String msg = "🌡 Temp: " + String(t) + " C\n";
          msg += "💧 Humidity: " + String(h) + " %";
          bot.sendMessage(current_sender_id, msg, "");
        }
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
// 4. Nhận lệnh điều khiển từ Web (V0)
BLYNK_WRITE(V0) {
  ledStatus = param.asInt();
  digitalWrite(LED_PIN, ledStatus);
  Serial.print("Web/App Command -> LED: ");
  Serial.println(ledStatus ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP); 
  pinMode(GAS_PIN, INPUT);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");

  // Thiết lập Timer
  timer.setInterval(1000L, updateUptime);  // V4 nhảy mỗi 1s
  timer.setInterval(2000L, updateSensors); // V1, V2, V3 nhảy mỗi 2s
  client.setInsecure();
}

void loop() {
  Blynk.run();
  timer.run();
  checkButton();
  handleTelegram(); 
}