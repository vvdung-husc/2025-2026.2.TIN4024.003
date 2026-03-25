	/*
	THÔNG TIN NHÓM 13
	1. Thân Hoàng Phước Minh
  2. Nguyễn Tấn Nhân
  3. Nguyễn Thành Trung
	*/
#define BLYNK_TEMPLATE_ID "TMPL6iGHfH-M1"
#define BLYNK_TEMPLATE_NAME "IoT Team 13"
#define BLYNK_AUTH_TOKEN "yVi_5-_wYBsZxu3YOIu05e8bPJXfKszz"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Thông tin mạng và Telegram
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
#define BOTtoken "8701041356:AAHxrUqpLUk6Ez-26wPUSGnWAzgOuwYPapc"
#define GROUP_ID "-5002231155"

// Cấu hình chân cắm
#define LED_PIN 2
#define DHTPIN 15
#define DHTTYPE DHT22
#define MQ2_PIN 34
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Khởi tạo đối tượng
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan;
int botRequestDelay = 1000; 
float t, h;
int gasValue;

// Hàm cập nhật dữ liệu cảm biến và OLED
void updateSensors() {
  t = dht.readTemperature();
  h = dht.readHumidity();
  gasValue = analogRead(MQ2_PIN);
  unsigned long uptime = millis() / 1000;

  // Đẩy lên Blynk
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V3, gasValue);
  Blynk.virtualWrite(V4, uptime);

  // Hiển thị lên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("--- SYSTEM STATUS ---");
  display.printf("Temp: %.1f C\n", t);
  display.printf("Humi: %.1f %%\n", h);
  display.printf("Gas: %d\n", gasValue);
  display.printf("Uptime: %lu s\n", uptime);
  display.println("---------------------");
  display.println("     IoT - Team 13"); // Hiển thị Team 13 ở cuối
  display.display();
}

// Xử lý lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID) continue; // Chỉ nhận lệnh từ nhóm Team 13

    String text = bot.messages[i].text;
    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V0, 1);
      bot.sendMessage(chat_id, "✅ LED đã bật sáng!", "");
    } 
    else if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V0, 0);
      bot.sendMessage(chat_id, "❌ LED đã tắt!", "");
    }
    else if (text == "/led_status") {
      String status = digitalRead(LED_PIN) ? "ĐANG BẬT" : "ĐANG TẮT";
      bot.sendMessage(chat_id, "💡 Trạng thái đèn: " + status, "");
    }
    else if (text == "/get_weather") {
      String msg = "🌡 Nhiệt độ: " + String(t, 1) + "°C\n💧 Độ ẩm: " + String(h, 1) + "%";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// Đồng bộ nút nhấn từ Blynk về thiết bị
BLYNK_WRITE(V0) {
  int value = param.asInt();
  digitalWrite(LED_PIN, value);
  bot.sendMessage(GROUP_ID, value ? "Blynk: Đèn đã BẬT" : "Blynk: Đèn đã TẮT", "");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED fail"));
  }

  // Kết nối WiFi và Blynk
  Blynk.begin(auth, ssid, pass);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Chứng chỉ bảo mật cho Telegram

  bot.sendMessage(GROUP_ID, "🚀 Hệ thống IoT Team 13 đã sẵn sàng!", "");
}

void loop() {
  Blynk.run();
  
  if (millis() - lastTimeBotRan > botRequestDelay) {
    updateSensors();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}