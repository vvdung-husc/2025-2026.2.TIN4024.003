/*
THÔNG TIN NHÓM 1
1. Đoàn Đức Kiệt
2. 
3. ...
*/

#define BLYNK_TEMPLATE_ID "TMPL6XNMMkTmW"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "zJRX8608ctPtNZm8SwQdva3ZIPsCP_Pg"

#include <ESP8266WiFi.h> // Thay đổi thư viện cho ESP8266
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h> // Thay đổi thư viện Blynk cho ESP8266
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <time.h>

// --- THÔNG TIN WIFI WOKWI VÀ TELEGRAM ---
char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";            
#define BOT_TOKEN "8567455182:AAGnk0zfzTyhZ67WAQbSlG7gSldr65r3_iE"
#define CHAT_ID "-5039341360" 

// --- ĐỊNH NGHĨA CHÂN CHO ESP8266 (NodeMCU) ---
#define LED_PIN 12       // Chân D6 trên ESP8266
#define DHTPIN 14        // Chân D5 trên ESP8266
#define DHTTYPE DHT22
#define OLED_SDA 4       // Chân D2 (SDA mặc định của ESP8266)
#define OLED_SCL 5       // Chân D1 (SCL mặc định của ESP8266)
#define MQ2_PIN A0       // ESP8266 chỉ có 1 chân Analog là A0

DHT dht(DHTPIN, DHTTYPE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
BlynkTimer timer;

float lastTemp = -999.0;
float lastHum = -999.0;
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
bool ledState = false; 

time_t bootTime; 

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

bool WelcomeDisplayTimeout(uint msSleep = 5000){
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;    
  return bDone;
}

// Xử lý lệnh bật tắt LED từ Blynk
BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  ledState = (pinValue == 1);
  digitalWrite(LED_PIN, ledState ? HIGH : LOW); 
}

// Xử lý lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH); 
      Blynk.virtualWrite(V1, 1); 
      bot.sendMessage(chat_id, "💡 Đèn LED đã được BẬT", "");
    } 
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW); 
      Blynk.virtualWrite(V1, 0); 
      bot.sendMessage(chat_id, "🌑 Đèn LED đã được TẮT", "");
    } 
    else if (text == "/led_status") {
      String status = ledState ? "ĐANG BẬT 💡" : "ĐANG TẮT 🌑";
      bot.sendMessage(chat_id, "Trạng thái LED hiện tại: " + status, "");
    } 
    else if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      
      // ESP8266 ADC đọc từ 0-1023
      int rawGas = analogRead(MQ2_PIN);
      int gasPercent = map(rawGas, 0, 1023, 0, 100);
      
      if (isnan(t) || isnan(h)) {
        bot.sendMessage(chat_id, "Lỗi: Không thể đọc dữ liệu từ cảm biến DHT!", "");
      } else {
        String msg = "🌡 Thời tiết hiện tại:\n- Nhiệt độ: " + String(t) + "°C\n- Độ ẩm: " + String(h) + "%\n- Khí Gas: " + String(gasPercent) + "%";
        bot.sendMessage(chat_id, msg, "");
      }
    }
    else if (text == "/start") {
      bot.sendMessage(chat_id, "Chào mừng! Gõ /get_weather, /led_on, /led_off, hoặc /led_status để điều khiển.", "");
    }
  }
}

// Gửi dữ liệu định kỳ
void processSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  // ESP8266 ADC đọc từ 0-1023
  int rawGas = analogRead(MQ2_PIN);
  int gasPercent = map(rawGas, 0, 1023, 0, 100);
  
  time_t now = time(nullptr);
  int uptime_seconds = now - bootTime; 
  
  bool sensorValid = !isnan(t) && !isnan(h);

  // Gửi thông số lên Blynk
  Blynk.virtualWrite(V0, uptime_seconds); 
  Blynk.virtualWrite(V4, gasPercent); 
  Blynk.virtualWrite(V5, "Team 1 - IoT");

  if (sensorValid) {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);

    // Cảnh báo thời tiết thay đổi qua Telegram
    if (abs(t - lastTemp) >= 1.0 || abs(h - lastHum) >= 1.0) {
      if (lastTemp != -999.0) {
        String msg = "⚠️ Cảnh báo thay đổi thời tiết:\nNhiệt độ: " + String(t) + "°C\nĐộ ẩm: " + String(h) + "%";
        bot.sendMessage(CHAT_ID, msg, "");
      }
      lastTemp = t;
      lastHum = h;
    }
  }

  // Cập nhật màn hình OLED
  if (WelcomeDisplayTimeout()) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    
    oled.setCursor(0, 10); oled.print("Team 1 - IoT");
    oled.drawLine(0, 12, 128, 12);
    
    if (sensorValid) {
      oled.setCursor(0, 24); oled.print("Temp: "); oled.print(t, 1); oled.print(" C");
      oled.setCursor(0, 36); oled.print("Hum : "); oled.print(h, 1); oled.print(" %");
    } else {
      oled.setCursor(0, 24); oled.print("Temp: Error");
      oled.setCursor(0, 36); oled.print("Hum : Error");
    }
    
    oled.setCursor(0, 48); oled.print("Gas : "); oled.print(gasPercent); oled.print(" %");
    
    oled.setCursor(70, 24); oled.print("LED:"); oled.print(ledState ? "ON " : "OFF");
    oled.setCursor(70, 36); oled.print("Up:"); oled.print(uptime_seconds); oled.print("s");
    
    oled.sendBuffer();
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 
  
  // ESP8266 không cần pinMode cho A0 khi dùng analogRead, nhưng để nguyên cũng không sao.

  dht.begin();
  // Khởi tạo I2C cho ESP8266
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");  
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "IoT - Nhóm 1");  
  oled.sendBuffer();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Đồng bộ thời gian thực
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(500);
    now = time(nullptr);
  }
  
  bootTime = time(nullptr);
  
  // ESP8266 sử dụng setInsecure để bỏ qua xác thực chứng chỉ SSL với Telegram
  secured_client.setInsecure();

  // Dọn dẹp tin nhắn kẹt của Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while(numNewMessages) {
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  timer.setInterval(2500L, processSensorData);
}

void loop() {
  Blynk.run();
  timer.run();

  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}