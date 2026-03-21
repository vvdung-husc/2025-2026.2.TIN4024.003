#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6XhIYVWXZ"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "API KEY"
//wVRepimK-oqZePwd70K-a37hUvNmCDBl
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <stdarg.h>

// ========================= WIFI =========================
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL  6

// ========================= OPENWEATHER =========================
#define OPENWEATHERMAP_KEY "YOUR_API_KEY"
// 134b458bdf220c83c8d0c5b7fea38bdd

// ========================= BLYNK VIRTUAL PIN =========================
// V0: Thời gian hoạt động
// V1: Địa chỉ IPv4
// V2: Link Google Maps
// V3: Nhiệt độ
#define VPIN_UPTIME   V0
#define VPIN_IP       V1
#define VPIN_MAPS     V2
#define VPIN_TEMP     V3

// ========================= STRUCT =========================
struct IP4_Info {
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;
String urlWeather = "";

unsigned long currentMilliseconds = 0;

// =========================================================
// Hàm kiểm tra đến thời điểm thực hiện chưa
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (currentMilliseconds - ulTimer < millisecond) return false;
  ulTimer = currentMilliseconds;
  return true;
}

// =========================================================
// Hàm format chuỗi giống printf
String StringFormat(const char *fmt, ...) {
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

  int size = len + 1;
  char *buffer = (char *)malloc(size);
  if (!buffer) {
    va_end(args);
    return "";
  }

  vsnprintf(buffer, size, fmt, args);
  va_end(args);

  String result = buffer;
  free(buffer);
  return result;
}

// =========================================================
// Parse dữ liệu từ API: http://ip4.iothings.vn/?geo=1
// Dạng dữ liệu thường là:
// ip|country_code|country|region|city|longitude|latitude
bool parseGeoInfo(String payload, IP4_Info &ipInfo) {
  payload.trim();
  if (payload.length() == 0) return false;

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

  if (index < 7) {
    Serial.println("parseGeoInfo() => Du lieu khong du 7 truong");
    return false;
  }

  ipInfo.ip4 = values[0];
  ipInfo.longitude = values[5];
  ipInfo.latitude = values[6];

  Serial.println("========== GEO INFO ==========");
  Serial.printf("IP Address   : %s\n", values[0].c_str());
  Serial.printf("Country Code : %s\n", values[1].c_str());
  Serial.printf("Country      : %s\n", values[2].c_str());
  Serial.printf("Region       : %s\n", values[3].c_str());
  Serial.printf("City         : %s\n", values[4].c_str());
  Serial.printf("Longitude    : %s\n", values[5].c_str());
  Serial.printf("Latitude     : %s\n", values[6].c_str());
  Serial.println("==============================");

  return true;
}

// =========================================================
// Gọi API lấy IPv4, latitude, longitude
void getGeoAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("getGeoAPI() => WiFi chua ket noi");
    return;
  }

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  Serial.println("\nDang goi API lay IPv4 + GeoInfo...");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();

    Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
    Serial.println("Payload:");
    Serial.println(response);

    bool ok = parseGeoInfo(response, ip4Info);
    if (!ok) {
      Serial.println("Khong the phan tich du lieu geo");
      http.end();
      return;
    }

    String urlGoogleMaps = StringFormat(
      "https://www.google.com/maps/place/%s,%s",
      ip4Info.latitude.c_str(),
      ip4Info.longitude.c_str()
    );

    Serial.println("\n========== KET QUA ==========");
    Serial.printf("IPv4         : %s\n", ip4Info.ip4.c_str());
    Serial.printf("Latitude     : %s\n", ip4Info.latitude.c_str());
    Serial.printf("Longitude    : %s\n", ip4Info.longitude.c_str());
    Serial.printf("Google Maps  : %s\n", urlGoogleMaps.c_str());

    urlWeather = StringFormat(
      "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
      ip4Info.latitude.c_str(),
      ip4Info.longitude.c_str(),
      OPENWEATHERMAP_KEY
    );

    Serial.printf("OpenWeather  : %s\n", urlWeather.c_str());
    Serial.println("=============================\n");

  } else {
    Serial.print("getGeoAPI() => Error on sending GET: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// =========================================================
// Gửi IPv4 và Google Maps lên Blynk 1 lần
void sendInfoToBlynkOnce() {
  static bool done = false;
  if (done) return;

  if (ip4Info.ip4 == "" || ip4Info.latitude == "" || ip4Info.longitude == "") {
    return;
  }

  String link = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longitude.c_str()
  );

  Blynk.virtualWrite(VPIN_IP, ip4Info.ip4);
  Blynk.virtualWrite(VPIN_MAPS, link);

  Serial.println("Da gui IPv4 va Google Maps len Blynk");
  done = true;
}

// =========================================================
// Cập nhật nhiệt độ từ OpenWeatherMap
void updateTemperature() {
  static unsigned long lastTime = 0;
  static float lastTemp = -9999.0;

  if (!IsReady(lastTime, 10000)) return; // 10 giây
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("updateTemperature() => WiFi chua ket noi");
    return;
  }

  if (urlWeather == "") {
    Serial.println("updateTemperature() => urlWeather dang rong");
    return;
  }

  HTTPClient http;
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");

  Serial.println("Dang goi OpenWeatherMap...");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();

    Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
    Serial.println("Weather JSON:");
    Serial.println(response);

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    } else {
      if (!doc["main"]["temp"].isNull()) {
        float temp = doc["main"]["temp"].as<float>();
        Serial.print("Nhiet do hien tai: ");
        Serial.println(temp);

        if (temp != lastTemp) {
          lastTemp = temp;
          Blynk.virtualWrite(VPIN_TEMP, temp);
          Serial.println("Da cap nhat nhiet do len Blynk");
        }
      } else {
        Serial.println("Khong tim thay truong main.temp trong JSON");
      }
    }

  } else {
    Serial.print("updateTemperature() => Error on sending GET: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// =========================================================
// Cập nhật thời gian hoạt động lên Blynk mỗi 1 giây
void updateUptimeToBlynk() {
  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  unsigned long uptimeSeconds = currentMilliseconds / 1000;
  Blynk.virtualWrite(VPIN_UPTIME, uptimeSeconds);
}

// =========================================================
void connectWiFi() {
  Serial.print("Dang ket noi WiFi: ");
  Serial.println(WIFI_SSID);

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

// =========================================================
void connectBlynk() {
  Serial.println("Dang ket noi Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);

  if (Blynk.connect()) {
    Serial.println("Blynk connected!");
  } else {
    Serial.println("Khong ket noi duoc Blynk");
  }
}

// =========================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n===== ESP32 API HTTP GET + BLYNK =====");

  connectWiFi();
  connectBlynk();
  getGeoAPI();
}

// =========================================================
void loop() {
  Blynk.run();

  currentMilliseconds = millis();

  sendInfoToBlynkOnce();
  updateTemperature();
  updateUptimeToBlynk();
}