#include <Arduino.h>
#define LED_PIN 17
void setup() {
  printf("Hello World\n");
  pinMode(LED_PIN, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
