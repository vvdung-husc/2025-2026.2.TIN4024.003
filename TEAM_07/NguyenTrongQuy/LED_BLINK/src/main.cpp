#include <Arduino.h>

// GPIO theo diagram.json
#define LED_RED     17
#define LED_YELLOW  16
#define LED_GREEN   4

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Traffic Light Simulation");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Tắt tất cả LED ban đầu
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// Hàm bật 1 đèn và đếm giây
void turnOnWithCountdown(int ledPin, const char* name, int seconds) {
  digitalWrite(ledPin, HIGH);

  for (int i = seconds; i > 0; i--) {
    Serial.print(name);
    Serial.print(" - còn ");
    Serial.print(i);
    Serial.println(" giây");
    delay(1000);
  }

  digitalWrite(ledPin, LOW);
}

void loop() {
  // 🔴 ĐÈN ĐỎ – 5 giây
  turnOnWithCountdown(LED_RED, "DEN DO", 5);

  // 🟡 ĐÈN VÀNG – 2 giây
  turnOnWithCountdown(LED_YELLOW, "DEN VANG", 2);

  // 🟢 ĐÈN XANH – 5 giây
  turnOnWithCountdown(LED_GREEN, "DEN XANH", 5);
}
