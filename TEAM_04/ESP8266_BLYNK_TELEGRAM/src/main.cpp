/*
THÔNG TIN NHÓM 4
1.Hồ Trọng Nghĩa - Telegram: Hồ Trọng Nghĩa
2.Nguyễn Thắng - Telegram: Tngnguy251
3.Hồ Gia Long - Telegram: Hồ Gia Long
4.Hoàng Nhật Duy - Telegram: nhật Duy
5.Phạm Văn Quân - Telegram: quannn56
*/

#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6eY88EV_1"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM1"
#define BLYNK_AUTH_TOKEN "jy6LQTe22OKTx3WCwuHpHW7Rkmsqv5zn"

#include <ESP8266WiFi.h> 
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h> 
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> 
#include <DHT.h>

char ssid[] = "CNTT-MMT"; 
char pass[] = "13572468";

#define BOT_TOKEN "8201594390:AAF54wA3GyW0x9KsH7PGBqFuRoxQQc2bb5Y"
#define CHAT_ID "-5087448271"
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime = 0;
const unsigned long BOT_MTBS = 1000;

// KHAI BÁO CHÂN THEO SƠ ĐỒ MẠCH ESP8266 V3
#define SCL_PIN 5       // D1
#define SDA_PIN 4       // D2
#define DHTPIN 0        // D3
#define LED_PIN 2       // D4 (Đèn LED trên bo mạch, dùng chung với chân PIR)
#define STEP_PIN 14     // D5
#define DIR_PIN 12      // D6
#define RELAY2_PIN 13   // D7
#define RELAY1_PIN 15   // D8
#define MQ2_PIN A0      // A0

#define DHTTYPE DHT11   

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
BlynkTimer timer;

float lastTemp = 0.0;
float lastHum = 0.0;
bool ledState = false; // Trạng thái LED (false = Tắt, true = Bật)
bool relay2State = false;

// Biến quản lý trạng thái bật/tắt của Bot Telegram
bool isBotActive = false; 

void handleNewMessages(int numNewMessages);
void mainSensorTask();

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n--- BẮT ĐẦU KHỞI ĐỘNG ---");
  
  // Khởi tạo các chân GPIO
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  
  // Tạm ẩn pinMode của PIR_PIN để không bị xung đột với LED_PIN trên cùng chân D4
  // pinMode(PIR_PIN, INPUT); 
  
  // Khởi tạo đèn LED trên bo mạch (chân D4)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Mặc định TẮT (Với ESP8266, mức HIGH là tắt LED trên bo)
  
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  
  if(!display.begin(0x3C, true)) { 
    Serial.println(F("Lỗi: Không tìm thấy OLED"));
  }
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE); 
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("1. Booting System...");
  display.display();

  dht.begin();
  secured_client.setInsecure(); 
  
  display.println("2. Connecting WiFi...");
  display.display();
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  display.println("-> WiFi OK!");
  display.display();

  display.println("3. Init Telegram...");
  display.display();
  Serial.println("Sending Telegram boot message...");
  bot.sendMessage(CHAT_ID, "Hệ thống IoT - Team 4 đã khởi động!\nNhập /start để bắt đầu điều khiển.", "");
  display.println("-> Telegram OK!");
  display.display();

  display.println("4. Init Blynk...");
  display.display();
  Serial.println("Connecting to Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(); 
  display.println("-> Blynk OK!");
  display.display();

  delay(1000); 
  timer.setInterval(3000L, mainSensorTask);
}

// Hàm nhận lệnh từ Blynk điều khiển LED D4
BLYNK_WRITE(V1) {
  int pinValue = param.asInt(); 
  ledState = (pinValue == 1);
  // Đèn LED D4 bật khi mức LOW, tắt khi mức HIGH
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);
}

