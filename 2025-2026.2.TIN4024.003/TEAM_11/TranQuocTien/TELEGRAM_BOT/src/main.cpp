#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "secrets.h" 

// --- Cấu hình Mạng Tĩnh cho Wokwi [cite: 2026-03-09] --- 
const char* ssid = "Wokwi-GUEST";
const char* password = ""; 

IPAddress local_IP(10, 13, 37, 2);  
IPAddress gateway(10, 13, 37, 1);   
IPAddress subnet(255, 255, 255, 0); 
IPAddress primaryDNS(8, 8, 8, 8);   
IPAddress secondaryDNS(8, 8, 4, 4);

// --- Cấu hình Pin (Giữ nguyên 23 và 27) [cite: 2026-01-30] ---
const int LED_PIN = 23;    
const int PIR_PIN = 27;    

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

unsigned long lastTimeBotRan;
const int botRequestDelay = 1000;

// Hàm xử lý tin nhắn Telegram [cite: 2026-03-09]
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/start") {
      String welcome = "Hệ thống đã thông suốt DNS.\n";
      welcome += "/led_on : Bật đèn\n";
      welcome += "/led_off : Tắt đèn\n";
      welcome += "/get_state : Kiểm tra trạng thái.";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "LED tại chân 23: ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "LED tại chân 23: OFF", "");
    }

    if (text == "/get_state") {
      String res = digitalRead(LED_PIN) ? "Đang BẬT" : "Đang TẮT";
      bot.sendMessage(chat_id, "Trạng thái LED: " + res, "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);

  // Thực hiện Model Surgery cho lớp mạng [cite: 2026-01-21]
  WiFi.disconnect(true);
  delay(1000);

  // Ép thông số IP và DNS để tránh lệ thuộc vào DHCP của Wokwi [cite: 2026-03-09]
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Cấu hình IP tĩnh thất bại!");
  }

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối Wokwi-GUEST");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nKết nối thành công!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
  Serial.print("DNS hiện tại: "); Serial.println(WiFi.dnsIP()); 

  client.setInsecure(); // Chấp nhận HTTPS không cần chứng chỉ [cite: 2026-03-09]
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}