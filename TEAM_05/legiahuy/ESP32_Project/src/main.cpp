#include <Arduino.h>

#define LED_PIN LED_BUILTIN

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, LOW);   // bật LED
  delay(1000);

  digitalWrite(LED_PIN, HIGH);  // tắt LED
  delay(1000);
}