#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram
// #define BOT_TOKEN "xxx"
// #define CHAT_ID "xxx"
#include "secret.h" //tạo file secret.h trong src để tránh lộ thông tin 

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// PIN
#define LED_PIN 23
#define PIR_PIN 27

bool ledState = false;
bool autoMode = false;
bool lastMotionState = false;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;

    if (text == "/start") {
      bot.sendMessage(CHAT_ID,
        "Xin chào, hệ thống đã thông suốt DNS.\n"
        "Sử dụng các lệnh sau để điều khiển đèn LED:\n\n"
        "/led_on : Bật đèn\n"
        "/led_off : Tắt đèn\n"
        "/get_state : Kiểm tra trạng thái",
        "");
    }

    if (text == "/led_on") {
      autoMode = false;
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      bot.sendMessage(CHAT_ID, "LED bật (manual)", "");
    }

    if (text == "/led_off") {
      autoMode = false;
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      bot.sendMessage(CHAT_ID, "LED tắt (manual)", "");
    }

    if (text == "/get_state") {
      String msg = "LED: ";
      msg += (ledState ? "ON" : "OFF");
      msg += "\nMode: ";
      msg += (autoMode ? "AUTO" : "MANUAL");

      bot.sendMessage(CHAT_ID, msg, "");
    }

    if (text == "/auto_on") {
      autoMode = true;
      bot.sendMessage(CHAT_ID, "Đã bật chế độ AUTO", "");
    }

    if (text == "/auto_off") {
      autoMode = false;
      bot.sendMessage(CHAT_ID, "Đã tắt AUTO", "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");

  client.setInsecure();
}

void loop() {
  // 📩 Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  // 🕵️ PIR xử lý
  if (autoMode) {
    bool motion = digitalRead(PIR_PIN);

    if (motion && !lastMotionState) {
      Serial.println("Motion detected!");
      digitalWrite(LED_PIN, HIGH);
      ledState = true;

      bot.sendMessage(CHAT_ID, "⚠️ Có chuyển động!", "");
    }

    if (!motion && lastMotionState) {
      Serial.println("No motion");
      digitalWrite(LED_PIN, LOW);
      ledState = false;
    }

    lastMotionState = motion;
  }

  delay(500);
}