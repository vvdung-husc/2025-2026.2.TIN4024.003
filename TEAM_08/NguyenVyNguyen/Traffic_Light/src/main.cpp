#include <Arduino.h>
#include <TM1637Display.h>

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

#define BUTTON_PIN 23

// ===== TIME SIMULATION =====
int currentHour = 23;   // ĐỔI GIỜ Ở ĐÂY để test

bool isNightTime() {
  return (currentHour >= 22 || currentHour < 6);
}

// ===== TIMER =====
bool IsReady(unsigned long &t, uint32_t ms) {
  if (millis() - t < ms) return false;
  t = millis();
  return true;
}

// ===== STATE =====
uint8_t state = 0;  
int countdown = 5;
bool ledStatus = false;
bool displayOn = true;

unsigned long blinkTimer = 0;
unsigned long secTimer = 0;

void turnOffAll() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void changeState(uint8_t newState) {
  state = newState;
  switch(state) {
    case 0: countdown = 5; break;
    case 1: countdown = 7; break;
    case 2: countdown = 3; break;
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  changeState(0);
}

void loop() {

  // ===== CHẾ ĐỘ BAN ĐÊM =====
  if (isNightTime()) {

    // Nhấp nháy vàng mỗi 500ms
    if (IsReady(blinkTimer, 500)) {
      ledStatus = !ledStatus;
      digitalWrite(LED_YELLOW, ledStatus);
    }

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);

    display.clear();  // ban đêm tắt hiển thị
    return;           // bỏ qua phần giao thông ban ngày
  }

  // ===== NÚT NHẤN =====
  if (digitalRead(BUTTON_PIN) == LOW) {
    displayOn = !displayOn;
    delay(200);
  }

  // ===== ĐẾM GIÂY =====
  if (IsReady(secTimer, 1000)) {
    countdown--;
    if (countdown <= 0) {
      changeState((state + 1) % 3);
    }
  }

  // ===== NHẤP NHÁY =====
  if (IsReady(blinkTimer, 500)) {
    ledStatus = !ledStatus;
  }

  turnOffAll();

  if (state == 0) digitalWrite(LED_RED, ledStatus);
  if (state == 1) digitalWrite(LED_GREEN, ledStatus);
  if (state == 2) digitalWrite(LED_YELLOW, ledStatus);

  if (displayOn) display.showNumberDec(countdown, false);
  else display.clear();
}
