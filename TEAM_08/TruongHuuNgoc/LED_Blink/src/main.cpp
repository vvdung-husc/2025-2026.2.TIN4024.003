#include <Arduino.h>

uint8_t LED_RED = 17;

void setup() {
  // put your setup code here, to run once:
  printf("Hello Iot!\n");
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_RED, HIGH);
  delay(500);
  digitalWrite(LED_RED, LOW);
  delay(500);
  // printf("Loop running!\n");
  // sleep(2);
}
