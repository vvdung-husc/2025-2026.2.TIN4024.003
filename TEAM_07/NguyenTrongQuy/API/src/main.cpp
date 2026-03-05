#include <Arduino.h>

/* Blynk */
#define BLYNK_TEMPLATE_ID "TMPL6q7N9b6-s"
#define BLYNK_TEMPLATE_NAME "API"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"   // <-- thay khi chạy thật

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

/* ===================== STRUCT ===================== */

struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;

unsigned long currentMiliseconds = 0;

/* ===================== TIMER ===================== */

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

/* ===================== FORMAT STRING ===================== */

String StringFormat(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  char* buffer = (char*)malloc(len + 1);

  va_start(args, fmt);
  vsnprintf(buffer, len + 1, fmt, args);
  va_end(args);

  String result = buffer;
  free(buffer);

  return result;
}

/* ===================== GEO PARSE ===================== */

void parseGeoInfo(String payload, IP4_Info& ipInfo) {

  String values[7];
  int index = 0;

  while (payload.length() > 0 && index < 7) {

    int pos = payload.indexOf('|');

    if (pos == -1) {
      values[index++] = payload;
      break;
    }

    values[index++] = payload.substring(0, pos);
    payload = payload.substring(pos + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.longtitude = values[5];
  ipInfo.latitude = values[6];

  Serial.printf("IP: %s\n", values[0].c_str());
  Serial.printf("City: %s\n", values[4].c_str());
  Serial.printf("Longitude: %s\n", values[5].c_str());
  Serial.printf("Latitude: %s\n", values[6].c_str());
}

/* ===================== WEATHER ===================== */

#define OPENWEATHERMAP_KEY "YOUR_OPENWEATHER_API_KEY"

String urlWeather;

/* ===================== GET GEO ===================== */

void getAPI() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi error");
    return;
  }

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  if (httpCode > 0) {

    String response = http.getString();

    parseGeoInfo(response, ip4Info);

    String googleMaps = StringFormat(
      "https://www.google.com/maps/place/%s,%s",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str()
    );

    Serial.println(googleMaps);

    urlWeather = StringFormat(
      "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str(),
      OPENWEATHERMAP_KEY
    );

    Serial.println(urlWeather);
  }

  http.end();
}

/* ===================== GET TEMP ===================== */

void updateTemp() {

  static unsigned long lastTime = 0;
  static float temp_ = -1000;

  if (!IsReady(lastTime, 10000)) return;  // 10s

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi error");
    return;
  }

  HTTPClient http;

  http.begin(urlWeather);

  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();

    StaticJsonDocument<1024> doc;

    if (deserializeJson(doc, payload) == DeserializationError::Ok) {

      float temp = doc["main"]["temp"];

      if (temp != temp_) {

        temp_ = temp;

        Serial.print("Temperature: ");
        Serial.println(temp_);

        Blynk.virtualWrite(V3, temp_);
      }
    }
  }

  http.end();
}

/* ===================== BLYNK ===================== */

void onceCalled() {

  static bool done = false;
  if (done) return;

  done = true;

  String link = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longtitude.c_str()
  );

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, link);
}

void uptimeBlynk() {

  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  Blynk.virtualWrite(V0, lastTime / 1000);
}

/* ===================== SETUP ===================== */

void setup() {

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

/* ===================== LOOP ===================== */

void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();
  updateTemp();
  uptimeBlynk();
}