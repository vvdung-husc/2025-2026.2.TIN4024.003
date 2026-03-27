// ========================= BLYNK CONFIG =========================
#define BLYNK_TEMPLATE_ID "TMPL6q7N9b6-s"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "dmovmjdz2-ns9rizZ-HZM_axbAMWw3I1"

// ========================= INCLUDE =========================
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>
#include <stdarg.h>

// ========================= WIFI CONFIG =========================
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL  6

// ========================= OPENWEATHER =========================
// Thay API key thật của bạn vào đây
#define OPENWEATHERMAP_KEY "9dc3891bec15c42c054d4cd0e6277621"

// ========================= STRUCT =========================
struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

// ========================= GLOBAL =========================
IP4_Info ip4Info;
unsigned long currentMiliseconds = 0;
String urlWeather = "";

// ========================= TIMER HELPER =========================
bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// ========================= STRING FORMAT =========================
String StringFormat(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  va_list argsCopy;
  va_copy(argsCopy, args);
  int len = vsnprintf(nullptr, 0, fmt, argsCopy);
  va_end(argsCopy);

  if (len <= 0) {
    va_end(args);
    return "";
  }

  char* buffer = (char*)malloc(len + 1);
  if (buffer == nullptr) {
    va_end(args);
    return "";
  }

  vsnprintf(buffer, len + 1, fmt, args);
  va_end(args);

  String result = String(buffer);
  free(buffer);
  return result;
}

// ========================= PARSE GEO INFO =========================
// Dữ liệu từ: http://ip4.iothings.vn/?geo=1
// Format thường là:
// IP|CountryCode|Country|Region|City|Longitude|Latitude
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
  ipInfo.longtitude = values[5];
  ipInfo.latitude = values[6];

  Serial.println("===== GEO INFO =====");
  Serial.printf("IP Address   : %s\n", values[0].c_str());
  Serial.printf("Country Code : %s\n", values[1].c_str());
  Serial.printf("Country      : %s\n", values[2].c_str());
  Serial.printf("Region       : %s\n", values[3].c_str());
  Serial.printf("City         : %s\n", values[4].c_str());
  Serial.printf("Longitude    : %s\n", values[5].c_str());
  Serial.printf("Latitude     : %s\n", values[6].c_str());
  Serial.println("====================");
}

// ========================= GET GEO API =========================
void getAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error: WiFi not connected");
    return;
  }

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("Geo API code: %d\n", httpResponseCode);
    Serial.println(response);

    parseGeoInfo(response, ip4Info);

    String urlGoogleMaps = StringFormat(
      "https://www.google.com/maps/place/%s,%s",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str()
    );

    Serial.printf("IPv4 => %s\n", ip4Info.ip4.c_str());
    Serial.printf("Google Maps => %s\n", urlGoogleMaps.c_str());

    urlWeather = StringFormat(
      "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str(),
      OPENWEATHERMAP_KEY
    );

    Serial.printf("Weather URL => %s\n", urlWeather.c_str());
  } else {
    Serial.print("Error calling Geo API: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// ========================= UPDATE TEMP =========================
void updateTemp() {
  static unsigned long lastTime = 0;
  static float oldTemp = -9999.0;

  if (!IsReady(lastTime, 10000)) return;   // mỗi 10 giây
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("updateTemp() Error: WiFi not connected");
    return;
  }

  if (urlWeather.length() == 0) {
    Serial.println("updateTemp() Error: urlWeather is empty");
    return;
  }

  HTTPClient http;
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("Weather API code: %d\n", httpResponseCode);
    Serial.println(response);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
    } else {
      float temp = doc["main"]["temp"] | NAN;

      if (!isnan(temp)) {
        if (temp != oldTemp) {
          oldTemp = temp;
          Serial.print("Nhiet do: ");
          Serial.println(temp);

          Blynk.virtualWrite(V3, temp);
        }
      } else {
        Serial.println("Không đọc được nhiệt độ từ JSON");
      }
    }
  } else {
    Serial.print("Error calling Weather API: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// ========================= SEND IP + MAP ONCE =========================
void onceCalled() {
  static bool done = false;
  if (done) return;
  if (ip4Info.ip4.length() == 0) return;

  done = true;

  String link = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longtitude.c_str()
  );

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, link);
}

// ========================= UPTIME =========================
void uptimeBlynk() {
  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;   // mỗi 1 giây

  unsigned long seconds = lastTime / 1000;
  Blynk.virtualWrite(V0, seconds);
}

// ========================= WIFI CONNECT =========================
void connectWiFi() {
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

// ========================= BLYNK CONNECT =========================
void connectBlynk() {
  Blynk.config(BLYNK_AUTH_TOKEN);

  Serial.println("Connecting to Blynk...");
  if (Blynk.connect(10000)) {
    Serial.println("Blynk connected!");
  } else {
    Serial.println("Blynk connection failed!");
  }
}

// ========================= SETUP =========================
void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();
  connectBlynk();
  getAPI();
}

// ========================= LOOP =========================
void loop() {
  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();
  updateTemp();
  uptimeBlynk();
}