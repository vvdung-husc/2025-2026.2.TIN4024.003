#include <Arduino.h>
#define LED_RED     26
#define LED_YELLOW  25
#define LED_GREEN   33

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  digitalWrite(LED_RED, HIGH);
  delay(5000);
  digitalWrite(LED_RED, LOW);

  digitalWrite(LED_YELLOW, HIGH);
  delay(3000);
  digitalWrite(LED_YELLOW, LOW);

  digitalWrite(LED_GREEN, HIGH);
  delay(7000);
  digitalWrite(LED_GREEN, LOW);
}
