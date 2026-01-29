#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

void blinkLed(int pin, unsigned long durationMs) {
  unsigned long startTime = millis();

  while (millis() - startTime < durationMs) {
    digitalWrite(pin, HIGH);
    delay(500);  
    digitalWrite(pin, LOW);
    delay(500);   
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void loop() {
  blinkLed(LED_RED, 7000);

  blinkLed(LED_YELLOW, 3000);

  blinkLed(LED_GREEN, 5000);
}
