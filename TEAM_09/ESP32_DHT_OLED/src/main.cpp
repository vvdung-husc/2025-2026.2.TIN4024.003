/*
THÔNG TIN NHÓM TEAM_09
1. Trần Văn Mỹ
2. 
3. 
4. 
5.
*/

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <DHT.h>

// ===================== PIN CONFIG (ESP32) =====================
// Bạn có thể đổi pin cho đúng với mạch của bạn trên Wokwi
static const int PIN_DHT   = 15;   // DHT22 DATA
static const int LED_GREEN = 27;
static const int LED_YELLOW = 26;
static const int LED_RED   = 25;

// OLED I2C (thường trên ESP32 Devkit V1):
// SDA = 21, SCL = 22 (Wokwi mặc định cũng hay vậy)
static const int OLED_SDA = 21;
static const int OLED_SCL = 22;

// ===================== DHT =====================
#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);

// ===================== OLED SSD1306 =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Nếu OLED của bạn là 0x3C (thường là vậy trên Wokwi)
static const uint8_t OLED_ADDR = 0x3C;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===================== TIMING =====================
// đọc DHT mỗi 2 giây (DHT22 đọc nhanh quá dễ lỗi)
static const uint32_t SENSOR_INTERVAL_MS = 2000;

// nhấp nháy LED mỗi 400ms
static const uint32_t BLINK_INTERVAL_MS = 400;

// ===================== STATE =====================
static uint32_t lastSensorMs = 0;
static uint32_t lastBlinkMs  = 0;
static bool blinkState = false;

static float tempC = NAN;
static float humi  = NAN;

enum TempLevel : uint8_t {
  LV_TOO_COLD,
  LV_COLD,
  LV_COOL,
  LV_WARM,
  LV_HOT,
  LV_TOO_HOT
};

static TempLevel currentLevel = LV_COOL;

