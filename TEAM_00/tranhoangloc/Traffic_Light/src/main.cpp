#include <Arduino.h>
#include <TM1637Display.h>

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define TM_CLK 18
#define TM_DIO 19

TM1637Display display(TM_CLK, TM_DIO);

#define BTN_PIN 23

void countdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i, true); 
    delay(1000);
  }
}

void setup() {

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void loop() {
  // ===== ĐÈN ĐỎ =====
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  countdown(5);

  // ===== ĐÈN VÀNG =====
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, LOW);
  countdown(3);

  // ===== ĐÈN XANH =====
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  countdown(5);

  digitalWrite(LED_GREEN, LOW);
}
