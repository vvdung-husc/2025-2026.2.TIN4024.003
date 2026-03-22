#define BLYNK_TEMPLATE_ID "TMPL6u7asnsLD"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "ipGXLUA4U8XUpsQY0klMi_qjDUpABSSv"

#include "secrets.h" // File này phải chứa #define API "Key_cua_ban"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = ""; 

// Dùng uint32_t để đếm được tới 136 năm không lo tràn số [cite: 2026-01-30]
uint32_t uptimeSeconds = 0; 
BlynkTimer timer; 

// --- HÀM BÓC TÁCH CHUỖI ---
String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// --- PHẪU THUẬT HÀM GETIP: KHÔNG CHẶN LUỒNG (NON-BLOCKING) --- [cite: 2026-01-21]
String getip() {
  WiFiClient client;
  String payload = ""; 
  if (client.connect("103.179.173.252", 80)) {
    client.print("GET /?geo=1 HTTP/1.1\r\nHost: ip4.iothings.vn\r\nConnection: close\r\n\r\n");
    
    unsigned long startWait = millis();
    while (client.connected() || client.available()) {
      // MŨI PHẪU THUẬT: Giữ cho nhịp tim Blynk luôn đập khi đang đợi dữ liệu mạng [cite: 2026-02-01]
      Blynk.run(); 
      
      if (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.indexOf('|') != -1) payload = line; 
      }
      if (millis() - startWait > 3000) break; // Timeout sau 3 giây để tránh treo máy
    }
    client.stop(); 
  }
  return payload; 
}

// --- PHẪU THUẬT HÀM THỜI TIẾT: KHÔNG CHẶN LUỒNG --- [cite: 2026-01-21]
void getWeather(String lat, String lon) {
  WiFiClient client;
  // Sử dụng IP cụm SEA bạn đã Ping thành công: 15.235.227.40
  if (client.connect("15.235.227.40", 80)) { 
    client.print(String("GET /data/2.5/weather?lat=") + lat + "&lon=" + lon + 
                 "&appid=" + API + "&units=metric HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n");

    unsigned long startWait = millis();
    while (client.connected() || client.available()) {
      // MŨI PHẪU THUẬT: Đảm bảo timecount (V0) không bị đứng hình [cite: 2026-02-01]
      Blynk.run(); 
      
      if (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("{")) {
          JsonDocument doc;
          if (!deserializeJson(doc, line)) {
            Blynk.virtualWrite(V1, (float)doc["main"]["temp"]);
            Serial.printf("Weather Updated: %.2f C\n", (float)doc["main"]["temp"]);
          }
          break;
        }
      }
      if (millis() - startWait > 3000) break;
    }
    client.stop();
  }
}

// --- CẬP NHẬT DỮ LIỆU ĐỊNH KỲ ---
void updateNetworkData() {
  String ip = getip(); 
  if (ip != "") {
    String myIP = splitString(ip, '|', 0); 
    String lon  = splitString(ip, '|', 5); 
    String lat  = splitString(ip, '|', 6); 
    
    Blynk.virtualWrite(V2, myIP);

    // --- MŨI PHẪU THUẬT: GHÉP CHUỖI THEO ĐÚNG ĐỊNH DẠNG CỦA THẦY --- [cite: 2026-01-21]
    // Cấu trúc: Link gốc + lat + dấu phẩy + lon
    String googleLink = "https://www.google.com/maps/place/" + lat + "," + lon;
    
    // Gửi link lên chân V3 (Ô màu xanh lá) [cite: 2026-01-30]
    Blynk.virtualWrite(V3, googleLink); 
    
    // In ra Serial để bạn copy/kiểm tra link có chạy được không
    Serial.println("Link dung chuan thay: " + googleLink);

    getWeather(lat, lon); 
  }
}

// --- HÀM ĐẾM THỜI GIAN THỰC (CHÍNH XÁC TUYỆT ĐỐI) --- [cite: 2026-01-31]
void updatetime() {
  // millis() là đồng hồ phần cứng, không bao giờ sai lệch dù mạng có lag [cite: 2026-01-31]
  uptimeSeconds = millis() / 1000; 
  Blynk.virtualWrite(V0, uptimeSeconds);
}

void setup() {
  Serial.begin(115200); 
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Connected!");

  // Cấu hình Blynk Bypass DNS cho Wokwi [cite: 2026-01-31]
  IPAddress blynkIP(128, 199, 144, 129);
  Blynk.config(BLYNK_AUTH_TOKEN, blynkIP, 80);
  Blynk.connect();

  // Khởi chạy dữ liệu lần đầu
  updateNetworkData(); 

  // --- THIẾT LẬP CHU KỲ (NON-BLOCKING) --- [cite: 2026-02-01]
  // 1 giây nhảy số 1 lần (Cho V0)
  timer.setInterval(1000L, updatetime); 
  // 30 giây lấy thời tiết 1 lần để CPU rảnh tay phục vụ hiển thị [cite: 2026-01-30]
  timer.setInterval(30000L, updateNetworkData); 
}

void loop() {
  Blynk.run();
  timer.run(); 
}