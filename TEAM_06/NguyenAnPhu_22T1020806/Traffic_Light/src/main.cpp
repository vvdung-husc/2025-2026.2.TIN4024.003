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

<<<<<<< HEAD
// ===== global variables ==========
bool displayEnabled = true;   // true = show countdown, false = clear
bool lastButtonState = HIGH;  // for edge detection
bool nightMode = false;


// ===== TM1637 display object =====
TM1637Display display(TM_CLK, TM_DIO);

// ===== Function declaration =====
void runLed(uint8_t pin, int durationSeconds);
bool isDark(uint8_t doPin, uint8_t aoPin);
void handleButton();
void logLED(uint8_t pin);
void blinkLed(uint8_t pin, int durationSeconds);
void toNight(); void toDay();

// ===== main =====
void setup() {
  // hold push button for 1 second or more to toggle display
  Serial.begin(115200);
  Serial.println("----------------");
  Serial.println("Started");

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(LDR_DO_PIN, INPUT);
  pinMode(LDR_AO_PIN, INPUT);

  display.setBrightness(7);   // 0–7
  display.clear();
}

void loop() {

  if (isDark(LDR_DO_PIN, LDR_AO_PIN)) {
    // 🌙 NIGHT MODE
    display.clear();

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);

    Serial.println("Night mode: blinking YELLOW");

    blinkLed(YELLOW_LED, -1);  // infinite blink until light
    return;
  }

  // ☀️ DAY MODE (normal traffic light)
  runLed(RED_LED, 7);
  runLed(GREEN_LED, 7);
  runLed(YELLOW_LED, 3);
}


// ===== Function definition =====
void toNight() {
  if (nightMode) return;  // already in night mode

  if (!isDark(LDR_DO_PIN, LDR_AO_PIN)) return;

  nightMode = true;

  Serial.println("Night mode: blinking YELLOW");

  // Turn everything off first
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  display.clear();

  // Blink yellow until light returns
  while (isDark(LDR_DO_PIN, LDR_AO_PIN)) {
    digitalWrite(YELLOW_LED, HIGH);
    delay(500);
    digitalWrite(YELLOW_LED, LOW);
    delay(500);
  }

  toDay();
}

void toDay() {
  Serial.println("Day mode: traffic light reset");

  nightMode = false;

  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  display.clear();
}


void blinkLed(uint8_t pin, int durationSeconds) {
  unsigned long startTime = millis();

  while (true) {
    // Exit infinite blink if it becomes light again
    if (durationSeconds < 0) {
      if (!isDark(LDR_DO_PIN, LDR_AO_PIN)) {
        break;
      }
    }

    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);

    if (durationSeconds > 0) {
      if ((millis() - startTime) >= durationSeconds * 1000UL) {
        break;
      }
    }
  }
}

=======
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

>>>>>>> 754aa5c827405ec8fb9c1091de34bcd748150b0a


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

<<<<<<< HEAD
void logLED(uint8_t pin) {
  Serial.print("LED: ");
  String res = "";
  switch (pin)
  {
    case RED_LED:
      Serial.println("RED");
      break;

    case YELLOW_LED:
      Serial.println("yellow");
      break;

    case GREEN_LED:
      Serial.println("green");
      break;
    default:
      break;
  }
  Serial.println();
}


void runLed(uint8_t pin, int durationSeconds) {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  digitalWrite(pin, HIGH);

  for (int i = durationSeconds; i > 0; i--) {

    toNight();        // 🌙 interrupt if it becomes dark
    if (nightMode) return;

    handleButton();   // button still responsive

    if (displayEnabled) {
      display.showNumberDec(i, true);
    } else {
      display.clear();
=======


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
>>>>>>> 754aa5c827405ec8fb9c1091de34bcd748150b0a
    }
  }

<<<<<<< HEAD

  digitalWrite(pin, LOW);
  display.clear();
}

bool isDark(uint8_t doPin, uint8_t aoPin) {
  // Digital output from LDR module
  int digitalState = digitalRead(doPin);
  // Usually: LOW = light, HIGH = dark (depends on module trim pot)

  // Analog value for debugging / tuning
  int analogValue = analogRead(aoPin); // 0–4095

  Serial.print("LDR AO: ");
  Serial.print(analogValue);
  Serial.print(" | DO: ");
  Serial.println(digitalState);

  // Primary decision: digital comparator
  if (digitalState == HIGH) {
    return true;   // dark
  }

  return false;    // light
=======
  if (nightMode) {
    nightModeTask();
  } else {
    trafficLightTask();
  }
>>>>>>> 754aa5c827405ec8fb9c1091de34bcd748150b0a
}
