/*
THÔNG TIN NHÓM 16
1. Lê Anh Quân - Telegram: @tomle2111
2. Phạm Ngọc Khanh - Telegram: @thang3008
*/
#define BLYNK_TEMPLATE_ID "TMPL6gHIEGzSP"
#define BLYNK_TEMPLATE_NAME "IoTTeam 16"
#define BLYNK_AUTH_TOKEN "DAVxm9fyomHF5qPgmpNMGI3lJ4EtO3Uk"

// Thông tin Telegram Bot
#define BOT_TOKEN "8669492985:AAHfi8RuyyBm74ZJHsJTIOTngFCH1Ovwezw"
#define CHAT_ID "-5263469312"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Cấu hình chân (Đã sửa khớp với mạch Wokwi bạn đang nối) ---
#define DHT_PIN 15
#define LED_PIN 2
#define DHT_TYPE DHT22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// --- Khởi tạo đối tượng ---
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
BlynkTimer timer;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Biến toàn cục
bool ledState = false;
float lastTemp = -999.0;
float lastHum = -999.0;
int gasValue = 0;

// ==========================================
// 1. XỬ LÝ LỆNH TỪ BLYNK (APP/WEB)
// ==========================================
BLYNK_WRITE(V4) {
  int value = param.asInt();
  ledState = (value == 1);
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  bot.sendMessage(CHAT_ID, ledState ? "Đèn LED vừa được BẬT từ Blynk!" : "Đèn LED vừa được TẮT từ Blynk!", "");
}

// ==========================================
// 2. XỬ LÝ TIN NHẮN TỪ TELEGRAM
// ==========================================
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V4, 1);
      bot.sendMessage(chat_id, "Đèn LED đã BẬT.", "");
    } 
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V4, 0);
      bot.sendMessage(chat_id, "Đèn LED đã TẮT.", "");
    } 
    else if (text == "/led_status") {
      String status = ledState ? "ĐANG BẬT" : "ĐANG TẮT";
      bot.sendMessage(chat_id, "Trạng thái LED: " + status, "");
    } 
    else if (text.startsWith("/get_weather")) { // Đã bổ sung Khí Gas vào đây
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      String msg = "📊 **Thông số hiện tại:**\n";
      msg += "🌡️ Nhiệt độ: " + String(t, 1) + " °C\n";
      msg += "💧 Độ ẩm: " + String(h, 1) + " %\n";
      msg += "💨 Khí Gas: " + String(gasValue) + " ppm";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

void checkTelegram() {
  if (WiFi.status() == WL_CONNECTED) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}

// ==========================================
// 3. ĐỌC CẢM BIẾN, CẬP NHẬT OLED VÀ BLYNK
// ==========================================
void updateSystem() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  gasValue = random(100, 500); // Giả lập khí gas vì Wokwi chưa có MQ-2 thật
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi đọc DHT22!");
    return;
  }

  // Cập nhật OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("LED: "); display.print(ledState ? "ON" : "OFF");
  display.setCursor(0, 15);
  display.print("Temp: "); display.print(t, 1); display.print(" C");
  display.setCursor(0, 30);
  display.print("Hum : "); display.print(h, 1); display.print(" %");
  display.setCursor(0, 45);
  display.print("Gas : "); display.print(gasValue);
  display.setCursor(64, 45);
  display.print(" | TEAM 16");
  display.display();

  // Gửi lên Blynk
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, gasValue);

  // Cảnh báo Telegram khi có biến động lớn
  if (lastTemp != -999.0) {
    if (abs(t - lastTemp) >= 1.0 || abs(h - lastHum) >= 2.0) {
      String msg = "⚠️ **Cảnh báo thay đổi môi trường!**\n";
      msg += "🌡️ Nhiệt độ: " + String(t, 1) + " °C\n";
      msg += "💧 Độ ẩm: " + String(h, 1) + " %\n";
      msg += "💨 Khí Gas: " + String(gasValue) + " ppm";
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
  lastTemp = t;
  lastHum = h;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  dht.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Lỗi OLED"));
  }
  display.clearDisplay();
  display.display();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  client.setInsecure();
  Blynk.config(auth);
  Blynk.connect();

  timer.setInterval(2000L, updateSystem);
  timer.setInterval(1000L, checkTelegram);
}

void loop() {
  if (Blynk.connected()) { Blynk.run(); }
  timer.run();
}