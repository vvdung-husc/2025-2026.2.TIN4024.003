#include <Arduino.h>
#include <TM1637Display.h>

#define LED_RED    27
#define LED_YELLOW 26
#define LED_GREEN  25

#define BTN_PIN   23
#define LED_BLUE  21 
#define LED_BLUE_ON   HIGH
#define LED_BLUE_OFF  LOW

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

enum TrafficState { RED, GREEN, YELLOW };
TrafficState currentState = RED;

int remainSeconds = 3;

bool buttonMode = false;
bool lastBtnState = HIGH;

unsigned long lastSecondMillis = 0;

void applyState(TrafficState s) {
  currentState = s;

  digitalWrite(LED_RED,    s == RED    ? HIGH : LOW);
  digitalWrite(LED_GREEN,  s == GREEN  ? HIGH : LOW);
  digitalWrite(LED_YELLOW, s == YELLOW ? HIGH : LOW);

  if (s == RED)    remainSeconds = 3;
  if (s == GREEN)  remainSeconds = 5;
  if (s == YELLOW) remainSeconds = 2;

  if (buttonMode) display.showNumberDec(remainSeconds, true);
}

void nextState() {
  if (currentState == RED) applyState(GREEN);
  else if (currentState == GREEN) applyState(YELLOW);
  else applyState(RED);
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(BTN_PIN, INPUT_PULLUP);

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LED_BLUE_OFF);

  display.setBrightness(7);
  display.clear();

  applyState(RED);
  lastSecondMillis = millis();
}

void loop() {
  unsigned long now = millis();

  //NÚT TOGGLE
  bool btnState = digitalRead(BTN_PIN);
  if (lastBtnState == HIGH && btnState == LOW) {
    buttonMode = !buttonMode;

    if (buttonMode) {
      digitalWrite(LED_BLUE, LED_BLUE_ON);
      display.showNumberDec(remainSeconds, true);
    } else {
      digitalWrite(LED_BLUE, LED_BLUE_OFF);
      display.clear();
    }
    delay(30);
  }
  lastBtnState = btnState;

  //TICK 1 GIÂY: đếm & đổi đèn
  if (now - lastSecondMillis >= 1000) {
    lastSecondMillis += 1000;

    remainSeconds--;
    if (remainSeconds <= 0) {
      nextState(); // tự set lại remainSeconds theo pha mới
    } else {
      if (buttonMode) display.showNumberDec(remainSeconds, true);
    }
  }
}
