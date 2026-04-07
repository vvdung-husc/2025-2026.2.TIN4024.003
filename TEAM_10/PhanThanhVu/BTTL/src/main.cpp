#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPLxxxxxxx"
#define BLYNK_TEMPLATE_NAME "ESP32 DHT OLED"
#define BLYNK_TEMPLATE_ID "TMPL6ZhQFzPmd"
#define BLYNK_TEMPLATE_NAME "BTL"
#define BLYNK_AUTH_TOKEN "BKQWzUOrW1IzaOCRB5H3nv09Fgkwa_Kn"
#include <WiFi.h>
#include <Wire.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define DHTPIN 16
#define DHTTYPE DHT22
#define LED_PIN 5

#define OLED_SDA 13
#define OLED_SCL 12
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
BlynkTimer timer;

float temperature = 0;
float humidity = 0;
bool ledState = false;

void updateOLED(float t, float h) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ESP32 Wokwi + Blynk");

  display.setCursor(0, 18);
  display.print("Temp: ");
  display.print(t, 1);
  display.println(" C");

  display.setCursor(0, 32);
  display.print("Humi: ");
  display.print(h, 1);
  display.println(" %");

  display.setCursor(0, 46);
  display.print("LED : ");
  display.println(ledState ? "ON" : "OFF");

  display.display();
}

void sendSensorData() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Doc DHT loi!");
    return;
  }

  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" C | Do am: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, ledState);

  updateOLED(temperature, humidity);
}

BLYNK_WRITE(V3) {
  int value = param.asInt();
  ledState = value;
  digitalWrite(LED_PIN, ledState);
  Blynk.virtualWrite(V2, ledState);
  updateOLED(temperature, humidity);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED khong hoat dong!");
    while (1);
  }

  display.clearDisplay();
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Dang ket noi...");
  display.display();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}