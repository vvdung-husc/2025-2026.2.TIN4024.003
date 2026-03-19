/*
THÔNG TIN NHÓM 11
1. Trần Quốc Tiến
2. Trần Đức Quốc Chí
*/

#define BLYNK_TEMPLATE_ID "TMPL6c0cqfIJ6"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "9gbOkBxTHIn7Iu-tFhjPAKEZDYZjzquO"

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
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
  // 1. Đọc dữ liệu từ cảm biến
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gas = analogRead(GAS_PIN); // Đọc giá trị analog từ cảm biến Gas

  // Kiểm tra nếu cảm biến DHT bị lỗi (tránh gửi dữ liệu rác lên Blynk)
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // 2. Gửi dữ liệu lên Blynk App
  Blynk.virtualWrite(V1, t);   // Nhiệt độ
  Blynk.virtualWrite(V2, h);   // Độ ẩm
  Blynk.virtualWrite(V3, gas); // Nồng độ Gas

  // 3. Tự động cảnh báo qua Telegram nếu Gas vượt ngưỡng nguy hiểm (Ví dụ > 2000)
  if (gas > 2000) {
    String alertMsg = "🚨 NGUY HIỂM: Phát hiện rò rỉ GAS!\n";
    alertMsg += "Nồng độ: " + String(gas);
    bot.sendMessage(chat_id, alertMsg, "");
  }

  // 4. Gửi cập nhật Nhiệt độ/Độ ẩm định kỳ nếu có thay đổi đáng kể
  if (abs(t - lastTemp) > 0.5 || abs(h - lastHum) > 1) {
    String msg = "📊 Cập nhật môi trường:\n";
    msg += "Nhiệt độ: " + String(t) + "°C\n";
    msg += "Độ ẩm: " + String(h) + "%";
    
    bot.sendMessage(chat_id, msg, "");
    
    // Lưu lại giá trị cũ để so sánh cho lần sau
    lastTemp = t;
    lastHum = h;
  }

  // 5. In ra Serial Monitor để theo dõi (Debug)
  Serial.printf("T: %.1fC | H: %.1f%% | Gas: %d\n", t, h, gas);
  Serial.println("--- Information provided by Team 11 ---");
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
  // 1. Chỉ lấy tin nhắn mới 1 lần duy nhất mỗi khi Timer gọi hàm này
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  // 2. Nếu có tin nhắn mới thì xử lý
  if (numNewMessages > 0) {
    Serial.printf("Có %d tin nhắn mới!\n", numNewMessages);
    
    for (int i = 0; i < numNewMessages; i++) {
      String current_sender_id = String(bot.messages[i].chat_id); 
      String text = bot.messages[i].text;

      if (text == "/start") {
        String welcome = "Xin chào, thông tin nhóm 11:\n";
        welcome += "1. Trần Quốc Tiến\n2. Trần Đức Quốc Chí\n3. Lê Tấn Toàn\n4. Đặng Tấn Phát\n5. Hồ Văn Thạnh\n\n";
        welcome += "Sử dụng lệnh:\n/led_on : Bật đèn\n/led_off : Tắt đèn\n/get_state : Trạng thái\n/get_weather : Nhiệt độ\n/get_gas : Khí Gas";
        bot.sendMessage(current_sender_id, welcome, "");
      }   

      else if (text == "/led_on") {
        ledStatus = HIGH;
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V0, HIGH);
        bot.sendMessage(current_sender_id, "✅ Đèn đã BẬT", "");
      }

      else if (text == "/led_off") {
        ledStatus = LOW;
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V0, LOW);
        bot.sendMessage(current_sender_id, "❌ Đèn đã TẮT", "");
      }

      else if (text == "/get_state") { 
        String statusMsg = (ledStatus == HIGH) ? "Đèn đang BẬT 💡" : "Đèn đang TẮT 🌑";
        bot.sendMessage(current_sender_id, statusMsg, "");
      }

      else if (text == "/get_weather") {
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        String msg = "🌡 " + String(t) + "°C | 💧 " + String(h) + "%";
        bot.sendMessage(current_sender_id, msg, "");
      }

      else if (text == "/get_gas") {
        int gasValue = analogRead(GAS_PIN); 
        bot.sendMessage(current_sender_id, "☁️ Gas: " + String(gasValue), "");
      }
    }
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
  timer.setInterval(3000L, handleTelegram);
}

void loop() {
  Blynk.run();
  timer.run();
  checkButton();
  // handleTelegram(); hàm này đã được gọi trong Timer nên không cần gọi ở đây nữa
}