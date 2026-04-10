#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h> // Thư viện cực kỳ quan trọng cho Telegram
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"
#include <time.h>

// Mình đã điền sẵn Token và Chat ID chuẩn xác cho bạn, không có khoảng trắng thừa
String BOT_TOKEN = "8638784937:AAHTSRdnsLUaPvuz6Cu_lDuFjVT-xqxih4s"; 
String CHAT_ID = "8696067864";

// --- CẤU HÌNH PHẦN CỨNG ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHTesp dht;

// --- CẤU HÌNH NTP (GIỜ VIỆT NAM) ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600; // GMT+7
const int   daylightOffset_sec = 0;

unsigned long lastTime = 0;
void sendTelegramMessage(String text);

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo DHT22
  dht.setup(15, DHTesp::DHT22);
  
  // Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Lỗi khởi tạo OLED"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // Kết nối WiFi
  WiFi.begin("Wokwi-GUEST", "", 6);
  display.setCursor(0,0);
  display.print("Ket noi WiFi...");
  display.display();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Lấy giờ Internet
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return;
  }

  float temp = dht.getTemperature();
  float hum = dht.getHumidity();

  // Hiển thị lên OLED
  display.clearDisplay();
  display.setTextSize(2); 
  display.setCursor(15, 10);
  display.printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  
  display.setTextSize(1); 
  display.setCursor(15, 40);
  display.printf("Temp: %.1f C", temp);
  display.setCursor(15, 50);
  display.printf("Hum: %.1f %%", hum);
  display.display();

  // Gửi tin nhắn Telegram mỗi khi đồng hồ ở giây số 00
  if (timeinfo.tm_sec == 0 && millis() - lastTime > 2000) {
    lastTime = millis();
    
    // Đã thay dấu cách bằng %20, xuống dòng bằng %0A để chống lỗi HTTP
    String message = "Thong_bao_thoi_tiet:%0AThoi_gian:%20" + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + 
                     "%0ANhiet_do:%20" + String(temp, 1) + "C%0ADo_am:%20" + String(hum, 1) + "%25";
    sendTelegramMessage(message);
  }
  
  delay(100);
}

// Hàm gửi tin nhắn (Đã có Client Secure)
void sendTelegramMessage(String text) {
  if(WiFi.status()== WL_CONNECTED){
    WiFiClientSecure client;
    client.setInsecure(); // Lệnh bắt buộc để bỏ qua xác thực HTTPS
    
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + text;
    
    http.begin(client, url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("Đã gửi Telegram. Mã phản hồi: ");
      Serial.println(httpResponseCode); // Nếu ra 200 là thành công!
    } else {
      Serial.print("Lỗi gửi tin nhắn: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}