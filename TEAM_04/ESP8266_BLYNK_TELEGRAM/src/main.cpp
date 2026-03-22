#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6IfflSWyk"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM" 
#define BLYNK_AUTH_TOKEN "65zSfaeQXzjVmZqpLpQWWm8YtSIJchR2"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h> 
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

#define BOT_TOKEN "8201594390:AAF54wA3GyW0x9KsH7PGBqFuRoxQQc2bb5Y"
#define CHAT_ID "-5087448271"
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime = 0;
const unsigned long BOT_MTBS = 1000;

#define DHTPIN 4      
#define DHTTYPE DHT22
#define LED_PIN 2      
#define SDA_PIN 21     
#define SCL_PIN 22     

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

float lastTemp = 0.0;
float lastHum = 0.0;
bool ledState = false;

// Biến quản lý trạng thái bật/tắt của Bot Telegram
bool isBotActive = false; 

void handleNewMessages(int numNewMessages);
void mainSensorTask();

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n--- BẮT ĐẦU KHỞI ĐỘNG ---");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Lỗi: Không tìm thấy OLED"));
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
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
  bot.sendMessage(CHAT_ID, "Hệ thống IoT - Team X đã khởi động!\nNhập /start để bắt đầu điều khiển.", "");
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

BLYNK_WRITE(V1) {
  int pinValue = param.asInt(); 
  ledState = (pinValue == 1);
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void mainSensorTask() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int gasValue = random(10, 300); 

  unsigned long uptimeSec = millis() / 1000;
  String uptimeStr = String(uptimeSec / 60) + "m " + String(uptimeSec % 60) + "s";

  Serial.println("--- Đọc cảm biến ---");
  Serial.print("Temp: "); Serial.println(t);
  Serial.print("Hum: "); Serial.println(h);

  Blynk.virtualWrite(V0, uptimeStr);
  Blynk.virtualWrite(V5, "Team X - IoT");
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
  display.print("LED: "); display.println(ledState ? "ON" : "OFF");
  
  if (!isnan(t)) {
    display.print("Temp: "); display.print(t); display.println(" C");
    display.print("Hum:  "); display.print(h); display.println(" %");
  } else {
    display.println("DHT: Đang chờ...");
  }
  
  display.print("Gas MQ2: "); display.println(gasValue);
  display.println("--- Team X ---");
  display.display();
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    // 1. Kiểm tra lệnh /start
    if (text == "/start") {
      isBotActive = true;
      String welcomeMsg = "✅ Đã kết nối! Bạn có thể dùng các lệnh sau:\n";
      welcomeMsg += "/led_on - Bật đèn\n";
      welcomeMsg += "/led_off - Tắt đèn\n";
      welcomeMsg += "/led_status - Xem trạng thái đèn\n";
      welcomeMsg += "/get_weather - Xem thời tiết\n";
      welcomeMsg += "/end - Ngắt kết nối";
      bot.sendMessage(chat_id, welcomeMsg, "");
      continue; // Bỏ qua các lệnh dưới, chuyển sang tin nhắn tiếp theo
    }

    // 2. Kiểm tra lệnh /end
    if (text == "/end") {
      isBotActive = false;
      bot.sendMessage(chat_id, "❌ Đã ngắt kết nối với Bot. Nhập /start để kết nối lại.", "");
      continue;
    }

    // 3. Xử lý các lệnh chức năng (Chỉ hoạt động nếu isBotActive == true)
    if (isBotActive) {
      if (text == "/led_on") {
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V1, 1);
        bot.sendMessage(chat_id, "Đèn đã được BẬT.", "");
      } 
      else if (text == "/led_off") {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
        Blynk.virtualWrite(V1, 0);
        bot.sendMessage(chat_id, "Đèn đã được TẮT.", "");
      }
      else if (text == "/led_status") {
        String status = ledState ? "ĐANG BẬT" : "ĐANG TẮT";
        bot.sendMessage(chat_id, "Trạng thái đèn: " + status, "");
      }
      else if (text == "/get_weather") {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t) && !isnan(h)) {
          bot.sendMessage(chat_id, "🌡 Nhiệt độ: " + String(t) + "°C\n💧 Độ ẩm: " + String(h) + "%", "");
        } else {
          bot.sendMessage(chat_id, "Lỗi đọc cảm biến!", "");
        }
      } 
      else {
        bot.sendMessage(chat_id, "Sai cú pháp! Xem lại các lệnh bằng cách nhập /start", "");
      }
    } 
    // 4. Nếu chưa nhập /start mà cố tình gọi lệnh
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