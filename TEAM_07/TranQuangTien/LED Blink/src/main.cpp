#include <Arduino.h>

#define LED_GREEN  16
#define LED_YELLOW 17
#define LED_RED    18

void setup() {
  Serial.begin(115200);
  printf("Traffic Light System Start\n");

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  // ===== GREEN =====
  printf("GREEN ON (3.5s)\n");
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  delay(3500);

  // ===== YELLOW =====
  printf("YELLOW ON (1s)\n");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, LOW);
  delay(1000);

  // ===== RED =====
  printf("RED ON (3.5s)\n");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  delay(3500);
}
