#include <Arduino.h>
#include <TM1637Display.h>

// ===== Pin definitions =====
#define RED_LED     27
#define YELLOW_LED  23
#define GREEN_LED   5

#define TM_CLK      22
#define TM_DIO      21

#define BUTTON_PIN  4

#define LDR_DO_PIN  14
#define LDR_AO_PIN  13

// ===== Timing IDs =====
#define TIMER_LED       0
#define TIMER_BLINK     1
#define TIMER_BUTTON    2
#define TIMER_LDR       3

// ===== Global state =====
bool displayEnabled = true;
bool lastButtonState = HIGH;
bool nightMode = false;

// Traffic light state
enum TrafficState {
  RED,
  GREEN,
  YELLOW
};

TrafficState trafficState = RED;
int countdown = 7;

// ===== TM1637 =====
TM1637Display display(TM_CLK, TM_DIO);



// ======================================================
// ⏱️ NON-BLOCKING TIMER
// ======================================================
bool isReady(uint8_t id, int durationMs) {
  static unsigned long lastTime[40];
  unsigned long now = millis();

  if (now - lastTime[id] >= (unsigned long)durationMs) {
    lastTime[id] = now;
    return true;
  }
  return false;
}



// ======================================================
// 🌗 LDR CHECK
// ======================================================
bool isDark(uint8_t doPin, uint8_t aoPin) {
  int digitalState = digitalRead(doPin);
  return (digitalState == HIGH);   // HIGH = dark (module dependent)
}



// ======================================================
// 🔘 BUTTON HANDLER (non-blocking debounce)
// ======================================================
void handleButton() {
  if (!isReady(TIMER_BUTTON, 50)) return;

  bool currentState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentState == LOW) {
    displayEnabled = !displayEnabled;
    if (!displayEnabled) display.clear();
  }

  lastButtonState = currentState;
}



// ======================================================
// 🌙 NIGHT MODE (non-blocking)
// ======================================================
void nightModeTask() {
  static bool ledState = false;

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  display.clear();

  if (isReady(TIMER_BLINK, 500)) {
    ledState = !ledState;
    digitalWrite(YELLOW_LED, ledState);
  }

  // Exit night mode when light returns
  if (!isDark(LDR_DO_PIN, LDR_AO_PIN)) {
    nightMode = false;
    digitalWrite(YELLOW_LED, LOW);
    trafficState = RED;
    countdown = 7;
    display.clear();
  }
}



// ======================================================
// 🚦 TRAFFIC LIGHT STATE MACHINE
// ======================================================
void trafficLightTask() {

  if (!isReady(TIMER_LED, 1000)) return;

  countdown--;

  if (displayEnabled) {
    display.showNumberDec(countdown, true);
  } else {
    display.clear();
  }

  if (countdown > 0) return;

  // Switch state
  switch (trafficState) {

    case RED:
      trafficState = GREEN;
      countdown = 7;
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      break;

    case GREEN:
      trafficState = YELLOW;
      countdown = 3;
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      break;

    case YELLOW:
      trafficState = RED;
      countdown = 7;
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      break;
  }
}



// ======================================================
// ⚙️ SETUP
// ======================================================
void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_DO_PIN, INPUT);
  pinMode(LDR_AO_PIN, INPUT);

  display.setBrightness(7);
  display.clear();

  digitalWrite(RED_LED, HIGH);  // start with RED
}



// ======================================================
// 🔁 LOOP (FAST & NON-BLOCKING)
// ======================================================
void loop() {

  handleButton();

  // Check LDR periodically
  if (isReady(TIMER_LDR, 200)) {
    if (isDark(LDR_DO_PIN, LDR_AO_PIN)) {
      nightMode = true;
    }
  }

  if (nightMode) {
    nightModeTask();
  } else {
    trafficLightTask();
  }
}
