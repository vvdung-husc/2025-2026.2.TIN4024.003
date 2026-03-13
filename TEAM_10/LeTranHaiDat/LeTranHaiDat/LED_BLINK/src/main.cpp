#include <Arduino.h>

uint8_t LED_GREEN = 27;
bool Led_Green_Status = false;
unsigned long ulTimer = 0;

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  if(IsReady(ulTimer,1000)){
    Led_Green_Status = !Led_Green_Status;
    digitalWrite(LED_GREEN, Led_Green_Status ? HIGH : LOW); 
  }
}