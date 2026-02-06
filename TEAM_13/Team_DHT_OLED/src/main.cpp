/*
THÔNG TIN NHÓM X
1. Nguyễn Tấn Nhân
2. Thân Hoàng Phước Minh
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= DHT =================
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= LED =================
#define LED_GREEN  15
#define LED_YELLOW 2
#define LED_RED    4

// ================= TIMER =================
unsigned long lastDHTRead = 0;
unsigned long lastBlink = 0;
const unsigned long DHT_INTERVAL = 2000;   // 2s
const unsigned long BLINK_INTERVAL = 500;  // LED blink

bool ledState = false;

// ================= DATA =================
float temperature = 0;
float humidity = 0;
String statusText = "";
int activeLED = 0; // 0:none, 1:green, 2:yellow, 3:red

// ================= FUNCTION =================
void updateStatus(float t) {
  if (t < 13) {
    statusText = "TOO COLD";
    activeLED = 1;
  } else if (t < 20) {
    statusText = "COLD";
    activeLED = 1;
  } else if (t < 25) {
    statusText = "COOL";
    activeLED = 2;
  } else if (t < 30) {
    statusText = "WARM";
    activeLED = 2;
  } else if (t < 35) {
    statusText = "HOT";
    activeLED = 3;
  } else {
    statusText = "TOO HOT";
    activeLED = 3;
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.println(" C");

  display.print("Humi: ");
  display.print(humidity, 1);
  display.println(" %");

  display.println("----------------");
  display.setTextSize(2);
  display.println(statusText);

  display.display();
}

void blinkLED() {
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    lastBlink = millis();
    ledState = !ledState;

    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    if (ledState) {
      if (activeLED == 1) digitalWrite(LED_GREEN, HIGH);
      if (activeLED == 2) digitalWrite(LED_YELLOW, HIGH);
      if (activeLED == 3) digitalWrite(LED_RED, HIGH);
    }
  }
}

// ================= SETUP =================
void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Wire.begin(13, 12);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  dht.begin();
}

// ================= LOOP =================
void loop() {
  // Read DHT (non-blocking)
  if (millis() - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temperature = t;
      humidity = h;
      updateStatus(temperature);
      updateOLED();
    }
  }

  blinkLED();
}
