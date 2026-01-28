#include <Arduino.h>
#define LED_RED    25
#define LED_YELLOW 26
#define LED_GREEN  27

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.begin(115200);
}

void loop() {
  // Đèn xanh
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  Serial.println("LED [GREEN] ON -> 7 Seconds");
  delay(7000);

  // Đèn vàng
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, LOW);
  Serial.println("LED [YELLOW] ON -> 3 Seconds");
  delay(3000);

  // Đèn đỏ
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  Serial.println("LED [RED] ON -> 5 Seconds");
  delay(5000);
}
