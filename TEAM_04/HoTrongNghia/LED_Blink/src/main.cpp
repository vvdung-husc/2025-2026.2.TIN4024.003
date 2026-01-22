#include <Arduino.h>


#define LED_PIN 2


void setup() {
 pinMode(LED_PIN, OUTPUT);
 Serial.begin(115200);
 Serial.println("Hello, Wokwi!");
}


void loop() {
 digitalWrite(LED_PIN, HIGH); // Bật đèn
 delay(500);
 digitalWrite(LED_PIN, LOW);  // Tắt đèn
 delay(500);
}
