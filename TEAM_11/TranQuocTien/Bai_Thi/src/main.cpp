#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include "secrets.h"

// --- CẤU HÌNH MẠNG VÀ TELEGRAM ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const String botToken = Token; // Lấy từ secrets.h
const String chatID = ID;      // Lấy từ secrets.h

// --- CẤU HÌNH THIẾT BỊ MÀN HÌNH ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- ĐỊNH NGHĨA CHÂN PIN ---
const int soilMoisturePin = 34; 
const int pumpRelayPin = 5; 
const int ledPin = 2; 

// --- THÔNG SỐ LOGIC ---
int dryThreshold = 30; // Ngưỡng độ ẩm kích hoạt (Có thể thay đổi qua Telegram)
int lastUpdateId = 0;  // Lưu trữ ID tin nhắn Telegram đã đọc
bool isPumping = false; 
int lastMoisture = -1;

// --- NGUYÊN MẪU HÀM ---
String urlEncode(String str);
void sendTelegramMessage(String message);
void updateDisplays(int moisture, bool pumpStatus);
void checkTelegramCommands(int currentMoisture);

void setup() {
  Serial.begin(115200);
  
  pinMode(pumpRelayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pumpRelayPin, LOW);
  digitalWrite(ledPin, LOW);

  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  
  // Khởi tạo OLED
  if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  oled.clearDisplay();
  oled.setTextColor(WHITE);

  // Kết nối WiFi
  Serial.print("Đang kết nối WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // --- THÊM MỚI: GỬI MENU HƯỚNG DẪN KHI VỪA KHỞI ĐỘNG XONG ---
  String welcomeMsg = "🤖 HỆ THỐNG TƯỚI CÂY ĐÃ KHỞI ĐỘNG!\n\n";
  welcomeMsg += "Danh sách các lệnh hỗ trợ:\n";
  welcomeMsg += "👉 /status : Xem độ ẩm và trạng thái bơm hiện tại.\n";
  welcomeMsg += "👉 /set <số> : Cài đặt ngưỡng độ ẩm tự động tưới (VD: /set 40).";
  
  sendTelegramMessage(welcomeMsg);
}

void loop() {
  // 1. Đọc cảm biến
  int analogValue = analogRead(soilMoisturePin);
  int moisturePercent = map(analogValue, 0, 4095, 0, 100);

  // 2. Kiểm tra lệnh từ Telegram
  checkTelegramCommands(moisturePercent);

  // 3. Logic điều khiển và cảnh báo
  if (moisturePercent < dryThreshold) {
    if (!isPumping) {
      digitalWrite(pumpRelayPin, HIGH);
      digitalWrite(ledPin, HIGH);
      isPumping = true;
      Serial.println("Phát hiện đất khô. Đã bật máy bơm!");
    }
    
    // Chỉ gửi cảnh báo khi độ ẩm thay đổi để tránh spam
    if (moisturePercent != lastMoisture) {
      String msg = "⚠️ Cảnh báo: Độ ẩm hiện tại " + String(moisturePercent) + "%. (Dưới ngưỡng " + String(dryThreshold) + "%). Bơm đang hoạt động.";
      sendTelegramMessage(msg);
      lastMoisture = moisturePercent;
    }
  } else {
    if (isPumping) {
      digitalWrite(pumpRelayPin, LOW);
      digitalWrite(ledPin, LOW);
      isPumping = false;
      Serial.println("Độ ẩm ổn định. Đã tắt máy bơm.");
      lastMoisture = -1; // Reset để lần khô tiếp theo vẫn gửi cảnh báo
    }
  }

  // 4. Cập nhật màn hình
  updateDisplays(moisturePercent, isPumping);
  
  delay(2000);
}

// --- CÁC HÀM PHỤ TRỢ ---

// Hàm kiểm tra và nhận lệnh từ Telegram
void checkTelegramCommands(int currentMoisture) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Bỏ qua xác minh chứng chỉ SSL
    HTTPClient http;

    String url = "https://api.telegram.org/bot" + botToken + "/getUpdates?offset=" + String(lastUpdateId + 1);
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        JsonArray result = doc["result"];
        for (JsonObject msg : result) {
          lastUpdateId = msg["update_id"]; 

          String text = msg["message"]["text"];
          String senderId = msg["message"]["chat"]["id"];

          // Chỉ xử lý nếu tin nhắn đến từ đúng Chat ID của bạn
          if (senderId == chatID) {
            
            // Lệnh 1: /set <giá_trị>
            if (text.startsWith("/set ")) {
              String valueStr = text.substring(5);
              int newValue = valueStr.toInt();

              if (newValue > 0 && newValue <= 100) {
                dryThreshold = newValue;
                Serial.println("Đã cập nhật ngưỡng mới: " + String(dryThreshold) + "%");
                
                String reply = "✅ Đã cập nhật ngưỡng kích hoạt thành: " + String(dryThreshold) + "%\n";
                reply += "💧 Độ ẩm đất hiện tại: " + String(currentMoisture) + "%";
                sendTelegramMessage(reply);
              } else {
                sendTelegramMessage("❌ Lỗi: Giá trị ngưỡng không hợp lệ. Vui lòng nhập từ 1 đến 100.");
              }
            }
            
            // Lệnh 2: /status
            else if (text == "/status") {
              String statusMsg = "📊 TRẠNG THÁI HỆ THỐNG:\n";
              statusMsg += "💧 Độ ẩm đất hiện tại: " + String(currentMoisture) + "%\n";
              statusMsg += "⚙️ Ngưỡng kích hoạt bơm: " + String(dryThreshold) + "%\n";
              statusMsg += "🚰 Máy bơm: ";
              statusMsg += (isPumping ? "ĐANG CHẠY 🟢" : "ĐANG TẮT 🔴");
              
              sendTelegramMessage(statusMsg);
            }
          }
        }
      }
    }
    http.end();
  }
}

// Hàm cập nhật dữ liệu hiển thị lên LCD và OLED
void updateDisplays(int moisture, bool pumpStatus) {
  // Cập nhật LCD
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moisture);
  lcd.print("%   ");
  lcd.setCursor(0, 1);
  lcd.print("Pump: ");
  lcd.print(pumpStatus ? "ON " : "OFF");

  // Cập nhật OLED
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("HE THONG TUOI CAY");
  oled.drawLine(0, 12, 128, 12, WHITE);
  
  oled.setTextSize(2);
  oled.setCursor(0, 25);
  oled.print("Hum: ");
  oled.print(moisture);
  oled.print("%");
  
  oled.setTextSize(1);
  oled.setCursor(0, 50);
  oled.print("PUMP STATUS: ");
  oled.print(pumpStatus ? "RUNNING" : "STOP   ");
  
  oled.display();
}

// Hàm mã hóa chuỗi để gửi qua HTTP GET (Xử lý tiếng Việt và khoảng trắng)
String urlEncode(String str) {
  String encodedString = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isalnum(c)) encodedString += c;
    else {
      char code0 = (c >> 4) & 0xf;
      char code1 = c & 0xf;
      encodedString += '%';
      encodedString += (char)(code0 > 9 ? code0 - 10 + 'A' : code0 + '0');
      encodedString += (char)(code1 > 9 ? code1 - 10 + 'A' : code1 + '0');
    }
  }
  return encodedString;
}

// Hàm gửi tin nhắn Telegram
void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    
    String encodedMessage = urlEncode(message);
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + encodedMessage;
    
    http.begin(client, url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode != 200) {
      Serial.print("Lỗi gửi Telegram: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}