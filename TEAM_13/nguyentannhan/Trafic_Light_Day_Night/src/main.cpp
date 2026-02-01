#include <Arduino.h>
#include <TM1637Display.h>

// ================= PIN =================
#define CLK 18
#define DIO 19

#define RED_PIN     27
#define YELLOW_PIN  26
#define GREEN_PIN   25
#define STATUS_LED  21
#define BUTTON_PIN  23
#define LDR_PIN     13

TM1637Display display(CLK, DIO);

// ================= TIME =================
#define RED_TIME      10
#define GREEN_TIME     7
#define YELLOW_TIME    3

#define BLINK_TIME   400        
#define LDR_THRESHOLD 2500
#define LDR_INTERVAL  500       
// ================= FSM =================
enum State {
  STOPPED,
  RED_RUNNING,
  GREEN_RUNNING,
  YELLOW_RUNNING,
  NIGHT_BLINK
};

State currentState = STOPPED;

// ================= VARIABLE =================
unsigned long lastSecond = 0;
unsigned long blinkMillis = 0;
unsigned long lastLdrRead = 0;

int countdown = 0;
bool blinkState = false;
bool lastBtn = HIGH;
bool dashShown = false;
int lastLdrValue = 0;

// ================= HELPER =================
void setLights(bool r, bool y, bool g) {
  digitalWrite(RED_PIN, r);
  digitalWrite(YELLOW_PIN, y);
  digitalWrite(GREEN_PIN, g);
}

void showDashOnce() {
  if (dashShown) return;
  static const uint8_t dash[] = { 0x40, 0x40, 0x40, 0x40 };
  display.setSegments(dash);
  dashShown = true;
}

void enterState(State newState) {
  currentState = newState;
  lastSecond = millis();
  blinkMillis = millis();
  dashShown = false;

  switch (currentState) {
    case STOPPED:
      setLights(LOW, LOW, LOW);
      digitalWrite(STATUS_LED, HIGH);
      display.clear();
      break;

    case RED_RUNNING:
      setLights(HIGH, LOW, LOW);
      digitalWrite(STATUS_LED, LOW);
      countdown = RED_TIME;
      display.showNumberDec(countdown, true);
      break;

    case GREEN_RUNNING:
      setLights(LOW, LOW, HIGH);
      countdown = GREEN_TIME;
      display.showNumberDec(countdown, true);
      break;

    case YELLOW_RUNNING:
      setLights(LOW, HIGH, LOW);
      countdown = YELLOW_TIME;
      display.showNumberDec(countdown, true);
      break;

    case NIGHT_BLINK:
      setLights(LOW, LOW, LOW);
      display.clear();
      blinkState = false;
      break;
  }
}

// ================= BUTTON =================
void checkButton() {
  bool btn = digitalRead(BUTTON_PIN);

  if (btn == LOW && lastBtn == HIGH) {
    delay(20); // debounce ngắn hơn
    if (digitalRead(BUTTON_PIN) == LOW) {
      if (currentState == STOPPED)
        enterState(RED_RUNNING);
      else
        enterState(STOPPED);
    }
  }
  lastBtn = btn;
}

// ================= SETUP =================
void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  display.setBrightness(0x07); 

  enterState(STOPPED);
}

// ================= LOOP =================
void loop() {
  checkButton();
  if (currentState == STOP6PED) return;

  unsigned long now = millis();

  // ===== READ LDR THEO CHU KỲ =====
  if (now - lastLdrRead >= LDR_INTERVAL) {
    lastLdrRead = now;
    lastLdrValue = analogRead(LDR_PIN);

    if (lastLdrValue > LDR_THRESHOLD && currentState != NIGHT_BLINK) {
      enterState(NIGHT_BLINK);
    }

    if (lastLdrValue <= LDR_THRESHOLD && currentState == NIGHT_BLINK) {
      enterState(RED_RUNNING);
    }
  }

  switch (currentState) {

    case RED_RUNNING:
    case GREEN_RUNNING:
    case YELLOW_RUNNING:
      if (now - lastSecond >= 1000) {
        lastSecond = now;
        countdown--;
        display.showNumberDec(countdown, true);

        if (countdown <= 0) {
          if (currentState == RED_RUNNING)
            enterState(GREEN_RUNNING);
          else if (currentState == GREEN_RUNNING)
            enterState(YELLOW_RUNNING);
          else
            enterState(RED_RUNNING);
        }
      }
      break;

    case NIGHT_BLINK:
      showDashOnce();
      if (now - blinkMillis >= BLINK_TIME) {
        blinkMillis = now;
        blinkState = !blinkState;
        digitalWrite(YELLOW_PIN, blinkState);
      }
      break;
  }
}
