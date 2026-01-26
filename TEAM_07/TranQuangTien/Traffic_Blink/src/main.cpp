#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27

void blinkLed(int pin, int times, const char* name) {
  Serial.printf("LED [%s] BLINK => %d Times\n", name, times);

  for (int i = 1; i <= times; i++) {
    digitalWrite(pin, HIGH);
    Serial.printf("  %s blink %d\n", name, i);
    delay(500);          // ON 0.5s

    digitalWrite(pin, LOW);
    delay(500);          // OFF 0.5s
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.println("Traffic Light Blink Started");
}

void loop() {
  blinkLed(LED_RED, 5, "RED");
  blinkLed(LED_YELLOW, 3, "YELLOW");
  blinkLed(LED_GREEN, 7, "GREEN");
}
