#include <Arduino.h>
#include <TM1637Display.h>

// ===== CHÂN KẾT NỐI =====
#define RED 18
#define YELLOW 5
#define GREEN 17
#define GREEN2 12
#define BUTTON 13
#define LDR 34

#define CLK 22
#define DIO 23

TM1637Display display(CLK, DIO);

#define DARK_THRESHOLD 2500  

// ===== HÀM NHẤP NHÁY + ĐẾM NGƯỢC =====
void blinkWithCountdown(int ledPin, int seconds) {
  for (int i = seconds; i > 0; i--) {
    display.showNumberDec(i, true);

    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin, LOW);
    delay(700);
  }
}

// ===== CHẾ ĐỘ CẢNH BÁO =====
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

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LDR, INPUT);

  display.setBrightness(7);
}

void loop() {
  int lightValue = analogRead(LDR);
  bool buttonPressed = digitalRead(BUTTON) == LOW;

  // ===== CHẾ ĐỘ CẢNH BÁO =====
  if (buttonPressed || lightValue > DARK_THRESHOLD) {
    warningMode();
    return;
  }

  // ===== ĐÈN XANH =====
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  blinkWithCountdown(GREEN, 3);

  // ===== ĐÈN VÀNG =====
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
  blinkWithCountdown(YELLOW, 3);

  // ===== ĐÈN ĐỎ =====
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, LOW);
  blinkWithCountdown(RED, 3);
}
