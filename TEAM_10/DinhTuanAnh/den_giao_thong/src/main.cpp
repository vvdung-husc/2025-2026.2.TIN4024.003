#include <Arduino.h>
#include <TM1637Display.h>

#define RED 18
#define YELLOW 5
#define GREEN 17
#define GREEN2 12
#define BUTTON 13
#define LDR 34

#define CLK 22
#define DIO 23

TM1637Display display(CLK, DIO);

// Ngưỡng tối
#define DARK_THRESHOLD 2500  

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LDR, INPUT);

  display.setBrightness(7);
}

void blinkLed(int pin, int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(delayMs);
    digitalWrite(pin, LOW);
    delay(delayMs);
  }
}

void countdown(int seconds) {
  for (int i = seconds; i > 0; i--) {
    display.showNumberDec(i, true);
    delay(1000);
  }
}

void warningMode() {
  display.clear();

  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);

  digitalWrite(YELLOW, HIGH);
  digitalWrite(GREEN2, HIGH);
  delay(300);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN2, LOW);
  delay(300);
}

void loop() {
  int lightValue = analogRead(LDR);
  bool buttonPressed = digitalRead(BUTTON) == LOW;

  // ===== CHẾ ĐỘ CẢNH BÁO =====
  if (buttonPressed || lightValue > DARK_THRESHOLD) {
    warningMode();
    return;
  }

  // ===== ĐÈN XANH NHẤP NHÁY =====
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);

  for (int i = 3; i > 0; i--) {
    display.showNumberDec(i, true);
    digitalWrite(GREEN, HIGH);
    delay(300);
    digitalWrite(GREEN, LOW);
    delay(700);
  }

  // ===== ĐÈN VÀNG =====
  digitalWrite(YELLOW, HIGH);
  countdown(3);
  digitalWrite(YELLOW, LOW);

  // ===== ĐÈN ĐỎ =====
  digitalWrite(RED, HIGH);
  countdown(3);
  digitalWrite(RED, LOW);
}