void mainSensorTask() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int gasValue = analogRead(MQ2_PIN); 

  unsigned long uptimeSec = millis() / 1000;
  String uptimeStr = String(uptimeSec / 60) + "m " + String(uptimeSec % 60) + "s";

  Serial.println("--- Đọc cảm biến ---");
  Serial.print("Temp: "); Serial.println(t);
  Serial.print("Hum: "); Serial.println(h);
  Serial.print("Gas: "); Serial.println(gasValue);

  Blynk.virtualWrite(V0, uptimeStr);
  Blynk.virtualWrite(V5, "Team 4 - IoT");
  Blynk.virtualWrite(V4, gasValue); 
  
  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
    
    // Vẫn tự động gửi cảnh báo nhiệt độ/độ ẩm dù Bot đang ở trạng thái /end hay /start
    if (abs(t - lastTemp) >= 1.0 || abs(h - lastHum) >= 5.0) {
      String alertMsg = "⚠️ Cập nhật:\nNhiệt độ: " + String(t) + "°C\nĐộ ẩm: " + String(h) + "%";
      bot.sendMessage(CHAT_ID, alertMsg, "");
      lastTemp = t;
      lastHum = h;
    }
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Uptime: "); display.println(uptimeStr);
  display.print("LED(D4):"); display.print(ledState ? "ON " : "OFF");
  display.print(" RL2: "); display.println(relay2State ? "ON" : "OFF");
  
  if (!isnan(t)) {
    display.print("Temp: "); display.print(t); display.println(" C");
    display.print("Hum:  "); display.print(h); display.println(" %");
  } else {
    display.println("DHT: Đang chờ...");
  }
  
  display.print("Gas MQ2: "); display.println(gasValue);
  display.println("--- Team 4 ---");
  display.display();
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/start") {
      isBotActive = true;
      String welcomeMsg = "✅ Đã kết nối! Bạn có thể dùng các lệnh sau:\n";
      welcomeMsg += "/led_on - Bật đèn LED (D4)\n";
      welcomeMsg += "/led_off - Tắt đèn LED (D4)\n";
      welcomeMsg += "/led_status - Xem trạng thái đèn\n";
      welcomeMsg += "/get_weather - Xem thời tiết\n";
      welcomeMsg += "/end - Ngắt kết nối";
      bot.sendMessage(chat_id, welcomeMsg, "");
      continue; 
    }

    if (text == "/end") {
      isBotActive = false;
      bot.sendMessage(chat_id, "❌ Đã ngắt kết nối với Bot. Nhập /start để kết nối lại.", "");
      continue;
    }

    if (isBotActive) {
      // --- ĐIỀU KHIỂN LED TRÊN BO MẠCH (D4) ---
      if (text == "/led_on") {
        ledState = true;
        digitalWrite(LED_PIN, LOW); // LOW = Sáng
        Blynk.virtualWrite(V1, 1);
        bot.sendMessage(chat_id, "Đèn LED (D4) đã được BẬT.", "");
      } 
      else if (text == "/led_off") {
        ledState = false;
        digitalWrite(LED_PIN, HIGH); // HIGH = Tắt
        Blynk.virtualWrite(V1, 0);
        bot.sendMessage(chat_id, "Đèn LED (D4) đã được TẮT.", "");
      }
      else if (text == "/led_status") {
        String status = ledState ? "ĐANG BẬT" : "ĐANG TẮT";
        bot.sendMessage(chat_id, "Trạng thái đèn LED (D4): " + status, "");
      }
      else if (text == "/get_weather") {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t) && !isnan(h)) {
          bot.sendMessage(chat_id, "🌡 Nhiệt độ: " + String(t) + "°C\n💧 Độ ẩm: " + String(h) + "%", "");
        } else {
          bot.sendMessage(chat_id, "Lỗi đọc cảm biến DHT11!", "");
        }
      } 
      else {
        bot.sendMessage(chat_id, "Sai cú pháp! Xem lại các lệnh bằng cách nhập /start", "");
      }
    } 
    else {
      bot.sendMessage(chat_id, "Bot đang khóa 🔒. Vui lòng nhập /start để mở khóa điều khiển.", "");
    }
  }
}

void loop() {
  Blynk.run();
  timer.run();

  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}