// #include <Arduino.h>

// uint8_t LED_RED = 17;
// uint8_t LED_YELLOW = 16;
// uint8_t LED_GREEN = 18;

// //Non-blocking
// bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
//   if (millis() - ulTimer < millisecond) return false;
//   ulTimer = millis();
//   return true;
// }

// void setup() {
//   // put your setup code here, to run once:
//   printf("Welcome IoT\n");
//   pinMode(LED_RED, OUTPUT); // Set GPIO18 as an output pin
//   pinMode(LED_YELLOW, OUTPUT); // Set GPIO18 as an output pin
//   pinMode(LED_GREEN, OUTPUT); // Set GPIO18 as an output pin
// }

// unsigned long ulTimer = 0;
// bool bLEDStatus = false;
// void loop() {
//   if (IsReady(ulTimer,5000)){
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW); 
//   }
//   if (IsReady(ulTimer,3000)){
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(LED_YELLOW, bLEDStatus ? HIGH : LOW); 
//   }
//   if (IsReady(ulTimer,5000)){
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(LED_GREEN, bLEDStatus ? HIGH : LOW);
//   }
// }
// // void loop() {
// //   // put your main code here, to run repeatedly:
// //   printf("[LED_RED] => HIGH\n");
// //   digitalWrite(LED_RED, HIGH); // Turn LED ON
// //   delay(500); // Wait for 500 milliseconds
// //   printf("[LED_RED] => LOW\n");
// //   digitalWrite(LED_RED, LOW); // Turn LED OFF
// //   delay(500); // Wait for 500 milliseconds  
// // }
#include <Arduino.h>

uint8_t LED_RED = 17;
uint8_t LED_YELLOW = 16;
uint8_t LED_GREEN = 18;

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
  pinMode(LED_YELLOW, OUTPUT); // Set GPIO18 as an output pin
  pinMode(LED_GREEN, OUTPUT); // Set GPIO18 as an output pin
}

unsigned long ulTimerRed = 0;
unsigned long ulTimerYellow = 0;
unsigned long ulTimerGreen = 0;
bool bLEDRedStatus = false;
bool bLEDYellowStatus = false;
bool bLEDGreenStatus = false;
void loop() {
  if (IsReady(ulTimerRed,5000)){
    bLEDRedStatus = !bLEDRedStatus;
    digitalWrite(LED_RED, bLEDRedStatus ? HIGH : LOW); 
  }
  if (IsReady(ulTimerYellow,3000)){
    bLEDYellowStatus = !bLEDYellowStatus;
    digitalWrite(LED_YELLOW, bLEDYellowStatus ? HIGH : LOW); 
  }
  if (IsReady(ulTimerGreen,7000)){
    bLEDGreenStatus = !bLEDGreenStatus;
    digitalWrite(LED_GREEN, bLEDGreenStatus ? HIGH : LOW);
  }
}