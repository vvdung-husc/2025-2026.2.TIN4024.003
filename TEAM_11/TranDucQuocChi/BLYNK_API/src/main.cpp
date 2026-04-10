#define BLYNK_TEMPLATE_ID "TMPL6gu_-ytmm"
#define BLYNK_TEMPLATE_NAME "BLYNKIPA"
#define BLYNK_AUTH_TOKEN "qjVx7gvWW6K9APiM1kv7hPvgI9m5WiRx"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; // Đổi thành WiFi nhà bạn nếu dùng board thật
char pass[] = "";            // Nhập mật khẩu nếu dùng board thật

// API Thời tiết (Đang lấy ở Huế)
String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=Hue&appid=90bb375f8e5ad8245b926c458bef3a12";

BlynkTimer timer;

// Hàm 1: Gửi các thông số hệ thống (Thời gian HĐ & IP)
void sendSystemData() {
  // V0: Thời gian hoạt động (giây)
  Blynk.virtualWrite(V0, millis() / 1000);
  
  // V1: Địa chỉ IPv4
  Blynk.virtualWrite(V1, WiFi.localIP().toString());
}

// Hàm 2: Gọi API lấy Nhiệt độ và Tọa độ bản đồ
void getWeatherAndLocation() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(weatherUrl);
    int responseCode = http.GET();

    if (responseCode > 0) {
      String responseBody = http.getString();
      JSONVar weatherJson = JSON.parse(responseBody);

      if (JSON.typeof(weatherJson) != "undefined") {
        // 1. Lấy nhiệt độ -> Gửi vào V3
        double temp = (double)weatherJson["main"]["temp"] - 273.15;
        Blynk.virtualWrite(V3, temp);

        // 2. Lấy tọa độ Kinh/Vĩ độ để tạo link Google Maps -> Gửi vào V2
        double lat = (double)weatherJson["coord"]["lat"];
        double lon = (double)weatherJson["coord"]["lon"];
        String mapLink = "https://www.google.com/maps/place/" + String(lat, 6) + "," + String(lon, 6);
        Blynk.virtualWrite(V2, mapLink);

        Serial.println("--- Đã cập nhật API lên Blynk ---");
        Serial.println("Nhiệt độ: " + String(temp) + "°C");
        Serial.println("Bản đồ: " + mapLink);
      }
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);

  // --- BÊ NGUYÊN CÁCH KẾT NỐI BÀI TRƯỚC VÀO ĐÂY ---
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Dùng IP tĩnh của server Blynk để tránh lỗi DNS Failed trên Wokwi
  Blynk.config(auth, "128.199.144.129", 80);
  Blynk.connect();
  // ------------------------------------------------

  // Cài đặt Timer chạy định kỳ
  timer.setInterval(1000L, sendSystemData);         // Cập nhật thời gian & IP mỗi 1 giây
  timer.setInterval(10000L, getWeatherAndLocation); // Gọi API cập nhật thời tiết mỗi 10 giây
}

void loop() {
  Blynk.run();
  timer.run();
}