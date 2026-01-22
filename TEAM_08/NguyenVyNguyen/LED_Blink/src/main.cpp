#include <Arduino.h>

uint8_t LED_RED = 17;

void setup()
{
  // put your setup code here, to run once:
  printf("Welcome Iot\n");
  pinMode(LED_RED, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  printf("[LED_RED] => HIGH\n");
  digitalWrite(LED_RED, HIGH);
  delay(500);
  printf("[LED_RED] => LOW\n");
  digitalWrite(LED_RED, LOW);
  delay(500);
}
