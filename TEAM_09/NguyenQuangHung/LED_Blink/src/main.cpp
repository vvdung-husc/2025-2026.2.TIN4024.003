#include <Arduino.h>

#define LED_PIN 17   // KHỚP với diagram.json (esp:17)

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // LED ON
  delay(500);                  // 0.5 giây
  digitalWrite(LED_PIN, LOW);  // LED OFF
  delay(500);                  // 0.5 giây
}
