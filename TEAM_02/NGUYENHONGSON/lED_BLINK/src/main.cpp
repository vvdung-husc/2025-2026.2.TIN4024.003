#include <Arduino.h>

#define RED_LED    17
#define GREEN_LED  16
#define YELLOW_LED 25

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
}

void loop() {
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(4000);

  for (int i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, LOW);
    delay(300);
    digitalWrite(GREEN_LED, HIGH);
    delay(300);
  }

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  delay(2000);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(5000);
}
