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
  Serial.begin(115200);
  delay(50);
  Serial.println("setup start");
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  // ensure blue LED starts OFF (wiring: HIGH -> ON, LOW -> OFF)
  digitalWrite(BLUE_PIN, LOW);
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
    // detect stable falling edge (press)
    if ((now - lastButtonChangeMs) > debounceMs) {
      if (btn == LOW && lastButtonReading == HIGH) {
        // button pressed -> toggle blue blink mode
        blueMode = !blueMode;
        if (!blueMode) {
          // turning mode OFF -> ensure LED is OFF (LOW = OFF)
          digitalWrite(BLUE_PIN, LOW);
        } else {
          // turning mode ON -> start with LED ON immediately (HIGH = ON)
          blinkLastMs = now;
          blinkOn = true;
          digitalWrite(BLUE_PIN, HIGH);
        }
      }
    }
  }
  lastButtonReading = btn;

  // If in blue override mode: blink the blue LED continuously
  if (blueMode) {
    if (now - blinkLastMs >= blinkInterval) {
      blinkLastMs = now;
      blinkOn = !blinkOn;
      // LED active-high: HIGH -> ON, LOW -> OFF
      digitalWrite(BLUE_PIN, blinkOn ? HIGH : LOW);
      Serial.print("blink: ");
      Serial.println(blinkOn ? "ON" : "OFF");
    }
    // do NOT return; allow traffic to continue while blue warning blinks
  }

  // allow serial command 'b' to toggle blueMode for manual test
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'b' || c == 'B') {
      blueMode = !blueMode;
      Serial.print("serial toggle blueMode -> ");
      Serial.println(blueMode ? "ON" : "OFF");
      if (blueMode) {
        blinkLastMs = now;
        blinkOn = true;
        digitalWrite(BLUE_PIN, HIGH);
      } else {
        digitalWrite(BLUE_PIN, LOW);
      }
    }
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
