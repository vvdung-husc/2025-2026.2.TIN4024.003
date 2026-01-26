#include <Arduino.h>

#define LED_RED 25
#define LED_YELLOW 26
#define LED_GREEN 27

void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// Nháy 1 LED trong "durationMs" (tổng thời gian), mỗi nhịp on/off là "intervalMs"
void blinkFor(int pin, unsigned long durationMs, unsigned long intervalMs) {
  unsigned long start = millis();
  bool state = false;

  while (millis() - start < durationMs) {
    state = !state;
    allOff();                 // đảm bảo chỉ 1 đèn hoạt động
    digitalWrite(pin, state); // bật/tắt đèn đang active
    delay(intervalMs);
  }

  allOff(); // kết thúc pha thì tắt hết
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  allOff();
}

void loop() {
  // ĐỎ nháy liên tục trong 2s
  blinkFor(LED_RED, 2000, 200);     // 200ms đảo trạng thái 1 lần

  // VÀNG nháy liên tục trong 3s
  blinkFor(LED_YELLOW, 3000, 200);

  // XANH nháy liên tục trong 5s
  blinkFor(LED_GREEN, 5000, 200);
}
