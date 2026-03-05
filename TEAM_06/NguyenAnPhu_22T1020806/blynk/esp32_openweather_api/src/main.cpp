// Blynk setup — fill in your details
#define BLYNK_TEMPLATE_ID "TMPL6pDofs4kI"
#define BLYNK_TEMPLATE_NAME "esp32 openweather api"
#define BLYNK_AUTH_TOKEN "TSGxSA8wmFATNYA9D-kJ9tyIkZjkmPGW"

#include <WiFi.h>
#include <HTTPClient.h>
//
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>



#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ──────────────────────────────────────────────


// WiFi credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// OpenWeather API key
const char* apiKey = "0f7464550ddda6d793b3b22e0afb55d2";

// ──────────────────────────────────────────────
// URLs
const char* geoUrl      = "http://ip4.iothings.vn/?geo=1";
const char* weatherBase = "https://api.openweathermap.org/data/2.5/weather?";

// ──────────────────────────────────────────────
// Display setups (all I2C)
Adafruit_SSD1306 oledTime(128, 64, &Wire, -1);     // 0x3C - Time elapsed
Adafruit_SSD1306 oledTemp(128, 64, &Wire, -1);     // 0x3D - Temperature
Adafruit_SSD1306 oledIP(128, 64, &Wire, -1);       // 0x3E - IPv4 address

//declare functions
void showStartupMessage();
void fetchIPAndGeo();
void fetchWeather();
void updateTempDisplay();
void updateTimeDisplay();


// Variables
String publicIP = "";
String latitude  = "";
String longitude = "";
float temperature = 0.0;
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherInterval = 300000;  // 5 minutes

void setup() {
  Serial.begin(115200);
  delay(100);

  // Connect WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Start Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connectWiFi(ssid, pass);
  Blynk.connect();

  // Initialize all three OLEDs
  if (!oledTime.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed (Time)"));
  }
  if (!oledTemp.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println(F("SSD1306 allocation failed (Temp)"));
  }
  if (!oledIP.begin(SSD1306_SWITCHCAPVCC, 0x3E)) {
    Serial.println(F("SSD1306 allocation failed (IP)"));
  }

  oledTime.clearDisplay();   oledTime.setTextSize(2); oledTime.setTextColor(SSD1306_WHITE);
  oledTemp.clearDisplay();   oledTemp.setTextSize(2); oledTemp.setTextColor(SSD1306_WHITE);
  oledIP.clearDisplay();     oledIP.setTextSize(1);   oledIP.setTextColor(SSD1306_WHITE);

  showStartupMessage();

  // Fetch IP + geo once
  fetchIPAndGeo();

  // First weather fetch
  fetchWeather();
}

void loop() {
  Blynk.run();

  // Update time every second
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= 1000) {
    updateTimeDisplay();
    lastTime = millis();
  }

  // Refresh weather & temp display every 5 min
  if (millis() - lastWeatherUpdate >= weatherInterval) {
    fetchWeather();
    updateTempDisplay();
    lastWeatherUpdate = millis();
  }

  delay(10);  // helps Wokwi simulation
}

// ──────────────────────────────────────────────
// Show startup splash on all screens
void showStartupMessage() {
  const char* msg = "Starting...";

  oledTime.clearDisplay();
  oledTime.setCursor(10, 25);
  oledTime.print(msg);
  oledTime.display();

  oledTemp.clearDisplay();
  oledTemp.setCursor(10, 25);
  oledTemp.print(msg);
  oledTemp.display();

  oledIP.clearDisplay();
  oledIP.setCursor(10, 25);
  oledIP.print(msg);
  oledIP.display();

  delay(1500);
}

// ──────────────────────────────────────────────
// Fetch public IP + lat/lon from ip4.iothings.vn
void fetchIPAndGeo() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(geoUrl);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String payload = http.getString();

    // Format: IP|country_short|country_full|province|city|lon|lat
    int p1 = payload.indexOf('|');
    int p2 = payload.indexOf('|', p1 + 1);
    int p3 = payload.indexOf('|', p2 + 1);
    int p4 = payload.indexOf('|', p3 + 1);
    int p5 = payload.indexOf('|', p4 + 1);
    int p6 = payload.indexOf('|', p5 + 1);

    if (p6 > 0) {
      publicIP   = payload.substring(0, p1);
      longitude  = payload.substring(p5 + 1, p6);
      latitude   = payload.substring(p6 + 1);

      Blynk.virtualWrite(V1, publicIP);
      String mapUrl = "https://www.google.com/maps/@" + latitude + "," + longitude + ",15z";
      Blynk.virtualWrite(V2, mapUrl);

      Serial.println("IP: " + publicIP);
      Serial.println("Lat: " + latitude + "  Lon: " + longitude);

      updateIPDisplay();
    }
  } else {
    Serial.printf("Geo HTTP error: %d\n", code);
  }
  http.end();
}

// ──────────────────────────────────────────────
// Fetch temperature from OpenWeather
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED || latitude == "" || longitude == "") return;

  String url = String(weatherBase) + "lat=" + latitude + "&lon=" + longitude
               + "&appid=" + apiKey + "&units=metric";

  HTTPClient http;
  http.begin(url);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      temperature = doc["main"]["temp"].as<float>();

      Blynk.virtualWrite(V3, temperature);
      Serial.printf("Temperature: %.1f °C\n", temperature);

      updateTempDisplay();
    } else {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.printf("Weather HTTP error: %d\n", code);
  }
  http.end();
}

// ──────────────────────────────────────────────
// Update OLED displays
void updateTimeDisplay() {
  unsigned long secs = millis() / 1000;

  oledTime.clearDisplay();
  oledTime.setCursor(8, 10);
  oledTime.setTextSize(2);
  oledTime.print("Time");
  oledTime.setCursor(8, 38);
  oledTime.print(secs);
  oledTime.print(" s");
  oledTime.display();

  Blynk.virtualWrite(V0, (int)secs);
}

void updateTempDisplay() {
  oledTemp.clearDisplay();
  oledTemp.setCursor(8, 10);
  oledTemp.setTextSize(2);
  oledTemp.print("Temp");
  oledTemp.setCursor(12, 38);
  oledTemp.print(temperature, 1);
  oledTemp.print(" C");
  oledTemp.display();
}

void updateIPDisplay() {
  oledIP.clearDisplay();
  oledIP.setTextSize(1);
  oledIP.setCursor(0, 0);
  oledIP.println("Public IPv4:");
  oledIP.setTextSize(2);
  oledIP.setCursor(4, 24);

  // If IP is very long, it may wrap — but usually fits
  oledIP.println(publicIP);
  oledIP.display();
}