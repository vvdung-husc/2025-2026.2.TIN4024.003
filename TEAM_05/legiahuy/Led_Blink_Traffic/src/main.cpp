#include <Arduino.h>
#include <TM1637Display.h>

// ----------- LED PINS -----------
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32
#define PIN_LED_BLUE    21   // LED báo hệ thống chạy

// ----------- BUTTON -------------
#define PIN_BUTTON 23   // nối GND, INPUT_PULLUP

// ----------- TIME (ms) ----------
#define TIME_RED     10000
#define TIME_YELLOW  3000
#define TIME_GREEN   7000

#define BLINK_TIME   500
#define COUNTDOWN_INTERVAL 1000

// ----------- TM1637 -------------
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// ----------- STATE --------------
enum TrafficState {
  RED,
  GREEN,
  YELLOW
};

TrafficState currentState = RED;

// ----------- TIMER --------------
unsigned long stateTimer = 0;
unsigned long blinkTimer = 0;
unsigned long countdownTimer = 0;

// ----------- FLAG ---------------
bool ledStatus = false;
bool systemStarted = false;
bool lastButtonState = HIGH;

int remainingSeconds = 0;

void allOff() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
}


void setState(TrafficState newState, int timeMs) {
  currentState = newState;
  stateTimer = millis();
  countdownTimer = millis();
  remainingSeconds = timeMs / 1000;
  if(systemStarted)
    display.showNumberDec(remainingSeconds, true);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  display.setBrightness(0x0f);
  display.clear();

  allOff();
  digitalWrite(PIN_LED_BLUE, LOW);
}

void loop() {
  unsigned long now = millis();

  bool buttonState = digitalRead(PIN_BUTTON);
  if (lastButtonState == HIGH && buttonState == LOW) {
    systemStarted = !systemStarted;

    if (systemStarted) {
      Serial.println("SYSTEM STARTED");
      digitalWrite(PIN_LED_BLUE, HIGH);
      setState(RED, TIME_RED);
    } else {
      Serial.println("SYSTEM STOPPED");
      allOff();
      digitalWrite(PIN_LED_BLUE, LOW);
      display.clear();
    }
    delay(50); 
  }
  lastButtonState = buttonState;


  if (now - blinkTimer >= BLINK_TIME) {
    blinkTimer = now;
    ledStatus = !ledStatus;

    allOff();
    if(systemStarted)
      digitalWrite(PIN_LED_BLUE, ledStatus);

    if (currentState == RED)
      digitalWrite(PIN_LED_RED, ledStatus);
    else if (currentState == GREEN)
      digitalWrite(PIN_LED_GREEN, ledStatus);
    else if (currentState == YELLOW)
      digitalWrite(PIN_LED_YELLOW, ledStatus);
  }

  if (now - countdownTimer >= COUNTDOWN_INTERVAL && systemStarted) {
    countdownTimer = now;

    if (remainingSeconds > 0) {
      remainingSeconds--;
      display.showNumberDec(remainingSeconds, true);
    }
  }


  switch (currentState) {

    case RED:
      if (now - stateTimer >= TIME_RED) {
        setState(GREEN, TIME_GREEN);
      }
      break;

    case GREEN:
      if (now - stateTimer >= TIME_GREEN) {
        setState(YELLOW, TIME_YELLOW);
      }
      break;

    case YELLOW:
      if (now - stateTimer >= TIME_YELLOW) {
        setState(RED, TIME_RED);
      }
      break;
  }
}