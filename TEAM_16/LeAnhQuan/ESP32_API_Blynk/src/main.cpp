#define BLYNK_TEMPLATE_ID "TMPL6uC7HYB6J"
#define BLYNK_TEMPLATE_NAME "APIIOT"
#define BLYNK_AUTH_TOKEN "wJL4njin_fgtZ9fjWMWlc8ytod8T2ewy"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


char ssid[] = "Wokwi-GUEST";
char pass[] = "";


String openWeatherMapApiKey = "06ba71d4f106f2d34adddc1a8cdb0e63";

BlynkTimer timer;

void fetchLocationAndWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    
    Serial.println("Đang lấy thông tin Vị trí...");
    http.begin("http://ip-api.com/json/"); 
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      const char* ip = doc["query"]; 
      float lat = doc["lat"];
      float lon = doc["lon"];

      if (ip) {
        String ipv4 = String(ip);
        String latitude = String(lat, 6);
        String longitude = String(lon, 6);
        
        String mapsLink = "http://googleusercontent.com/maps.google.com/maps?q=" + latitude + "," + longitude;

        
        Blynk.virtualWrite(V1, ipv4);
        Blynk.virtualWrite(V2, mapsLink);
        
        Serial.println("Đã lấy được Vị trí: " + ipv4 + " | Tọa độ: " + latitude + "," + longitude);

        
        String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + latitude + "&lon=" + longitude + "&units=metric&appid=" + openWeatherMapApiKey;
        http.begin(weatherUrl);
        int weatherCode = http.GET();
        
        if (weatherCode > 0) {
          String weatherPayload = http.getString();
          DynamicJsonDocument weatherDoc(2048); // Đã tăng bộ nhớ để không bị lỗi khởi động lại
          DeserializationError error = deserializeJson(weatherDoc, weatherPayload);
          
          if (!error) {
            float temperature = weatherDoc["main"]["temp"];
            
            
            Blynk.virtualWrite(V3, temperature);
            
            Serial.println("Nhiệt độ hiện tại: " + String(temperature) + " °C");
            Serial.println("Đã cập nhật tất cả dữ liệu lên Blynk thành công!");
          } else {
            Serial.print("Lỗi dịch JSON: ");
            Serial.println(error.c_str());
          }
        } else {
          Serial.println("Lỗi gọi API Thời tiết. Mã lỗi: " + String(weatherCode));
        }
        http.end(); 
      }
    } else {
      Serial.println("Lỗi gọi API Vị trí. Mã lỗi: " + String(httpCode));
    }
    http.end(); 
  } else {
    Serial.println("Mất kết nối WiFi!");
  }
}


void sendUptime() {
  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Đang kết nối WiFi và Blynk...");
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  
  fetchLocationAndWeather();


  timer.setInterval(1000L, sendUptime);
  timer.setInterval(30000L, fetchLocationAndWeather);
}

void loop() {
  Blynk.run();
  timer.run();
}