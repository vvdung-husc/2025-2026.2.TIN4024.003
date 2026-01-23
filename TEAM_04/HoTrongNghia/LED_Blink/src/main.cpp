#include <Arduino.h>


#define LED_PIN 2


//Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}


void setup() {
 pinMode(LED_PIN, OUTPUT);
 Serial.begin(115200);
 Serial.println("Hello, Wokwi!");
}
// unsigned long ulTimer = 0;
// bool bLEDStatus = false;
// void loop() {
//   if (IsReady(ulTimer,1000)){
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(LED_PIN, bLEDStatus ? HIGH : LOW); 
//   }
// }

void loop() { 
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
