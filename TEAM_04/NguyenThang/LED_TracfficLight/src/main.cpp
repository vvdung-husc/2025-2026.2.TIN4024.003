#include <Arduino.h>
#include <TM1637Display.h>

/* ====== KHAI BÁO CHÂN ====== */
// Đèn giao thông
#define RED_PIN     23
#define YELLOW_PIN  22
#define GREEN_PIN   21

// LED phụ, nút nhấn, cảm biến
#define EXTRA_LED   19
#define BUTTON_PIN  18
#define SENSOR_PIN  34   // AO

// TM1637
#define CLK 5
#define DIO 4
TM1637Display display(CLK, DIO);

/* ====== BIẾN THỜI GIAN ====== */
int redTime = 10;
int greenTime = 10;
int yellowTime = 3;

bool nightMode = false;

/* ====== HÀM ĐIỀU KHIỂN ĐÈN ====== */
void setRed() {
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
}

void setGreen() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
}

void setYellow() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
}

/* ====== HÀM ĐẾM NGƯỢC ====== */
void countdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i, true);
    digitalWrite(EXTRA_LED, !digitalRead(EXTRA_LED));
    delay(1000);

    if (digitalRead(BUTTON_PIN) == LOW) {
      nightMode = !nightMode;
      delay(300);
      return;
    }
  }
}

/* ====== SETUP ====== */
void setup() {
  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(EXTRA_LED, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT);

  display.setBrightness(0x0f);
  display.clear();
}

/* ====== LOOP ====== */
void loop() {

  int sensorValue = analogRead(SENSOR_PIN);
  Serial.println(sensorValue);

  // Cảm biến điều chỉnh thời gian
  int base = map(sensorValue, 0, 4095, 5, 15);
  redTime = base;
  greenTime = base;
  yellowTime = 3;

  if (!nightMode) {
    // 🔴 ĐỎ
    setRed();
    countdown(redTime);

    // 🟢 XANH
    setGreen();
    countdown(greenTime);

    // 🟡 VÀNG
    setYellow();
    countdown(yellowTime);
  } 
  else {
    // 🌙 BAN ĐÊM: nhấp nháy vàng
    display.showNumberDec(0, true);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    digitalWrite(YELLOW_PIN, HIGH);
    delay(500);
    digitalWrite(YELLOW_PIN, LOW);
    delay(500);

    if (digitalRead(BUTTON_PIN) == LOW) {
      nightMode = false;
      delay(300);
    }
  }
}
