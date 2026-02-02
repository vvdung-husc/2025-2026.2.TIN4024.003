/*
THÔNG TIN NHÓM 8
1. Trương Hữu Ngọc
2. Hồ Bảo Toàn
3. Nguyễn Vỹ Nguyên
*/

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== DHT22 =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_RED 4
#define LED_YELLOW 2
#define LED_BLUE 15

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Timer =====
unsigned long dhtTimer = 0;
unsigned long blinkTimer = 0;

float temperature = 0;
float humidity = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE, LOW);

  dht.begin();

  Wire.begin(13,12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Khong tim thay OLED");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("System Ready");
  display.display();
  delay(3000);
}

void loop() {
  unsigned long now = millis();

  // ===== Doc DHT moi 2s =====
  if (now - dhtTimer >= 2000) {
    dhtTimer = now;

    float t = dht.readTemperature(); //Nếu lỗi -> t = NaN (Not a Number)
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) { //Kiểm tra dữ liệu có hợp lệ không (không phải NaN)
      temperature = t;
      humidity = h;
    }
  }

  // ===== Blink LED moi 500ms =====
  if (now - blinkTimer >= 500) {
    blinkTimer = now;
    ledState = !ledState;

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_BLUE, LOW);

    String status = "";

    if (ledState) {
      if (temperature < 13) {
        status = "TOO COLD";
        digitalWrite(LED_BLUE, HIGH);
      }
      else if (temperature < 20) {
        status = "COLD";
        digitalWrite(LED_BLUE, HIGH);
      }
      else if (temperature < 25) {
        status = "COOL";
        digitalWrite(LED_YELLOW, HIGH);
      }
      else if (temperature < 30) {
        status = "WARM";
        digitalWrite(LED_YELLOW, HIGH);
      }
      else if (temperature <= 35) {
        status = "HOT";
        digitalWrite(LED_RED, HIGH);
      }
      else {
        status = "TOO HOT";
        digitalWrite(LED_RED, HIGH);
      }
    }

    // ===== Cap nhat OLED =====
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.setCursor(0, 15);
    display.print("Hum: ");
    display.print(humidity);
    display.println(" %");

    display.setCursor(0, 30);
    display.print("Status: ");
    display.println(status);

    display.display();
  }
}
