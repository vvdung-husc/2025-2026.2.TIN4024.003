#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27

void blinkLed(int pin, int seconds, const char* name) {
  Serial.printf("LED [%s] ON => %d Seconds\n", name, seconds);

  for (int i = 0; i < seconds; i++) {
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void loop() {
  blinkLed(LED_RED, 5, "RED");
  delay(1000);

  blinkLed(LED_YELLOW, 3, "YELLOW");
  delay(1000);

  blinkLed(LED_GREEN, 7, "GREEN");
  delay(2000);
}
