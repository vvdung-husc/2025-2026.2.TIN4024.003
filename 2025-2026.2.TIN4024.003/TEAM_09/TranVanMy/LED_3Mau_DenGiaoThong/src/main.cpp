#include <Arduino.h>

static const int LED_TMP;
static const int LED_RED    = 25;
static const int LED_YELLOW = 26;
static const int LED_GREEN  = 27;

static void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  allOff();
}

void loop() {
  // ĐỎ 3 giây
  allOff();
  digitalWrite(LED_RED, HIGH);
  delay(3000);

  // VÀNG 2 giây
  allOff();
  digitalWrite(LED_YELLOW, HIGH);
  delay(2000);

  // XANH 4 giây
  allOff();
  digitalWrite(LED_GREEN, HIGH);
  delay(4000);
}
