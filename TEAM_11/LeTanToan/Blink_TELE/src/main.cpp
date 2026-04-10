/*Không up lên được cái chung nên e up riêng ạ
 * PROJECT: ESP32 BLYNK SMART HOME - NHÓM 11
 * BOARD: ESP32
 * CONNECTIVITY: WiFi (Blynk IoT Cloud & Telegram Bot)
 */
#include "secrets.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// --- 2. CẤU HÌNH CHÂN CẮM (PINS) ---
#define DHTPIN      4
#define DHTTYPE     DHT22
#define LED_PIN     23
#define GAS_PIN     34
#define BTN_PIN     22 

// --- 1. BIẾN TOÀN CỤC & KHỞI TẠO ĐỐI TƯỢNG ---
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool ledStatus = LOW;
bool lastBtnState = HIGH;
float currentT = 0; // Lưu nhiệt độ để Telegram lấy ngay lập tức
float currentH = 0; // Lưu độ ẩm để Telegram lấy ngay lập tức
bool gasAlertSent = false;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; 
char pass[] = ""; 

// --- 2. CÁC HÀM CẬP NHẬT DỮ LIỆU (WORKERS) ---

// Cập nhật thời gian hoạt động lên Blynk (V4)
void updateUptime() {
    Blynk.virtualWrite(V4, millis() / 1000);
}

// Đọc cảm biến định kỳ (V1, V2, V3)
void updateSensors() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int gas = analogRead(GAS_PIN);

    if (!isnan(h) && !isnan(t)) {
        currentT = t; // Cất vào biến để Telegram dùng
        currentH = h;
        Blynk.virtualWrite(V1, t); 
        Blynk.virtualWrite(V2, h); 
    }
    Blynk.virtualWrite(V3, gas); 

    // Cảnh báo Gas tự động qua Telegram
    if (gas > 2000 && !gasAlertSent) {
        bot.sendMessage(CHAT_ID, "⚠️ CẢNH BÁO: Phát hiện nồng độ khí GAS cao!", "");
        gasAlertSent = true;
    } else if (gas < 1500) { 
        gasAlertSent = false; 
    }
}

// Kiểm tra nút nhấn vật lý (GPIO 22)
void checkButton() {
    bool currentBtnState = digitalRead(BTN_PIN);
    if (currentBtnState == LOW && lastBtnState == HIGH) {
        delay(50); // Chống nhiễu
        if (digitalRead(BTN_PIN) == LOW) {
            ledStatus = !ledStatus;
            digitalWrite(LED_PIN, ledStatus);
            Blynk.virtualWrite(V0, ledStatus); // Cập nhật ngược lên App Blynk
        }
    }
    lastBtnState = currentBtnState;
}

// --- 3. XỬ LÝ TELEGRAM (TỐI ƯU TỐC ĐỘ) ---

void handleTelegram() {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            String text = bot.messages[i].text;
            String sender_id = bot.messages[i].chat_id;
            
            if (text == "/start") {
                String welcome = "Chào Toàn và Nhóm 11. Chọn lệnh điều khiển bên dưới:";
                // Tạo Menu nút bấm để không cần gõ lệnh
                String keyboardJson = "[[\"/led_on\", \"/led_off\"],[\"/led_status\", \"/get_weather\"]]";
                bot.sendMessageWithReplyKeyboard(sender_id, welcome, "", keyboardJson, true);
            }

            if (text == "/led_on") {
                digitalWrite(LED_PIN, HIGH);
                ledStatus = HIGH;
                Blynk.virtualWrite(V0, HIGH);
                bot.sendMessage(sender_id, "💡 Đèn đã BẬT", "");
            }

            if (text == "/led_off") {
                digitalWrite(LED_PIN, LOW);
                ledStatus = LOW;
                Blynk.virtualWrite(V0, LOW);
                bot.sendMessage(sender_id, "🌑 Đèn đã TẮT", "");
            }

            if (text == "/led_status") {
                bot.sendMessage(sender_id, ledStatus ? "Trạng thái: ĐANG BẬT" : "Trạng thái: ĐANG TẮT", "");
            }

            if (text == "/get_weather") {
                // Lấy giá trị từ biến global (Phản hồi cực nhanh)
                String msg = "🌡 Nhiệt độ: " + String(currentT, 1) + "°C\n";
                msg += "💧 Độ ẩm: " + String(currentH, 1) + "%";
                bot.sendMessage(sender_id, msg, "");
            }
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}

// --- 4. BLYNK CALLBACKS ---

BLYNK_CONNECTED() {
    Blynk.syncVirtual(V0); // Đồng bộ trạng thái nút nhấn khi vừa kết nối
}

BLYNK_WRITE(V0) {
    ledStatus = param.asInt();
    digitalWrite(LED_PIN, ledStatus);
}

// --- 5. SETUP & LOOP ---

void setup() {
    Serial.begin(115200);
    
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP); 
    pinMode(GAS_PIN, INPUT);

    dht.begin();
    Blynk.begin(auth, ssid, pass);
    client.setInsecure(); // Bỏ qua kiểm tra SSL để Telegram chạy nhanh hơn

    // Thiết lập Timer để tránh nghẽn Loop
    timer.setInterval(1000L, updateUptime);
    timer.setInterval(2000L, updateSensors);
    timer.setInterval(800L, handleTelegram); // Kiểm tra tin nhắn Telegram mỗi 0.8s
}

void loop() {
    Blynk.run();
    timer.run();
    checkButton(); // Nút bấm vật lý cần kiểm tra liên tục
}