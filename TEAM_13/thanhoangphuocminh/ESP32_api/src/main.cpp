#define BLYNK_TEMPLATE_ID "TMPL63GV3Kxsx"
#define BLYNK_TEMPLATE_NAME "ESP32APIPhuocMinh"
#define BLYNK_AUTH_TOKEN "DxokllqAs1EfY4UE8mxJkvDAfv-dZ5Uh"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   // Thư viện gọi API
#include <ArduinoJson.h>  // Thư viện xử lý JSON

// Nhớ đổi lại tên và pass WiFi nếu bạn chạy mạch thật nhé
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// Key lấy từ openweathermap.org
#define OPENWEATHERMAP_KEY "1c71d6f356feeac55a81d8010c8c321f"

// Cấu trúc lưu thông tin IPv4, lat, long từ http://ip4.iothings.vn/?geo=1
struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info; 
ulong currentMiliseconds = 0;
String urlWeather;

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

// Phân tích chuỗi trả về từ http://ip4.iothings.vn/?geo=1 và điền vào ipInfo
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
  ipInfo.latitude = values[6].c_str();
  ipInfo.longtitude = values[5].c_str();
  
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Country Code: %s\r\n", values[1].c_str());
  Serial.printf("Country: %s\r\n", values[2].c_str());
  Serial.printf("Region: %s\r\n", values[3].c_str());
  Serial.printf("City: %s\r\n", values[4].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
}

// API Get http://ip4.iothings.vn/?geo=1
void getAPI(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error in WiFi connection"); return;
  }
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();
  if(httpResponseCode > 0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
          
    parseGeoInfo(response, ip4Info);

    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n", ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps.c_str());

    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);

    Serial.printf("URL Weather => %s \r\n", urlWeather.c_str());      
  } else {
    Serial.print("Error on sending GET: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Cập nhật nhiệt độ từ urlWeather bằng API GET
void updateTemp(){
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return; // Cập nhật sau mỗi 10 giây
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("updateTemp() Error in WiFi connection"); 
    return;
  }

  HTTPClient http;   
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");
  int httpResponseCode = http.GET();
  
  if(httpResponseCode > 0){
    String response = http.getString();
          
    // Đã fix: Thay JsonDocument bằng DynamicJsonDocument để tránh lỗi với ArduinoJson cũ
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
    }
    else {
      float temp = doc["main"]["temp"]; // Lấy thông tin nhiệt độ
      
      if (temp_ != temp){ // Có thay đổi mới cập nhật lên Blynk
        temp_ = temp;
        Serial.print("Nhiet do: "); Serial.println(temp); 
        Blynk.virtualWrite(V3, temp_);
      }
    }
  } else {
    Serial.print("Error on getting Weather: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Chỉ gọi 1 lần để cập nhật IPv4, Link GoogleMaps
void onceCalled(){
  static bool done_ = false;
  if (done_) return;
  done_ = true;

  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
  
  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  
  Blynk.virtualWrite(V2, link.c_str());  
}

// Cập nhật uptime lên Blynk
void uptimeBlynk(){
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return; // Mỗi 1 giây
  ulong value = millis() / 1000; 
  Blynk.virtualWrite(V0, value);  
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  Blynk.connect();                

  getAPI();
}

void loop(void) {
  Blynk.run();  
  
  currentMiliseconds = millis();
  onceCalled(); 
  updateTemp();
  uptimeBlynk();
  // demo
}