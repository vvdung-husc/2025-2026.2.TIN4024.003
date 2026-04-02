// --- CẤU HÌNH BLYNK (Phải ở dòng đầu tiên) ---
#define BLYNK_TEMPLATE_ID "TMPL6lP47LK3W"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "7Ipu7P_9iz3XewgzuX3lA9O94VUS9WVq"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// --- CẤU HÌNH WIFI (Dành cho Wokwi) ---
char ssid[] = "Wokwi-GUEST";
char pass[] = ""; 

// --- CẤU HÌNH API ---
// Đã thay thế API Key của bạn vào đây:
String openWeatherApiKey = ""; 

BlynkTimer timer;

// Biến lưu trữ thông tin
String myIP = "";
float myLat = 0.0;
float myLon = 0.0;
String mapLink = "";

// Hàm đếm thời gian hoạt động (Uptime tính bằng giây)
void sendUptime() {
  long uptimeSeconds = millis() / 1000;
  Blynk.virtualWrite(V0, uptimeSeconds);
}

BLYNK_CONNECTED() {
  // Gửi tên sinh viên lên Blynk khi vừa kết nối thành công
  Blynk.virtualWrite(V4, "Hồ Gia Long"); 
}

// Hàm xử lý gọi API và bóc tách dữ liệu
void fetchApiData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    
    // ==========================================
    // 1. LẤY IP VÀ TỌA ĐỘ TỪ IOTHINGS (XỬ LÝ CHUỖI)
    // ==========================================
    Serial.println("\n--- Đang gọi API iothings.vn ---");
    http.begin(client, "http://ip4.iothings.vn/?geo=1"); 
    int httpCodeGeo = http.GET();
    
    if (httpCodeGeo == HTTP_CODE_OK) {
      String geoPayload = http.getString();
      geoPayload.trim(); // Xóa các khoảng trắng thừa
      
      Serial.println("=> Dữ liệu thô từ iothings: " + geoPayload);
      
      // --- BẮT ĐẦU XỬ LÝ CHUỖI ---
      // Dữ liệu: IP|QuốcGia|TênQuốcGia|Vùng|ThànhPhố|KinhĐộ|VĩĐộ
      int p1 = geoPayload.indexOf('|');
      int p2 = geoPayload.indexOf('|', p1 + 1);
      int p3 = geoPayload.indexOf('|', p2 + 1);
      int p4 = geoPayload.indexOf('|', p3 + 1);
      int p5 = geoPayload.indexOf('|', p4 + 1);
      int p6 = geoPayload.indexOf('|', p5 + 1);
      
      // Nếu tìm thấy đầy đủ các dấu '|'
      if (p6 != -1) {
        // Cắt chuỗi lấy dữ liệu
        myIP = geoPayload.substring(0, p1);
        myLon = geoPayload.substring(p5 + 1, p6).toFloat();
        myLat = geoPayload.substring(p6 + 1).toFloat();
        
        // Thay định dạng link cũ bằng link này
        mapLink = "/maps/place/" + String(myLat, 6) + "," + String(myLon, 6);
        
        Serial.println("=> IPv4: " + myIP);
        Serial.println("=> Vĩ độ (Lat): " + String(myLat, 6));
        Serial.println("=> Kinh độ (Lon): " + String(myLon, 6));
        Serial.println("=> Google Maps Link: " + mapLink);
        
        // Gửi thông tin lên Blynk
        Blynk.virtualWrite(V1, myIP);
        Blynk.virtualWrite(V2, mapLink);
        
        // Bắt buộc đóng kết nối API 1 trước khi mở API 2
        http.end(); 
        
        // ==========================================
        // 2. LẤY THỜI TIẾT TỪ OPENWEATHERMAP (XỬ LÝ JSON)
        // ==========================================
        Serial.println("\n--- Đang gọi API OpenWeatherMap ---");
        String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(myLat, 6) + "&lon=" + String(myLon, 6) + "&appid=" + openWeatherApiKey + "&units=metric";
        
        http.begin(client, weatherUrl);
        int httpCodeWeather = http.GET();
        
        if (httpCodeWeather == HTTP_CODE_OK) {
          String weatherPayload = http.getString();
          
          // --- BẮT ĐẦU XỬ LÝ JSON ---
          JsonDocument weatherDoc;
          DeserializationError errWeather = deserializeJson(weatherDoc, weatherPayload);
          
          if(!errWeather) {
            float temp = weatherDoc["main"]["temp"];
            Serial.println("=> Nhiệt độ: " + String(temp) + "°C");
            
            // Gửi dữ liệu nhiệt độ lên Blynk
            Blynk.virtualWrite(V3, temp);
          } else {
            Serial.println("Lỗi Parse JSON (Weather): " + String(errWeather.c_str()));
          }
        } else {
          Serial.println("Lỗi gọi OpenWeatherMap. Mã lỗi: " + String(httpCodeWeather));
        }
      } else {
        Serial.println("Lỗi: Cấu trúc chuỗi trả về không đúng định dạng!");
      }
    } else {
      Serial.println("Lỗi gọi API iothings. Mã lỗi: " + String(httpCodeGeo));
    }
    http.end(); // Đảm bảo đóng kết nối HTTP cuối cùng
  } else {
    Serial.println("Lỗi: Mất kết nối WiFi.");
  }
}

void setup() {
  Serial.begin(115200);
  
  Serial.println("Đang kết nối WiFi và Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Gọi API lần đầu ngay khi khởi động
  fetchApiData();
  
  // Thiết lập Timer
  timer.setInterval(1000L, sendUptime);       // Cập nhật Uptime mỗi 1 giây
  timer.setInterval(300000L, fetchApiData);   // Cập nhật API mỗi 5 phút
}

void loop() {
  Blynk.run();
  timer.run();
}