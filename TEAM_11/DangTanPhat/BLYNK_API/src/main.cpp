#define BLYNK_TEMPLATE_ID "TMPL6j4-R01jR"
#define BLYNK_TEMPLATE_NAME "API"
#define BLYNK_AUTH_TOKEN "6EkM9QwnCl7WRgdvf3emxkv2ZrY6ttJq"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// OpenWeatherMap API key
const char* WEATHER_API_KEY = "dabf9fb1d2cfa050a5ed757642b51f94";

BlynkTimer timer;
unsigned long uptimeSeconds = 0;

// -------------------- KẾT NỐI WIFI --------------------
void connectWiFi() {
  Serial.print("Dang ket noi WiFi");
  WiFi.begin(ssid, pass);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nKhong ket noi duoc WiFi!");
  }
}

// -------------------- KẾT NỐI BLYNK --------------------
void connectBlynk() {
  IPAddress blynkIP(128, 199, 144, 129);
  Blynk.config(BLYNK_AUTH_TOKEN, blynkIP, 80);

  Serial.print("Dang ket noi Blynk");
  int retry = 0;
  while (!Blynk.connected() && retry < 20) {
    Blynk.connect(1000);
    Serial.print(".");
    retry++;
  }

  if (Blynk.connected()) {
    Serial.println("\nBlynk connected!");
  } else {
    Serial.println("\nKhong ket noi duoc Blynk!");
  }
}

// -------------------- HÀM GET HTTP --------------------
String httpGET(String url) {
  HTTPClient http;
  String payload = "";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    payload = http.getString();
  } else {
    Serial.print("HTTP GET loi: ");
    Serial.println(httpCode);
  }

  http.end();
  return payload;
}

// -------------------- LẤY IP PUBLIC --------------------
String getPublicIP() {
  String ip = httpGET("http://api.ipify.org");
  ip.trim();
  return ip;
}

// -------------------- LẤY LAT/LON QUA IP --------------------
bool getGeoLocation(float &lat, float &lon) {
  String payload = httpGET("http://ip-api.com/json/");

  if (payload.length() == 0) return false;

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("Loi parse JSON vi tri");
    return false;
  }

  if (doc["status"] != "success") {
    Serial.println("Khong lay duoc vi tri");
    return false;
  }

  lat = doc["lat"].as<float>();
  lon = doc["lon"].as<float>();
  return true;
}

// -------------------- LẤY THỜI TIẾT --------------------
bool getWeather(float lat, float lon, float &temp) {
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
               String(lat, 6) + "&lon=" + String(lon, 6) +
               "&appid=" + WEATHER_API_KEY + "&units=metric";

  String payload = httpGET(url);
  if (payload.length() == 0) return false;

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("Loi parse JSON thoi tiet");
    return false;
  }

  if (!doc["main"]["temp"].is<float>() && !doc["main"]["temp"].is<int>()) {
    Serial.println("Khong co du lieu nhiet do");
    return false;
  }

  temp = doc["main"]["temp"].as<float>();
  return true;
}

// -------------------- CẬP NHẬT THỜI GIAN --------------------
void updateTimeCounter() {
  uptimeSeconds = millis() / 1000;
  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, uptimeSeconds);
  }
}

// -------------------- CẬP NHẬT DỮ LIỆU MẠNG --------------------
void updateNetworkData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi mat ket noi, dang thu lai...");
    connectWiFi();
    return;
  }

  String publicIP = getPublicIP();
  if (publicIP.length() > 0) {
    Serial.print("Public IP: ");
    Serial.println(publicIP);

    if (Blynk.connected()) {
      Blynk.virtualWrite(V2, publicIP);
    }
  } else {
    Serial.println("Khong lay duoc Public IP");
  }

  float lat = 0, lon = 0;
  if (getGeoLocation(lat, lon)) {
    String googleMapsLink = "https://www.google.com/maps?q=" + String(lat, 6) + "," + String(lon, 6);

    Serial.print("Lat: ");
    Serial.println(lat, 6);
    Serial.print("Lon: ");
    Serial.println(lon, 6);
    Serial.print("Google Maps: ");
    Serial.println(googleMapsLink);

    if (Blynk.connected()) {
      Blynk.virtualWrite(V3, googleMapsLink);
    }

    float temperature = 0;
    if (getWeather(lat, lon, temperature)) {
      Serial.print("Nhiet do: ");
      Serial.print(temperature);
      Serial.println(" C");

      if (Blynk.connected()) {
        Blynk.virtualWrite(V1, temperature);
      }
    } else {
      Serial.println("Khong lay duoc du lieu thoi tiet");
    }
  } else {
    Serial.println("Khong lay duoc vi tri");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();
  connectBlynk();

  updateNetworkData();

  timer.setInterval(1000L, updateTimeCounter);     // V0: uptime
  timer.setInterval(30000L, updateNetworkData);    // V1, V2, V3
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  timer.run();
}