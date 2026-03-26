#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Cấu hình Wifi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Thông tin Bot và Nhóm của Minh
#define BOTtoken "8761698298:AAF84WLwiYsRGnXjTeGj_P9kGDWCDr2nyzo"
#define GROUP_ID "-5133558189" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27;
const int ledPin = 23; 
bool motionDetected = false;

unsigned long lastTimeBotRan;
int botRequestDelay = 1000; // 1 giây quét tin nhắn 1 lần

// Ngắt khi có chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// Hàm xử lý lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  Serial.print("Co tin nhan moi: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID) continue; 

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    
    Serial.print("Lenh nhan duoc: ");
    Serial.println(text);

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "✅ Chao Minh, LED da BAT sang!", "");
    } 
    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "❌ LED da TAT!", "");
    }
    else if (text == "/get_state") {
      String status = digitalRead(ledPin) ? "DANG BAT" : "DANG TAT";
      bot.sendMessage(chat_id, "💡 Trang thai LED hien tai: " + status, "");
    }
    else if (text == "/start") {
      String welcome = "Chào mừng " + from_name + " đến với IoT Team!\n";
      welcome += "Dùng các lệnh: /led_on, /led_off, /get_state";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Mặc định đèn tắt khi khởi động
  
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Kết nối WiFi
  Serial.print("Connecting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Thêm chứng chỉ bảo mật
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  
  // Thông báo khi bắt đầu
  bot.sendMessage(GROUP_ID, "🚀 He thong cua Minh da Online!", "");
}

void loop() {
  // 1. Thông báo khi phát hiện chuyển động
  if (motionDetected) {
    Serial.println("Canh bao: Phat hien chuyen dong!");
    bot.sendMessage(GROUP_ID, "⚠️ Canh bao: Co chuyen dong trong khu vuc!", "");
    motionDetected = false;
  }

  // 2. Kiểm tra tin nhắn điều khiển sau mỗi 1 giây
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}