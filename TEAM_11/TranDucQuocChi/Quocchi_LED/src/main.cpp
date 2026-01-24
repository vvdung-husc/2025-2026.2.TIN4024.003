#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  pinMode(21, OUTPUT); //đỏ
  pinMode(22, OUTPUT); //xanh
  pinMode(23, OUTPUT); //vàng
}

int leds[] = {21, 22, 23};

void loop() {
  for (int i = 0; i < 3; i++) {
    // Tắt hết
    for (int j = 0; j < 3; j++) {
      digitalWrite(leds[j], LOW);
    }
    // Bật 1 đèn
    digitalWrite(leds[i], HIGH);
    delay(3000);
  }
}

