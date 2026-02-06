/*
THÔNG TIN NHÓM 11
1. Trần Quốc Tiến
2. Trần Đức Quốc Chí
3. Lê Tấn Toàn
4. Đặng Tấn Phát
5. Hồ Văn Thạnh
*/
#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===================== PIN MAP =====================
static const int8_t LED_RED    = 4;
static const int8_t LED_YELLOW = 2;
static const int8_t LED_CYAN   = 15; 

// ===================== DHT22 =====================
#define DHTPIN  16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===================== OLED =====================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===================== LED STATES =====================
enum LedColor { CYAN, YELLOW, RED };
static LedColor currentLed = CYAN;
static bool ledOn = false;
static unsigned long lastBlinkMs = 0;
static const unsigned long BLINK_PERIOD_MS = 250;

// ===================== TIMER & VARS =====================
static unsigned long lastReadMs = 0;
static const unsigned long READ_PERIOD_MS = 800;

enum Mode {
  MODE_AUTO_RANDOM, 
  MODE_REAL_SENSOR, 
  MODE_MANUAL_FIX   
};

Mode currentMode = MODE_REAL_SENSOR; 

float currentTemp = 25.0; 
float currentHum  = 60.0;
float prevTemp = -999.0; 
float prevHum  = -999.0;
static const float HYS = 0.5f;

// ===================== HELPERS =====================
void allLedOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_CYAN, LOW);
}

const char* ledName(LedColor c) {
  switch (c) {
    case CYAN:   return "CYAN";
    case YELLOW: return "YELLOW";
    case RED:    return "RED";
    default:     return "UNKNOWN";
  }
}

void updateBlinkLed() {
  unsigned long now = millis();
  if (now - lastBlinkMs < BLINK_PERIOD_MS) return;
  lastBlinkMs = now;
  ledOn = !ledOn;
  allLedOff();
  if (!ledOn) return;

  switch (currentLed) {
    case CYAN:   digitalWrite(LED_CYAN, HIGH); break;
    case YELLOW: digitalWrite(LED_YELLOW, HIGH); break;
    case RED:    digitalWrite(LED_RED, HIGH); break;
  }
}

// ===== XỬ LÝ LỆNH TẮT =====
void checkSerialCommand() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); 
    input.trim(); 
    input.toLowerCase(); 

    if (input == "w") {
      currentMode = MODE_REAL_SENSOR;
      Serial.println(F(">> [MODE] WOKWI SLIDER"));
      prevTemp = -999; 
    }
    else if (input == "a") {
      currentMode = MODE_AUTO_RANDOM;
      Serial.println(F(">> [MODE] AUTO RANDOM"));
      prevTemp = -999;
    }
    else if (input.startsWith("t")) {
      currentMode = MODE_MANUAL_FIX;
      currentTemp = input.substring(1).toFloat();
      prevTemp = -999; 
    }
  }
}

String tempLabel(float C) {
  if (C < 13.0) return "TOO COLD";
  if (C < 20.0) return "COLD";
  if (C < 25.0) return "COOL";
  if (C < 30.0) return "WARM";
  if (C <= 35.0) return "HOT";
  return "TOO HOT";
}

LedColor ledByTempWithHys(float C, LedColor prev) {
  if (prev == CYAN) {
    if (C >= 20.0f + HYS) return YELLOW;
    return CYAN;
  }
  if (prev == YELLOW) {
    if (C < 20.0f - HYS) return CYAN;
    if (C >= 30.0f + HYS) return RED;
    return YELLOW;
  }
  if (prev == RED) {
    if (C < 30.0f - HYS) return YELLOW;
    return RED;
  }
  if (C < 20.0f) return CYAN;
  if (C < 30.0f) return YELLOW;
  return RED;
}

void screenWrite(float C, float H, LedColor ledState) {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(tempLabel(C));

  display.setCursor(90, 0);
  if (currentMode == MODE_AUTO_RANDOM) display.print("AUTO");
  else if (currentMode == MODE_REAL_SENSOR) display.print("WOKWI");
  else display.print("FIXED");

  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(C, 1);
  display.cp437(true);
  display.write((uint8_t)248);
  display.print("C");

  display.setTextSize(1);
  display.setCursor(0, 38);
  display.print("Hum: ");
  display.print(H, 1);
  display.print("%  LED:");
  display.println(ledName(ledState));

  display.setTextSize(2);
  display.setCursor(0, 48);
  display.print(H, 1);
  display.print("%");

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_CYAN, OUTPUT);
  allLedOff();

  Wire.begin(13, 12);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true) delay(100);
  }
  display.clearDisplay();
  display.display();
  dht.begin();

  Serial.println(F("=== SYSTEM STARTED ==="));
  Serial.println(F(">> Go 'a' -> Auto "));
  Serial.println(F(">> Go 'w' -> Wokwi "));
}

void loop() {
  updateBlinkLed();
  checkSerialCommand(); 

  unsigned long now = millis();
  if (now - lastReadMs >= READ_PERIOD_MS) {
    lastReadMs = now;

    if (currentMode == MODE_AUTO_RANDOM) {
      currentTemp += random(-80, 81) / 10.0; 
      if (currentTemp < -5) currentTemp = -5;
      if (currentTemp > 60) currentTemp = 60;
      currentHum = 60.0; 
    }
    else if (currentMode == MODE_REAL_SENSOR) {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      if (!isnan(t) && !isnan(h)) {
        currentTemp = t;
        currentHum = h;
      }
    }

    if (currentTemp != prevTemp || currentHum != prevHum) {
      currentLed = ledByTempWithHys(currentTemp, currentLed);
      screenWrite(currentTemp, currentHum, currentLed);

      Serial.print(currentMode == MODE_AUTO_RANDOM ? "[AUTO] " : (currentMode == MODE_REAL_SENSOR ? "[WOKWI] " : "[FIXED] "));
      Serial.print("Temp="); Serial.print(currentTemp, 1);
      Serial.print("C | Hum="); Serial.print(currentHum, 1);
      Serial.print("% | Level="); Serial.print(tempLabel(currentTemp));
      Serial.print(" | LED="); Serial.println(ledName(currentLed));

      prevTemp = currentTemp;
      prevHum = currentHum;
    }
  }
}