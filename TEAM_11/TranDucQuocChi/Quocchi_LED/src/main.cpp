#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 17
#define SCL_PIN 16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int ledR = 21;
const int ledY = 22;
const int ledG = 23;

void showOLED(const char* label, int sec) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(label);

  display.setTextSize(4);
  display.setCursor(54, 24);
  display.println(sec);

  display.display();
}

// Nhấp nháy đúng 1 giây
void blink1Second(int pin, int interval_ms = 250) {
  int times = 1000 / interval_ms;
  for (int k = 0; k < times; k++) {
    digitalWrite(pin, !digitalRead(pin));
    delay(interval_ms);
  }
  digitalWrite(pin, LOW); // kết thúc 1 giây thì tắt 
}

void runLamp(const char* label, int pin) {
  // chạy 3 giây: mỗi giây vừa hiển thị số, vừa nhấp nháy
  for (int s = 3; s >= 1; s--) {
    showOLED(label, s);
    blink1Second(pin, 250);
  }
  digitalWrite(pin, LOW);
}

void setup() {
  pinMode(ledR, OUTPUT);
  pinMode(ledY, OUTPUT);
  pinMode(ledG, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) delay(10);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  runLamp("RED", ledR);
  runLamp("YELLOW", ledY);
  runLamp("GREEN", ledG);
}
