#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6jRRtrMkn"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "RrBHwaM542YwWZu7lSm1f5vSewA6EsAI"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

#define OPENWEATHERMAP_KEY ""

struct IP4_Info{
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;

ulong currentMiliseconds = 0;
String urlWeather = "";

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

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
  return s;
}

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

  urlWeather = StringFormat(
    "http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
    ipInfo.latitude.c_str(),
    ipInfo.longtitude.c_str(),
    OPENWEATHERMAP_KEY
  );
}

void getAPI(){
  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpResponseCode = http.GET();
  if(httpResponseCode == 200){
    String response = http.getString();
    parseGeoInfo(response, ip4Info);
  }
  http.end();
}

void updateTemp(){
  static ulong lastTime = 0;

  if (!IsReady(lastTime, 10000)) return;
  if(WiFi.status() != WL_CONNECTED) return;
  if(urlWeather == "") return;

  HTTPClient http;   
  http.begin(urlWeather);

  int httpResponseCode = http.GET();
  if(httpResponseCode == 200){
    String response = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      float temp = doc["main"]["temp"] | 0.0;
      Blynk.virtualWrite(V3, temp);
    }
  }
  http.end();
}

void onceCalled(){
  static bool done_ = false;
  if (done_) return;
  done_ = true;

  String link = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longtitude.c_str()
  );

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, link);
}

void uptimeBlynk(){
  static ulong lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  ulong value = millis() / 1000;
  Blynk.virtualWrite(V0, value);
}

void setup(void) {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

void loop(void) {

  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();
  updateTemp();
  uptimeBlynk();

}