#include <Arduino.h>
#include <TM1637Display.h>

/* =====================
   TM1637
   ===================== */
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

/* =====================
   LED GIAO THÔNG
   ===================== */
#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27

/* =====================
   LED XANH NHỎ (đấu ngược)
   ===================== */
#define LED_BLUE    21   // LOW = ON

/* =====================
   BUTTON
   ===================== */
#define BUTTON_PIN  23

/* =====================
   BIẾN TRẠNG THÁI
   ===================== */
bool systemOn = true;
bool lastButtonState = HIGH;

/* =====================
   HÀM TẮT HẾT
   ===================== */
void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, HIGH); // tắt LED xanh
  display.clear();
}

/* =====================
   HÀM NHẤP NHÁY + ĐẾM
   ===================== */
void blinkLed(int pin, int seconds) {
  for (int i = seconds; i > 0; i--) {
    display.showNumberDec(i, false);
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);

    // nếu đang chạy mà bị tắt → thoát ngay
    if (!systemOn) return;
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(LED_BLUE, HIGH); // tắt ban đầu
}

void loop() {
  /* ====== ĐỌC NÚT NHẤN (TOGGLE) ====== */
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    systemOn = !systemOn;   // đảo trạng thái
    delay(200);             // chống dội nút
  }
  lastButtonState = buttonState;

  /* ====== NẾU HỆ THỐNG TẮT ====== */
  if (!systemOn) {
    allOff();
    return;   // dừng loop tại đây
  }

  /* ====== HỆ THỐNG ĐANG MỞ ====== */
  digitalWrite(LED_BLUE, LOW); // LED xanh báo đang chạy

  blinkLed(LED_RED, 5);
  blinkLed(LED_YELLOW, 3);
  blinkLed(LED_GREEN, 7);
}
