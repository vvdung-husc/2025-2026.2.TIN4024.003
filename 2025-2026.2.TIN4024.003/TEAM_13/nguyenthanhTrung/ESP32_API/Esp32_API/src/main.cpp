#define BLYNK_TEMPLATE_ID "TMPL6KcJtKm42"
#define BLYNK_TEMPLATE_NAME "Blynk thoitiet"
#define BLYNK_AUTH_TOKEN "ew_QjpViW86JiDPqRPHkyER9QpfLSmqP"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Thông tin WiFi mặc định của Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cấu hình API
const String apiKey = "cfbc576d3f962dfb4de4465a5c4954dc";
const String lat = "16.4591267";
const String lon = "107.5901477";

BlynkTimer timer;

// Virtual Pins định nghĩa theo ảnh:
// V1: Thời gian hoạt động (Uptime)
// V2: Địa chỉ IPv4
// V3: Google Maps Link
// V4: Nhiệt độ (Gauge)

void sendDataToBlynk() {
  // 1. Tính thời gian hoạt động (giây)
  long uptime = millis() / 1000;
  Blynk.virtualWrite(V1, uptime);

  // 2. Lấy IPv4 công cộng
  HTTPClient http;
  http.begin("http://api.ipify.org");
  int httpCode = http.GET();
  if (httpCode > 0) {
    String ip = http.getString();
    Blynk.virtualWrite(V2, ip);
  }
  http.end();

  // 3. Gửi link Google Maps
  String mapLink = "https://www.google.com/maps/place/" + lat + "," + lon;
  Blynk.virtualWrite(V3, mapLink);

  // 4. Lấy nhiệt độ từ OpenWeatherMap
  String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey + "&units=metric";
  http.begin(weatherUrl);
  int weatherCode = http.GET();
  if (weatherCode > 0) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, payload);
    float temp = doc["main"]["temp"];
    Blynk.virtualWrite(V4, temp);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  
  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Thiết lập gửi dữ liệu mỗi 10 giây để tránh overload API
  timer.setInterval(10000L, sendDataToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}