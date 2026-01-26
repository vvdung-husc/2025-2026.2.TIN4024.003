#include <Arduino.h>

// Chân LED (an toàn cho ESP32)
#define LED_RED     17
#define LED_YELLOW  5
#define LED_GREEN   18


void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.println("Traffic Light Started");
}

void loop() {
  // 🔴 Đèn đỏ
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  Serial.println("RED");
  delay(2000);

  // 🟡 Đèn vàng
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, LOW);
  Serial.println("YELLOW");
  delay(1000);

  // 🟢 Đèn xanh
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  Serial.println("GREEN");
  delay(2000);
}
