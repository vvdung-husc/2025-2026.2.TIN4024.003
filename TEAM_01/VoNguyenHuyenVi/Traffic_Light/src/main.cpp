#include <Arduino.h>
#include <TM1637Display.h>

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define TM_CLK 18
#define TM_DIO 19
TM1637Display display(TM_CLK, TM_DIO);

#define LDR_PIN 34
#define LDR_THRESHOLD 1000

bool isDark() {
  int v = analogRead(LDR_PIN);
  Serial.println(v);
  return v < LDR_THRESHOLD;
}

void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  display.clear();
}

void countdown(int s) {
  for (int i = s; i >= 0; i--) {
    if (!isDark()) {
      allOff();
      return;
    }
    display.showNumberDec(i, true);
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7);
}

void loop() {
  if (!isDark()) {
    allOff();
    delay(300);
    return;
  }

  digitalWrite(LED_RED, HIGH);
  countdown(5);
  digitalWrite(LED_RED, LOW);

  digitalWrite(LED_YELLOW, HIGH);
  countdown(3);
  digitalWrite(LED_YELLOW, LOW);

  digitalWrite(LED_GREEN, HIGH);
  countdown(5);
  digitalWrite(LED_GREEN, LOW);
}