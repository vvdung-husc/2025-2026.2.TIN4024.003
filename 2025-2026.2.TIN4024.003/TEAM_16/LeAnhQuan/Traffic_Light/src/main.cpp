#include <Arduino.h>
#include <TM1637Display.h>

// ===== PIN =====
#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27

#define BUTTON_PIN  4
#define LDR_PIN     34

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ===== TIME =====
int tRed = 10;
int tYellow = 3;
int tGreen = 7;

bool showDisplay = true;
bool lastButtonState = HIGH;

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();
}

void handleButton() {
  bool current = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && current == LOW) {
    showDisplay = !showDisplay;
    delay(200); // debounce
  }
  lastButtonState = current;
}

bool isDark() {
  int light = analogRead(LDR_PIN);
  return light < 1500; // chỉnh nếu cần
}

void showTime(int t) {
  if (showDisplay) {
    display.showNumberDec(t, true);
  } else {
    display.clear();
  }
}

void loop() {
  // ===== NIGHT MODE =====
  if (isDark()) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    display.clear();
    delay(500);
    return;
  }

  // ===== RED =====
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  for (int i = tRed; i >= 0; i--) {
    handleButton();
    showTime(i);
    delay(1000);
  }

  // ===== GREEN =====
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);

  for (int i = tGreen; i >= 0; i--) {
    handleButton();
    showTime(i);
    delay(1000);
  }

  // ===== YELLOW =====
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);

  for (int i = tYellow; i >= 0; i--) {
    handleButton();
    showTime(i);
    delay(1000);
  }

  digitalWrite(LED_YELLOW, LOW);
}
