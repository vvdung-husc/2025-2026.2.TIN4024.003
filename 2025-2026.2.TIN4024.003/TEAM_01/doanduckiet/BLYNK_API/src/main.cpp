#define BLYNK_PRINT Serial
#include <Arduino.h>

// Thay thông số BLYNK của bạn vào đây
#define BLYNK_TEMPLATE_ID "TMPL6tTcbQoV9"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "qe0NQ6YOKrUwBnOCovaHewLSVFy4E3jW"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   // Thư viện gọi API
#include <ArduinoJson.h>  // Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// Cấu trúc lưu thông tin IPv4, lat, long từ http://ip4.iothings.vn/?geo=1
struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info; // Biến lưu trữ cấu trúc nhận được

ulong currentMiliseconds = 0; // Thời gian hiện tại - miliseconds 

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Định dạng chuỗi %s,%s,...
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Phân tích chuỗi trả về từ http://ip4.iothings.vn/?geo=1
void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  String values[7];
  int index = 0;
  
  while (payload.length() > 0 && index < 7) {
      int delimiterIndex = payload.indexOf('|');
      
      if (delimiterIndex == -1) {
          values[index++] = payload;
          break;
      }
      
      values[index++] = payload.substring(0, delimiterIndex);
      payload = payload.substring(delimiterIndex + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.latitude = values[6];
  ipInfo.longtitude = values[5];
  
  Serial.println("--- Thông tin Địa lý ---");
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Country Code: %s\r\n", values[1].c_str());
  Serial.printf("Country: %s\r\n", values[2].c_str());
  Serial.printf("Region: %s\r\n", values[3].c_str());
  Serial.printf("City: %s\r\n", values[4].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
  Serial.println("------------------------");
}

// Key lấy từ openweathermap.org khi đăng ký tài khoản
#define OPENWEATHERMAP_KEY "f876dda283cffcf4dda851c396b4f518" // Đừng quên thay KEY của bạn vào đây
String urlWeather;  // Biến lưu url thời tiết

// Gọi API lấy IPv4 và tọa độ
void getAPI(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error: Mất kết nối WiFi"); 
    return;
  }
  
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();
  if(httpResponseCode > 0){
    String response = http.getString();
          
    parseGeoInfo(response, ip4Info);

    // [ĐÃ SỬA] Đưa định dạng %s, %s vào link Google Maps
    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n", ip4Info.ip4.c_str());
    Serial.printf("Link Google Maps => %s\r\n", urlGooleMaps.c_str());

    // [ĐÃ SỬA] Xóa chữ 's' dư thừa sau %s ở tham số appid
    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);

    Serial.printf("URL Weather => %s \r\n", urlWeather.c_str());      
  } else {
    Serial.print("Lỗi khi gọi API Geo (GET): HTTP ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Cập nhật nhiệt độ từ OpenWeatherMap
void updateTemp(){
  static ulong lastTime = 0;
  static float temp_ = -999.0; // Khởi tạo một giá trị nhiệt độ vô lý để lần đầu chắc chắn sẽ cập nhật lên Blynk

  if (!IsReady(lastTime, 10000)) return; // Cập nhật sau mỗi 10 giây (bạn đang để 10000 ms)
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("updateTemp() Error: Mất kết nối WiFi"); 
    return;
  }

  HTTPClient http;   
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");
  
  int httpResponseCode = http.GET();
  if(httpResponseCode > 0){
    String response = http.getString();
          
    // Xử lý JSON trả về từ API
    JsonDocument doc; // Dành cho ArduinoJson v7
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
      Serial.print("Lỗi parse JSON Thời tiết: ");
      Serial.println(error.c_str());
    } else {
      float temp = doc["main"]["temp"]; // lấy thông tin nhiệt độ
      
      if (temp_ != temp){ // có thay đổi mới cập nhật lên Blynk
        temp_ = temp;
        Serial.print("Nhiệt độ hiện tại: "); Serial.print(temp); Serial.println(" °C"); 
        Blynk.virtualWrite(V3, temp_); // Giả sử V3 là chân ảo của nhiệt độ
      }
    }
  } else {
    Serial.print("Lỗi khi gọi API Thời tiết (GET): HTTP ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Chỉ gọi 1 lần để đẩy IPv4, Link GoogleMaps lên Blynk
void onceCalled(){
  static bool done_ = false;
  if (done_) return;
  
  // Đảm bảo dữ liệu đã được lấy trước khi gửi lên Blynk
  if (ip4Info.ip4 == "") return; 

  done_ = true;
  // [ĐÃ SỬA] Link bản đồ chính xác
  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  
  Blynk.virtualWrite(V2, link.c_str());  
}

// Cập nhật uptime lên Blynk
void uptimeBlynk(){
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return; // Mỗi 1 giây
  ulong value = millis() / 1000; // [ĐÃ SỬA] Dùng millis() trực tiếp cho Uptime
  Blynk.virtualWrite(V0, value); 
}

void setup(void) {
  Serial.begin(115200);
  
  // 1. Kết nối Wi-Fi như bình thường
  Serial.print("Đang kết nối Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Bỏ WIFI_CHANNEL đi
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối Wi-Fi thành công!");

  // 2. Áp dụng cách ép DNS THÀNH CÔNG TỪ CODE CỦA BẠN
  IPAddress googleDNS(8, 8, 8, 8);
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), googleDNS);
  Serial.println("Đã cập nhật DNS sang 8.8.8.8");

  // Đợi một chút cho mạng ổn định
  delay(1000);

  // 3. Kết nối vào máy chủ Blynk
  Serial.println("Đang kết nối với Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // 4. Gọi API lấy toạ độ và thời tiết
  // Lúc này ESP32 đã có DNS của Google, nó sẽ biên dịch được ip4.iothings.vn
  getAPI(); 
  Blynk.virtualWrite(V4, "Đoàn Đức Kiệt");
}

void loop(void) {
  // [ĐÃ XÓA] Lệnh return chặn vòng lặp
  
  if (Blynk.connected()) {
    Blynk.run();  
  }
  
  currentMiliseconds = millis();
  
  onceCalled(); 
  updateTemp();
  uptimeBlynk();
}