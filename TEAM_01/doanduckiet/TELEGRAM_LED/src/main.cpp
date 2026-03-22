#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Thay thế bằng thông tin mạng của bạn (nếu dùng mạch thật)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Khởi tạo Telegram BOT
#define BOTtoken "8567455182:AAGnk0zfzTyhZ67WAQbSlG7gSldr65r3_iE"
#define GROUP_ID "-5039341360"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Định nghĩa chân linh kiện
const int motionSensor = 27; // Cảm biến PIR
const int ledPin = 23;       // Đèn LED

bool motionDetected = false;

// Cài đặt thời gian quét tin nhắn Telegram (1 giây / lần)
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Hàm xử lý khi phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// Hàm định dạng chuỗi
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Hàm xử lý tin nhắn nhận được từ Telegram
void handleNewMessages(int numNewMessages) {
  Serial.print("Có tin nhắn mới: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    // Chỉ xử lý tin nhắn từ Group ID đã định (bảo mật)
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID){
      bot.sendMessage(chat_id, "Bạn không có quyền điều khiển thiết bị này!", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED đã tắt", ""); // Hoặc đổi thành "LED đã tắt"
    }

    if (text == "/get_state") {
      if (digitalRead(ledPin) == HIGH) {
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
  digitalWrite(ledPin, LOW); // Tắt LED lúc khởi động
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Kết nối WiFi
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nWiFi connected");
  
  bot.sendMessage(GROUP_ID, "IoT Developer started up");
}

void loop() {
  static uint count_ = 0;

  // 1. Kiểm tra tin nhắn Telegram đến (Polling) định kỳ mỗi giây
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // 2. Kiểm tra cảm biến chuyển động
  if(motionDetected){
    ++count_;
    Serial.print(count_); Serial.println(". MOTION DETECTED => Waiting to send to Telegram");    
    String msg = StringFormat("%u => Motion detected!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.print(count_); Serial.println(". Sent successfully to Telegram");
    motionDetected = false;
  }
}