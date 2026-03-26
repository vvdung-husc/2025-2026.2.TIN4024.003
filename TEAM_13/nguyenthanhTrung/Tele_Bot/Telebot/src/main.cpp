#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Cấu hình Telegram
#define BOTtoken "8786405645:AAGCHs9R1_lirLC77KClTCG9o1eAxgcRDCc"
// ID của NHÓM (Phải có dấu trừ ở đầu nếu là Group)
#define CHAT_ID "-5196248953" 

const int ledPin = 23;
bool ledState = LOW;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan;
int botRequestDelay = 1000;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    // Lấy ID của nơi gửi tin nhắn (có thể là chat riêng hoặc chat nhóm)
    String chat_id = String(bot.messages[i].chat_id);
    
    // Bảo mật: Chỉ xử lý tin nhắn đến từ Nhóm đã định nghĩa
    if (chat_id != CHAT_ID) {
      continue; 
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    // Xử lý các câu lệnh (loại bỏ phần @bot_name nếu có)
    if (text == "/start" || text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Chào mừng bạn đến với nhóm điều khiển thiết bị.\n\n";
      welcome += "Sử dụng menu hoặc gõ:\n";
      welcome += "/led_on : Bật sáng đèn\n";
      welcome += "/led_off : Tắt đèn\n";
      welcome += "/get_state : Trạng thái hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on" || text == "/led_on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }

    if (text == "/led_off" || text == "/led_off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, " LED đã tắt", "");
    }

    if (text == "/get_state" || text == "/get_state") {
      String state = ledState ? "ON" : "OFF";
      bot.sendMessage(chat_id, " LED is " + state, "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setInsecure(); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}