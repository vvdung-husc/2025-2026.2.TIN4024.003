#include <Arduino.h>
#include <TM1637Display.h>

// ===== CHÂN =====
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

// ===== KIỂM TRA CHUYỂN CHẾ ĐỘ =====
bool isWarningMode() {
  int lightValue = analogRead(LDR);
  bool buttonPressed = digitalRead(BUTTON) == LOW;
  return (buttonPressed || lightValue > DARK_THRESHOLD);
}

// ===== DELAY THÔNG MINH (có kiểm tra điều kiện) =====
bool smartDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (isWarningMode()) return true; // Thoát ngay
    delay(10);
  }
  return false;
}

// ===== NHẤP NHÁY + ĐẾM NGƯỢC (có thể bị ngắt) =====
bool blinkWithCountdown(int ledPin, int seconds) {
  for (int i = seconds; i > 0; i--) {
    display.showNumberDec(i, true);

    digitalWrite(ledPin, HIGH);
    if (smartDelay(300)) return true;

    digitalWrite(ledPin, LOW);
    if (smartDelay(700)) return true;
  }
  return false;
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

  // Nếu đang ở chế độ cảnh báo → lặp liên tục
  if (isWarningMode()) {
    warningMode();
    return;
  }

  // ===== XANH =====
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  if (blinkWithCountdown(GREEN, 3)) return;

  // ===== VÀNG =====
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
  if (blinkWithCountdown(YELLOW, 3)) return;

  // ===== ĐỎ =====
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, LOW);
  if (blinkWithCountdown(RED, 3)) return;
}
