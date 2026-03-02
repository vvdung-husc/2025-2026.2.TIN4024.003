/*
THÔNG TIN NHÓM TEAM_09
1. Trần Văn Mỹ
2. Đinh Hoàng Nhân
3. Lê Ngọc Minh Thư
4. Lê Văn Minh
5. Nguyễn Quang Hùng
*/

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <DHT.h>

// ===================== PIN CONFIG (ESP32) =====================
// Bạn có thể đổi pin cho đúng với mạch của bạn trên Wokwi
static const int PIN_DHT = 15; // DHT22 DATA
static const int LED_GREEN = 27;
static const int LED_YELLOW = 26;
static const int LED_RED = 25;

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
static uint32_t lastBlinkMs = 0;
static bool blinkState = false;

static float tempC = NAN;
static float humi = NAN;

enum TempLevel : uint8_t
{
  LV_TOO_COLD,
  LV_COLD,
  LV_COOL,
  LV_WARM,
  LV_HOT,
  LV_TOO_HOT
};

static TempLevel currentLevel = LV_COOL;

// ===================== HELPERS =====================
static void allLedsOff()
{
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

static const char *levelText(TempLevel lv)
{
  switch (lv)
  {
  case LV_TOO_COLD:
    return "TOO COLD";
  case LV_COLD:
    return "COLD";
  case LV_COOL:
    return "COOL";
  case LV_WARM:
    return "WARM";
  case LV_HOT:
    return "HOT";
  case LV_TOO_HOT:
    return "TOO HOT";
  }
  return "UNKNOWN";
}

static TempLevel classifyTemp(float t)
{
  if (t < 13.0)
    return LV_TOO_COLD;
  if (t < 20.0)
    return LV_COLD; // 13 - 20
  if (t < 25.0)
    return LV_COOL; // 20 - 25
  if (t < 30.0)
    return LV_WARM; // 25 - 30
  if (t <= 35.0)
    return LV_HOT;   // 30 - 35
  return LV_TOO_HOT; // > 35
}

static void updateOled(float t, float h, TempLevel lv)
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");

  display.setTextSize(2);
  display.setCursor(0, 12);
  if (isnan(t))
    display.print("--.--");
  else
    display.print(t, 2);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(0, 38);
  display.print("Humidity: ");

  display.setTextSize(2);
  display.setCursor(0, 48);
  if (isnan(h))
    display.print("--.--");
  else
    display.print(h, 2);
  display.print(" %");

  // trạng thái góc phải
  display.setTextSize(1);
  display.setCursor(78, 0);
  display.print(levelText(lv));

  display.display();
}

// LED nhấp nháy theo level: chỉ 1 màu nháy, 2 màu còn lại tắt
static void applyBlinkLed(TempLevel lv, bool on)
{
  allLedsOff();

  if (!on)
    return;

  if (lv == LV_TOO_COLD || lv == LV_COLD)
  {
    digitalWrite(LED_GREEN, HIGH);
    return;
  }

  if (lv == LV_COOL || lv == LV_WARM)
  {
    digitalWrite(LED_YELLOW, HIGH);
    return;
  }

  digitalWrite(LED_RED, HIGH);
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  allLedsOff();

  // I2C init
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("OLED init failed!");
    while (true)
      delay(100);
  }
  display.clearDisplay();
  display.display();

  dht.begin();

  // hiển thị màn hình chờ
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("ESP32 + DHT22 + OLED");
  display.setCursor(0, 14);
  display.print("Reading sensor...");
  display.display();
}

void loop() {
  uint32_t now = millis();

  // READ SENSOR
  if (now - lastSensorMs >= SENSOR_INTERVAL_MS) {
    lastSensorMs = now;

    float t = dht.readTemperature(); // Celsius
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
      Serial.println("Failed to read from DHT22!");
    } else {
      tempC = t;
      humi  = h;
      currentLevel = classifyTemp(tempC);
      Serial.printf("T=%.2fC, H=%.2f%%, Level=%s\n", tempC, humi, levelText(currentLevel));
    }

    // cập nhật OLED 
    updateOled(tempC, humi, currentLevel);
  }

  //  BLINK LED
  if (now - lastBlinkMs >= BLINK_INTERVAL_MS) {
    lastBlinkMs = now;
    blinkState = !blinkState;
    applyBlinkLed(currentLevel, blinkState);
  }
}