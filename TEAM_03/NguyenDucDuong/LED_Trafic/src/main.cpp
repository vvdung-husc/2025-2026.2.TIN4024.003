#include <Arduino.h>
#include <TM1637Display.h>

// Pin definitions (the wiring in diagram.json)
const uint8_t RED_PIN = 27;
const uint8_t YELLOW_PIN = 26;
const uint8_t GREEN_PIN = 25;
const uint8_t CLK_PIN = 18; // TM1637 CLK
const uint8_t DIO_PIN = 19; // TM1637 DIO
const uint8_t BLUE_PIN = 21;   // blue LED per diagram
const uint8_t BUTTON_PIN = 23; // pushbutton per diagram

TM1637Display display(CLK_PIN, DIO_PIN);

enum TrafficState { RED_STATE, GREEN_STATE, YELLOW_STATE };

const int DURATION_RED = 5;    // seconds
const int DURATION_YELLOW = 2; // seconds
const int DURATION_GREEN = 4;  // seconds

unsigned long stateStartMs = 0;
int stateDurationSec = 0;
TrafficState currentState = RED_STATE;
int lastDisplayedSec = -1;
// button / blue-blink override
bool blueMode = false;
int pausedRemaining = 0;
int lastButtonReading = HIGH;
unsigned long lastButtonChangeMs = 0;
const unsigned long debounceMs = 50;
// blink timing
unsigned long blinkLastMs = 0;
const unsigned long blinkInterval = 300; // ms
bool blinkOn = false;

void setState(TrafficState s) {
  currentState = s;
  stateStartMs = millis();
  lastDisplayedSec = -1; // force display update

  switch (s) {
    case RED_STATE:
      stateDurationSec = DURATION_RED;
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      break;
    case GREEN_STATE:
      stateDurationSec = DURATION_GREEN;
      digitalWrite(RED_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      break;
    case YELLOW_STATE:
      stateDurationSec = DURATION_YELLOW;
      digitalWrite(RED_PIN, LOW);
      digitalWrite(YELLOW_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      break;
  }
}

void nextState() {
  if (currentState == RED_STATE) setState(GREEN_STATE);
  else if (currentState == GREEN_STATE) setState(YELLOW_STATE);
  else setState(RED_STATE);
}

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(0x0f);
  setState(RED_STATE);
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsedSec = (now - stateStartMs) / 1000UL;

  int remaining = stateDurationSec - (int)elapsedSec;

  // --- button reading with debounce ---
  int btn = digitalRead(BUTTON_PIN);
  if (btn != lastButtonReading) {
    lastButtonChangeMs = now;
  }
  if ((now - lastButtonChangeMs) > debounceMs) {
    // stable
    if (btn == LOW && !blueMode) {
      // enter blue override (button pressed)
      blueMode = true;
      pausedRemaining = remaining > 0 ? remaining : 0;
      // turn off traffic LEDs
      digitalWrite(RED_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      // init blink (LED wired such that LOW = ON)
      blinkLastMs = now;
      blinkOn = false;
      digitalWrite(BLUE_PIN, HIGH); // ensure off state at start
      // clear display (optional)
      display.clear();
    } else if (btn == HIGH && blueMode) {
      // button released -> resume traffic timer where it paused
      blueMode = false;
      stateStartMs = now;
      stateDurationSec = pausedRemaining;
      lastDisplayedSec = -1;
      digitalWrite(BLUE_PIN, HIGH); // turn off when exiting override
      // reapply current traffic light outputs without resetting timer
      switch (currentState) {
        case RED_STATE:
          digitalWrite(RED_PIN, HIGH);
          digitalWrite(YELLOW_PIN, LOW);
          digitalWrite(GREEN_PIN, LOW);
          break;
        case GREEN_STATE:
          digitalWrite(RED_PIN, LOW);
          digitalWrite(YELLOW_PIN, LOW);
          digitalWrite(GREEN_PIN, HIGH);
          break;
        case YELLOW_STATE:
          digitalWrite(RED_PIN, LOW);
          digitalWrite(YELLOW_PIN, HIGH);
          digitalWrite(GREEN_PIN, LOW);
          break;
      }
    }
  }
  lastButtonReading = btn;

  // If in blue override mode: blink the blue LED continuously
  if (blueMode) {
    if (now - blinkLastMs >= blinkInterval) {
      blinkLastMs = now;
      blinkOn = !blinkOn;
      // LED wired reversed in diagram: LOW -> ON, HIGH -> OFF
      digitalWrite(BLUE_PIN, blinkOn ? LOW : HIGH);
    }
    return; // skip normal traffic processing while overriding
  }

  // Normal traffic flow
  if (remaining <= 0) {
    nextState();
    return;
  }

  if (remaining != lastDisplayedSec) {
    // show remaining seconds on right two digits of 4-digit TM1637
    // display.showNumberDec(number, leading_zero, length, pos)
    display.showNumberDec(remaining, false, 2, 2);
    lastDisplayedSec = remaining;
  }
}
