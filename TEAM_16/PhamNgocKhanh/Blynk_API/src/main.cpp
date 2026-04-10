// ==== BLYNK CONFIG ====
#define BLYNK_TEMPLATE_ID "TMPL66shGm2cm"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "wmN6cnuumJjxycivzZxcLJHE3-ORPrBf"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// ==== WIFI (Wokwi) ====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ==== API KEY ====
String weatherKey = "1f281c9fcb544be9d451ede7115ced03";

// ==== TIMER ====
BlynkTimer myTimer;

// ==== DATA ====
String currentIP;
float latitude = 0;
float longitude = 0;

// =====================================
// Gửi uptime
// =====================================
void updateUptime() {
  unsigned long t = millis() / 1000;
  Blynk.virtualWrite(V0, t);
}

// =====================================
// Hiển thị tên khi connect
// =====================================
BLYNK_CONNECTED() {
  Blynk.virtualWrite(V4, "TEAM 16 - ESP32");
}

// =====================================
// Lấy dữ liệu vị trí
// =====================================
bool getLocation() {
  HTTPClient http;
  WiFiClient client;

  http.begin(client, "http://ip4.iothings.vn/?geo=1");
  int code = http.GET();

  if (code != 200) {
    Serial.println("Loi goi API iothings!");
    http.end();
    return false;
  }

  String data = http.getString();
  http.end();

  Serial.println("RAW: " + data);

  // tách chuỗi
  int index = 0;
  int last = 0;
  int field = 0;

  while (index != -1) {
    index = data.indexOf('|', last);

    String part;
    if (index == -1) part = data.substring(last);
    else part = data.substring(last, index);

    if (field == 0) currentIP = part;
    if (field == 5) longitude = part.toFloat();
    if (field == 6) latitude = part.toFloat();

    last = index + 1;
    field++;
  }

  // kiểm tra dữ liệu hợp lệ
  if (latitude == 0 || longitude == 0) {
    Serial.println("Toa do khong hop le!");
    return false;
  }

  Serial.println("IP: " + currentIP);
  Serial.println("Lat: " + String(latitude));
  Serial.println("Lon: " + String(longitude));

  Blynk.virtualWrite(V1, currentIP);

  String link = "https://www.google.com/maps/place/" + String(latitude,6) + "," + String(longitude,6);
  Blynk.virtualWrite(V2, link);

  Serial.println("Lay vi tri thanh cong!");
  return true;
}

// =====================================
// Lấy thời tiết
// =====================================
void getWeather() {
  HTTPClient http;
  WiFiClient client;

  String url = "http://api.openweathermap.org/data/2.5/weather?";
  url += "lat=" + String(latitude,6);
  url += "&lon=" + String(longitude,6);
  url += "&appid=" + weatherKey;
  url += "&units=metric";

  http.begin(client, url);
  int code = http.GET();

  if (code != 200) {
    Serial.println("Loi lay weather!");
    http.end();
    return;
  }

  String json = http.getString();
  http.end();

  DynamicJsonDocument doc(2048);
  DeserializationError err = deserializeJson(doc, json);

  // kiểm tra lỗi JSON
  if (err) {
    Serial.println("JSON loi!");
    return;
  }

  float temp = doc["main"]["temp"];
  float hum  = doc["main"]["humidity"];

  Serial.println("Temp: " + String(temp));
  Serial.println("Hum: " + String(hum));

  Blynk.virtualWrite(V3, temp);
  Blynk.virtualWrite(V5, hum);
}

// =====================================
// Gọi toàn bộ API
// =====================================
void updateAll() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi mat ket noi!");
    return;
  }

  if (getLocation()) {
    getWeather();
  }
}

// =====================================
// SETUP
// =====================================
void setup() {
  Serial.begin(115200);

  Serial.println("Connecting...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  updateAll();

  myTimer.setInterval(1000L, updateUptime);
  myTimer.setInterval(180000L, updateAll);
}

// =====================================
// LOOP
// =====================================
void loop() {
  Blynk.run();
  myTimer.run();
}