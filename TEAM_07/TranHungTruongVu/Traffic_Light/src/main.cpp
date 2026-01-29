#include <Arduino.h>
#include <TM1637Display.h>

// ===== LED =====
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_BLUE 21

// ===== BUTTON =====
#define BUTTON_PIN 23

// ===== TM1637 =====
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// ===== Thời gian hiển thị (giây) =====
int redSeconds = 5;
int yellowSeconds = 3;
int greenSeconds = 7;

// ===== Tốc độ đếm =====
unsigned long speedFactor = 500;

// ===== Nhấp nháy LED =====
unsigned long blinkInterval = 300;

// ===== Trạng thái đèn =====
enum LightState { RED, YELLOW, GREEN };
LightState currentState = RED;

// ===== Thời gian =====
unsigned long stateStartTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastButtonTime = 0;

bool ledStatus = false;
int remainingSeconds = 0;

// ===== Trạng thái màn hình =====
bool displayEnabled = true;

void turnOffTrafficLED() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

int getCurrentSeconds() {
  if (currentState == RED) return redSeconds;
  if (currentState == YELLOW) return yellowSeconds;
  return greenSeconds;
}

int getCurrentLed() {
  if (currentState == RED) return LED_RED;
  if (currentState == YELLOW) return LED_YELLOW;
  return LED_GREEN;
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("Traffic Light Fixed!");

  display.setBrightness(7);

  remainingSeconds = getCurrentSeconds();
  display.showNumberDec(remainingSeconds);
}

void loop() {
  unsigned long now = millis();

  // ===== 1. BUTTON (không dùng delay) =====
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW && now - lastButtonTime > 200) {
    displayEnabled = !displayEnabled;
    lastButtonTime = now;
  }
  lastButtonState = buttonState;

  // ===== 2. LED BLUE =====
  digitalWrite(LED_BLUE, displayEnabled ? HIGH : LOW);

  int currentSeconds = getCurrentSeconds();
  int currentLed = getCurrentLed();

  // ===== 3. NHẤP NHÁY LED =====
  if (now - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = now;
    ledStatus = !ledStatus;
    turnOffTrafficLED();
    digitalWrite(currentLed, ledStatus ? HIGH : LOW);
  }

  // ===== 4. ĐẾM NGƯỢC =====
  unsigned long elapsed = now - stateStartTime;
  int secondsLeft = currentSeconds - (elapsed / speedFactor);
  if (secondsLeft < 0) secondsLeft = 0;

  if (secondsLeft != remainingSeconds) {
    remainingSeconds = secondsLeft;
    if (displayEnabled) display.showNumberDec(remainingSeconds);
  }

  if (!displayEnabled) {
    display.clear();
  }

  // ===== 5. CHUYỂN ĐÈN =====
  if (elapsed >= (unsigned long)currentSeconds * speedFactor) {
    stateStartTime = now;
    ledStatus = false;
    turnOffTrafficLED();

    if (currentState == RED) currentState = YELLOW;
    else if (currentState == YELLOW) currentState = GREEN;
    else currentState = RED;

    remainingSeconds = getCurrentSeconds();
    if (displayEnabled) display.showNumberDec(remainingSeconds);
  }
}
