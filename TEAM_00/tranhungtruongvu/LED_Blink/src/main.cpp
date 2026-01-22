#include <Arduino.h>

uint8_t LED_RED = 17;
uint8_t LED_GREEN = 4;

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  // put your setup code here, to run once:
  printf("Hello, World!\n");
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

unsigned long ulTimer = 0;
bool bLEDStatus = false;

void loop() {
  if (IsReady(ulTimer,1000)){
    bLEDStatus = !bLEDStatus;
    digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW); 
    digitalWrite(LED_GREEN, bLEDStatus ? LOW : HIGH);
  }
}

// void loop() {
//   printf("Looping...\n");
//   digitalWrite(LED_RED, HIGH); // Turn the LED on (HIGH is the voltage level)
//   delay(500);
//   digitalWrite(LED_RED, LOW); // Turn the LED off by making the voltage LOW
//   delay(500); //hạn chế sd
// }


