#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ---------------- THÔNG TIN CẤU HÌNH ----------------

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define BOT_TOKEN "8793384811:AAFTR346iQXoRMSygqPDOap59auhYd-Mrs8"
// ----------------------------------------------------

// Định nghĩa các chân GPIO theo file diagram.json
const int ledPin = 23;
const int pirPin = 27;

bool ledState = false; 

// Khởi tạo các client cho Telegram
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const unsigned long BOT_MTBS = 1000;
unsigned long bot_lasttime;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (from_name == "") from_name = "Guest";

    if (text == "/start") {
      String welcome = "Xin chào, Thạnh.\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = true;
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = false;
      bot.sendMessage(chat_id, "LED đã tắt", "");
    }
    else if (text == "/get_state") {
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

  // Cấu hình chân
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); 
  pinMode(pirPin, INPUT);

  // Kết nối WiFi Wokwi
  Serial.print("Connecting to WiFi ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  secured_client.setInsecure(); // Bỏ qua xác thực SSL

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  // Nhận lệnh từ Telegram
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  // --- NẾU BẠN MUỐN DÙNG CẢM BIẾN PIR ---
  // Bỏ comment (xóa dấu //) ở các dòng dưới đây để mạch tự báo 
  // về Telegram khi có người chuyển động.
  
  /*
  static bool motionDetected = false;
  int pirState = digitalRead(pirPin);
  
  if (pirState == HIGH && !motionDetected) {
    Serial.println("Phat hien chuyen dong!");
    // bot.sendMessage("CHAT_ID_CUA_BAN", "Phát hiện có chuyển động!", ""); 
    motionDetected = true;
    delay(1000); // Chống dội
  } else if (pirState == LOW) {
    motionDetected = false;
  }
  */
}