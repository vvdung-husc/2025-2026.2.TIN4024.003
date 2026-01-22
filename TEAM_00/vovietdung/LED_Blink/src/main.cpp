#include <Arduino.h>

uint8_t LED_RED = 17;

//Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  // put your setup code here, to run once:
  printf("Welcome IoT\n");
  pinMode(LED_RED, OUTPUT); // Set GPIO18 as an output pin
}

// unsigned long ulTimer = 0;
// bool bLEDStatus = false;
// void loop() {
//   if (IsReady(ulTimer,1000)){
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW); 
//   }
// }
void loop() {
  // put your main code here, to run repeatedly:
  printf("[LED_RED] => HIGH\n");
  digitalWrite(LED_RED, HIGH); // Turn LED ON
  delay(500); // Wait for 500 milliseconds
  printf("[LED_RED] => LOW\n");
  digitalWrite(LED_RED, LOW); // Turn LED OFF
  delay(500); // Wait for 500 milliseconds  
}


