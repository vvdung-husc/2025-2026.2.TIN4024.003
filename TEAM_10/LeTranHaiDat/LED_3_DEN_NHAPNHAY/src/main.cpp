#include <Arduino.h>

uint8_t PIN_RED = 25;
uint8_t PIN_GREEN = 27;
uint8_t PIN_YELLOW = 26;

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

unsigned long ulTimer = 0;
bool red_status = true;
bool green_status = false;
bool yellow_status = false;

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
}

void loop() {
  if(IsReady(ulTimer, 1000)) {
    if(red_status) {
      digitalWrite(PIN_RED, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_YELLOW, LOW);
      green_status = !green_status;
      red_status = !red_status;
    } else if(green_status) {
      digitalWrite(PIN_RED, LOW);
      digitalWrite(PIN_GREEN, HIGH);
      digitalWrite(PIN_YELLOW, LOW);
      yellow_status = !yellow_status;
      green_status = !green_status;
    } else if(yellow_status) {
      digitalWrite(PIN_RED, LOW);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_YELLOW, HIGH);
      red_status = !red_status;
      yellow_status = !yellow_status;
    }
  }
}