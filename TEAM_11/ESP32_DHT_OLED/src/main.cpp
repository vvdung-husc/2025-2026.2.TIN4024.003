/*
THÔNG TIN NHÓM 11
1. Trần Quốc Tiến
2. Trần Đức Quốc Chí
3. Lê Tấn Toàn
4. Đặng Tấn Phát
*/

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===================== PIN MAP (theo diagram) =====================
static const int8_t LED_RED    = 4;
static const int8_t LED_YELLOW = 2;
static const int8_t LED_GREEN  = 15;   // Wokwi màu cyan nhưng coi là GREEN

// ===================== DHT22 =====================
#define DHTPIN  16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===================== OLED SSD1306 =====================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===================== LED BLINK (không dùng delay) =====================
enum LedColor { GREEN, YELLOW, RED };

static LedColor currentLed = GREEN;
static bool ledOn = false;
static unsigned long lastBlinkMs = 0;
static const unsigned long BLINK_PERIOD_MS = 250;

// ===================== DHT READ PERIOD =====================
static unsigned long lastReadMs = 0;
static const unsigned long READ_PERIOD_MS = 1000; // DHT22 đọc 1s/lần ổn định

void allLedOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void updateBlinkLed() {
  unsigned long now = millis();
  if (now - lastBlinkMs < BLINK_PERIOD_MS) return;

  lastBlinkMs = now;
  ledOn = !ledOn;

  allLedOff();
  if (!ledOn) return;

  switch (currentLed) {
    case GREEN:  digitalWrite(LED_GREEN, HIGH); break;
    case YELLOW: digitalWrite(LED_YELLOW, HIGH); break;
    case RED:    digitalWrite(LED_RED, HIGH); break;
  }
}

// ===== phân loại nhiệt độ (6 mức text) =====
String tempLabel(float C) {
  if (C < 13) return "TOO COLD";
  if (C < 20) return "COLD";
  if (C < 25) return "COOL";
  if (C < 30) return "WARM";
  if (C <= 35) return "HOT";
  return "TOO HOT";
}

// ===== LED theo yêu cầu đề: <20 GREEN, 20-30 YELLOW, >=30 RED =====
LedColor ledByTemp(float C) {
  if (C < 20) return GREEN;
  if (C < 30) return YELLOW;
  return RED;
}

void showErrorOnOled(const char* msg) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}

void screenWrite(float C, float H) {
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Dòng 1: trạng thái nhiệt độ theo bảng
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.println(tempLabel(C));

  // Dòng 2: nhiệt độ số lớn + do C
  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(C, 2);

  // Degree symbol
  display.cp437(true);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(String(C, 2), 0, 14, &x1, &y1, &w, &h);
  display.setCursor(w + 6, 14);
  display.write((uint8_t)248);
  display.print("C");

  // Dòng 3: Humidity
  display.setTextSize(1);
  display.setCursor(0, 38);
  display.println("Humidity:");

  // Dòng 4: độ ẩm số lớn + %
  display.setTextSize(2);
  display.setCursor(0, 48);
  display.print(H, 2);

  display.getTextBounds(String(H, 2), 0, 48, &x1, &y1, &w, &h);
  display.setCursor(w + 6, 48);
  display.print("%");

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  allLedOff();

  // OLED I2C theo diagram: SDA=13, SCL=12
  Wire.begin(13, 12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) delay(100);
  }
  display.clearDisplay();
  display.display();

  dht.begin();
}

void loop() {
  // LED nhấp nháy liên tục theo currentLed
  updateBlinkLed();

  // Đọc DHT theo chu kỳ
  unsigned long now = millis();
  if (now - lastReadMs >= READ_PERIOD_MS) {
    lastReadMs = now;

    float C = dht.readTemperature();
    float H = dht.readHumidity();

    // Theo yêu cầu thực tế: phải xử lý khi đọc lỗi
    if (isnan(C) || isnan(H)) {
      Serial.println("DHT22 read failed!");
      currentLed = YELLOW;                 // báo lỗi bằng đèn vàng
      showErrorOnOled("DHT22 read failed!");
      return;
    }

    currentLed = ledByTemp(C);             // đúng ngưỡng LED theo đề
    screenWrite(C, H);                      // OLED hiển thị nhiệt + ẩm + trạng thái

    Serial.print("T=");
    Serial.print(C, 2);
    Serial.print("C  H=");
    Serial.print(H, 2);
    Serial.println("%");
  }
}
