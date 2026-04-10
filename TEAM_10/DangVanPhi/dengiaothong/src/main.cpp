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

#define LIGHT_THRESHOLD 2000

TM1637Display display(CLK, DIO);

bool isWarningMode() {
  int rawValue = analogRead(LDR);
  int lightValue = 4095 - rawValue;

  Serial.print("Light Value (Adjusted): ");
  Serial.println(lightValue);

  bool buttonPressed = digitalRead(BUTTON) == LOW;
  return buttonPressed || lightValue >= LIGHT_THRESHOLD;
}

bool smartDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (isWarningMode()) {
      return true;
    }
    delay(10);
  }
  return false;
}

bool blinkWithCountdown(int ledPin, int seconds) {
  for (int i = seconds; i > 0; i--) {
    display.showNumberDec(i, true);

    digitalWrite(ledPin, HIGH);
    if (smartDelay(300)) {
      return true;
    }

    digitalWrite(ledPin, LOW);
    if (smartDelay(700)) {
      return true;
    }
  }
  return false;
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

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LDR, INPUT);

  Serial.begin(115200);
  display.setBrightness(7);
}

void loop() {
  if (isWarningMode()) {
    warningMode();
    return;
  }

  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  if (blinkWithCountdown(GREEN, 3)) {
    return;
  }

  digitalWrite(GREEN, LOW);
  if (blinkWithCountdown(YELLOW, 3)) {
    return;
  }

  digitalWrite(YELLOW, LOW);
  if (blinkWithCountdown(RED, 3)) {
    return;
  }
}
