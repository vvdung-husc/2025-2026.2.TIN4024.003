/*
THÔNG TIN NHÓM X
1. Nguyễn Trọng Quý
2. Trần Quang Tiến
3. Trần Hưng Trường Vũ
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// ======================
// OLED CONFIG
// ======================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ======================
// DHT CONFIG
// ======================
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ======================
// LED PIN
// ======================
#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4

// ======================
// BLINK CONTROL
// ======================
unsigned long lastBlink = 0;
bool ledState = false;
const unsigned long blinkInterval = 1000;

// ======================
// STATUS CONTROL
// ======================
String lastStatus = "";

// ======================
// SETUP
// ======================
void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  dht.begin();

  Wire.begin(13, 12); // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

// ======================
// LED BLINK FUNCTION
// ======================
void blinkLed(int ledPin) {
  unsigned long now = millis();
  if (now - lastBlink >= blinkInterval) {
    lastBlink = now;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

// ======================
// LOOP
// ======================
void loop() {
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  if (isnan(temp) || isnan(humi)) {
    return;
  }

  // Reset LEDs
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  String statusText = "";
  int activeLed = -1;

  // ======================
  // TEMPERATURE LOGIC
  // ======================
  if (temp < 13) {
    statusText = "TOO COLD";
    activeLed = LED_GREEN;
  } else if (temp < 20) {
    statusText = "COLD";
    activeLed = LED_GREEN;
  } else if (temp < 25) {
    statusText = "COOL";
    activeLed = LED_YELLOW;
  } else if (temp < 30) {
    statusText = "WARM";
    activeLed = LED_YELLOW;
  } else if (temp < 35) {
    statusText = "HOT";
    activeLed = LED_RED;
  } else {
    statusText = "TOO HOT";
    activeLed = LED_RED;
  }

  // ======================
  // PRINT STATUS (ONLY ON CHANGE)
  // ======================
  if (statusText != lastStatus) {
    Serial.println(statusText);
    lastStatus = statusText;
  }

  // Blink LED
  if (activeLed != -1) {
    blinkLed(activeLed);
  }

  // ======================
  // OLED DISPLAY (NO STATUS)
  // ======================
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature:");
  display.print(temp, 1);
  display.print(" C");

  display.setCursor(0, 16);
  display.print("Humidity:   ");
  display.print(humi, 1);
  display.print(" %");

  display.display();
}
