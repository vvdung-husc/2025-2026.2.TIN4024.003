#include <Arduino.h>

uint8_t LED_RED = 17;

//Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  printf("Welcome IoT\n");
  pinMode(LED_RED, OUTPUT);
}

unsigned long ulTimer = 0;
bool bLEDStatus = false;
void loop() {
  if (IsReady(ulTimer,1000)){
    bLEDStatus = !bLEDStatus;
    digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW); 
  }
}


