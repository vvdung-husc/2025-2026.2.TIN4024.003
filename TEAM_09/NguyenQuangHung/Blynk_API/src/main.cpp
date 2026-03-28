#define BLYNK_TEMPLATE_ID "TMPL65DqTYs6o"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "yB4rAcQmnGJ8wSOn5XlwcVaa0Ooc77iO"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

#define OPENWEATHERMAP_KEY "YOUR_KEY"

String ip4;
String lat;
String lon;
String weatherURL;

unsigned long lastTemp = 0;
unsigned long lastUptime = 0;


// ================= LẤY IP + LAT + LON =================
void getAPI()
{
  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");

  int code = http.GET();

  if (code > 0)
  {
    String res = http.getString();
    Serial.println(res);

    String data[7];
    int i = 0;

    while (res.length() && i < 7)
    {
      int p = res.indexOf('|');
      if (p == -1) { data[i++] = res; break; }

      data[i++] = res.substring(0, p);
      res = res.substring(p + 1);
    }

    ip4 = data[0];
    lon = data[5];
    lat = data[6];

    String mapLink = "https://www.google.com/maps/place/" + lat + "," + lon;

    Serial.println(ip4);
    Serial.println(mapLink);

    Blynk.virtualWrite(V1, ip4);
    Blynk.virtualWrite(V2, mapLink);

    weatherURL =
      "https://api.openweathermap.org/data/2.5/weather?lat=" +
      lat +
      "&lon=" +
      lon +
      "&appid=" +
      OPENWEATHERMAP_KEY +
      "&units=metric";
  }

  http.end();
}


// ================= LẤY NHIỆT ĐỘ =================
void updateTemp()
{
  if (millis() - lastTemp < 10000) return;
  lastTemp = millis();

  HTTPClient http;
  http.begin(weatherURL);

  int code = http.GET();

  if (code > 0)
  {
    String res = http.getString();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, res);

    float temp = doc["main"]["temp"];

    Serial.print("Temp: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3, temp);
  }

  http.end();
}


// ================= UPTIME =================
void uptime()
{
  if (millis() - lastUptime < 1000) return;
  lastUptime = millis();

  Blynk.virtualWrite(V0, millis() / 1000);
}


// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}


// ================= LOOP =================
void loop()
{
  Blynk.run();

  updateTemp();
  uptime();
}