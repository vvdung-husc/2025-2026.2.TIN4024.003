#include <Arduino.h>
#include <TM1637Display.h>

/* =====================
   TM1637 (7 đoạn)
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
   LOW = ON
   ===================== */
#define LED_BLUE    21

/* =====================
   BUTTON (INPUT_PULLUP)
   ===================== */
#define BUTTON_PIN  23

/* =====================
   BIẾN TRẠNG THÁI
   ===================== */
bool systemOn = true;
bool lastButtonState = HIGH;

/* =====================
   TẮT TOÀN BỘ HỆ THỐNG
   ===================== */
void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, HIGH); // tắt LED xanh (đấu ngược)
  display.clear();
}

/* =====================
   NHẤP NHÁY + ĐẾM NGƯỢC
   ===================== */
void blinkLed(int pin, int seconds) {
  for (int i = seconds; i > 0; i--) {
    display.showNumberDec(i, false);

    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);

    // Nếu nhấn nút tắt giữa chừng → thoát ngay
    if (!systemOn) return;
  }
}

/* =====================
   SETUP
   ===================== */
void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(LED_BLUE, HIGH); // tắt LED xanh lúc khởi động
}

/* =====================
   LOOP
   ===================== */
void loop() {

  /* ===== ĐỌC NÚT NHẤN (TOGGLE) ===== */
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    systemOn = !systemOn;   // đảo trạng thái ON/OFF
    delay(200);             // chống dội nút
  }
  lastButtonState = buttonState;

  /* ===== NẾU HỆ THỐNG TẮT ===== */
  if (!systemOn) {
    allOff();
    return;
  }

  /* ===== HỆ THỐNG ĐANG CHẠY ===== */
  digitalWrite(LED_BLUE, LOW); // LED xanh báo đang chạy

  blinkLed(LED_RED, 5);     // đèn đỏ 5s
  blinkLed(LED_YELLOW, 3);  // đèn vàng 3s
  blinkLed(LED_GREEN, 7);   // đèn xanh 7s
}
