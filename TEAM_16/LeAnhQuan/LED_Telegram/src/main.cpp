#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Cấu hình WiFi ảo của Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. Điền mã Token em lấy từ BotFather vào đây
String BOTtoken = "8750914665:AAGgTWMqfUcg8TAOq2JmCXEuamYehdldEpc";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 4;   // Đèn LED
const int pirPin = 27;  // Cảm biến PIR
bool ledState = false;  // Biến lưu trạng thái đèn

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println("Nhận được lệnh: " + text);

    // Xử lý lệnh /start
    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    
    // Xử lý lệnh bật đèn
    if (text == "/led_on") {
      ledState = true;
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    
    // Xử lý lệnh tắt đèn
    if (text == "/led_off") {
      ledState = false;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }

    // Xử lý lệnh lấy trạng thái (MỚI BỔ SUNG)
    if (text == "/get_state") {
      if (ledState) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(pirPin, INPUT);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  Serial.print("Đang kết nối WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi đã kết nối thành công!");
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("Có tin nhắn mới!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}