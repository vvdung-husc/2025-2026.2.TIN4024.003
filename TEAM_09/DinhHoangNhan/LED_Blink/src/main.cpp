#include <Arduino.h>

uint8_t LED_RED = 17;

bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
  if (millis() - ulTimer < millisecond)
    return false;
  ulTimer = millis();
  return true;
}

void setup()
{
  // put your setup code here, to run once:
  printf("Welcome IoT\n");
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
