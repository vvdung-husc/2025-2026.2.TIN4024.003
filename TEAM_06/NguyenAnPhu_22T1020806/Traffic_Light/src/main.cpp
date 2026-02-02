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


void handleButton() {
  bool currentState = digitalRead(BUTTON_PIN);

  // Detect button press (HIGH → LOW)
  if (lastButtonState == HIGH && currentState == LOW) {
    displayEnabled = !displayEnabled;

    if (!displayEnabled) {
      Serial.println("Off");
      display.clear();
    }

    Serial.println("On");

    delay(200); // debounce
  }

  lastButtonState = currentState;
}

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
    }

    delay(1000);
  }


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
}
