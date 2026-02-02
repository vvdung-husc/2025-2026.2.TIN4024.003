// sketch.ino - ESP32 Traffic LEDs (blink 6s each)
#include <Arduino.h>

#define LED_RED    25
#define LED_YELLOW 32
#define LED_GREEN  33

const unsigned long BLINK_DURATION_MS = 6000; // 6 giây
const unsigned long BLINK_PERIOD_MS   = 500;  // chu kỳ nháy (500ms): 250ms ON + 250ms OFF

void blinkFor(unsigned long pin, unsigned long durationMs) {
  unsigned long start = millis();

  while (millis() - start < durationMs) {
    digitalWrite(pin, HIGH);
    delay(BLINK_PERIOD_MS / 2);
    digitalWrite(pin, LOW);
    delay(BLINK_PERIOD_MS / 2);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Tắt hết lúc khởi động
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void loop() {
  blinkFor(LED_RED, BLINK_DURATION_MS);
  blinkFor(LED_YELLOW, BLINK_DURATION_MS);
  blinkFor(LED_GREEN, BLINK_DURATION_MS);
}
