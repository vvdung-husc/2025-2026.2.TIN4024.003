#include <Arduino.h>

#define LED D4   // LED trên board NodeMCU

void setup() {
  pinMode(LED, OUTPUT);   // khai báo chân LED là output
}

void loop() {
  digitalWrite(LED, LOW);   // bật LED
  delay(200);               // delay 200ms

  digitalWrite(LED, HIGH);  // tắt LED
  delay(200);               // delay 200ms
}