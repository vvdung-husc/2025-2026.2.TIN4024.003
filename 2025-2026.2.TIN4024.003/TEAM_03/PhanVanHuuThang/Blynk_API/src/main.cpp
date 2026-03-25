
#include <Arduino.h>

//Thay thông số BLYNK của bạn vào đây
#define BLYNK_TEMPLATE_ID "TMPL6Rq3zXoK9"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "Vt8ecAZ-g6i9sXSwSjZ8bSA6_DvheUNI"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>   //Thư viện gọi API
#include <ArduinoJson.h>  //Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

//Cấu trúc lưu thông tin IPv4, lat, long từ http://ip4.iothings.vn/?geo=1
struct IP4_Info{
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info; //Biến lưu trữ cấu trúc nhận được từ GET http://ip4.iothings.vn/?geo=1

ulong currentMiliseconds = 0; //Thời gian hiện tại - miliseconds 

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

//Định dạng chuỗi %s,%s,...
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

//Phân tích chuỗi trả về từ http://ip4.iothings.vn/?geo=1 và điền vào ipInfo
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

//Key lấy từ openweathermap.org khi đăng ký tài khoản
#define OPENWEATHERMAP_KEY "YOUR_OPENWEATHERMAP_KEY_HERE" //Thay KEY của bạn vào đây
String urlWeather;  //Biến lưu url https://openweathermap.org/

//API Get http://ip4.iothings.vn/?geo=1
void getAPI(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error in WiFi connection"); return;
  }
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();
  if(httpResponseCode>0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
          
    parseGeoInfo(response, ip4Info);

    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s",ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n",ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps.c_str());

    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",ip4Info.latitude.c_str(),ip4Info.longtitude.c_str(),OPENWEATHERMAP_KEY);

    Serial.printf("URL => %s \r\n",urlWeather.c_str());      
  }else{
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//Cập nhật nhiêt độ từ urlWeather bằng API GET
void updateTemp(){
  static ulong lastTime = 0;

  if (!IsReady(lastTime, 10000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 10 giây
  
  if(urlWeather.length() == 0){
    Serial.println("[updateTemp] urlWeather chua duoc khoi tao!"); 
    return;
  }
  
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("[updateTemp] WiFi mat ket noi!"); 
    return;
  }

  if(!Blynk.connected()){
    Serial.println("[updateTemp] Blynk chua ket noi!");
    return;
  }

  Serial.println("[updateTemp] Dang goi API Weather...");
  HTTPClient http;   
  http.begin(urlWeather);
  int httpResponseCode = http.GET();
  Serial.printf("[updateTemp] HTTP code: %d\r\n", httpResponseCode);
  if(httpResponseCode == 200){
    String response = http.getString();
    Serial.println(response);
          
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.println("[updateTemp] JSON parse that bai!");
    } else {
      float temp = doc["main"]["temp"];
      Serial.printf("[updateTemp] Nhiet do: %.2f C\r\n", temp);
      Blynk.virtualWrite(V3, temp); // Luon gui len Blynk
      Serial.println("[updateTemp] Da gui len Blynk V3 thanh cong!");
    }
  } else {
    String response = http.getString();
    Serial.printf("[updateTemp] Loi API: %s\r\n", response.c_str());
  }
  http.end();
}

//Chỉ gọi 1 lần để cập nhật IPv4, Link GoogleMaps của Latitude, Longtitude
void onceCalled(){
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  String link = StringFormat("https://www.google.com/maps/place/%s,%s",ip4Info.latitude.c_str(),ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  //Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
  Blynk.virtualWrite(V2, link.c_str());  //Gửi giá trị lên chân ảo V2 trên ứng dụng Blynk.
}

//Cập nhật uptime lên BlynkBlynk
void uptimeBlynk(){
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  //Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk.
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

  // Cấu hình DNS server (Google DNS)
  IPAddress dns1(8, 8, 8, 8);
  IPAddress dns2(8, 8, 4, 4);
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns1, dns2);
  
  // Chờ DNS sẵn sàng
  delay(2000);
  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("DNS Server: ");
  Serial.println(WiFi.dnsIP());

  Blynk.config(BLYNK_AUTH_TOKEN); // Cấu hình Blynk với mã token
  Blynk.connect();                // Kết nối Blynk
  delay(1000);
  
  // Gửi giá trị test để xác nhận V3 hoạt động
  Blynk.virtualWrite(V3, 99.9);
  Serial.println("[setup] Da gui test value 99.9 len V3");

  getAPI();

}

void loop(void) {
  //return; //commnet để chạy vòng lặp
  
  Blynk.run();  // Chạy vòng lặp Blynk
  
  currentMiliseconds = millis();
  onceCalled(); 
  updateTemp();
  uptimeBlynk();

}