#include <Arduino.h>
#include <TM1637Display.h>

// ===== LED GPIO =====
#define LED_RED     14
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    27

// ===== BUTTON GPIO =====
#define BUTTON_PIN  15

// ===== TM1637 =====
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// ===== TIME (giây) =====
#define TIME_GREEN  10
#define TIME_YELLOW 3
#define TIME_RED    10

int cycleCount = 0;   // đếm số chu kỳ
bool ledBlueState = false;  // trạng thái đèn xanh dương
bool lastButtonState = HIGH; // trạng thái nút trước đó

// ================== KIỂM TRA NÚT LIÊN TỤC ==================
void checkButton() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  // Phát hiện nhả nút (chuyển từ LOW sang HIGH)
  if (lastButtonState == LOW && currentButtonState == HIGH) {
    ledBlueState = !ledBlueState;  // Đảo trạng thái đèn
    
    if (ledBlueState) {
      digitalWrite(LED_BLUE, HIGH);  // Bật đèn xanh dương
      display.clear();  // Tắt bảng đếm giây
    } else {
      digitalWrite(LED_BLUE, LOW);  // Tắt đèn xanh dương
    }
    delay(50);  // chống dội
  }
  
  lastButtonState = currentButtonState;
}

// ================== DELAY CÓ KIỂM TRA NÚT ==================
void delayWithButton(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    checkButton();
    delay(10);  // delay nhỏ để không quá nhanh
  }
}

// ================== VÀNG NHẤP NHÁY ==================
void blinkYellow() {
  if (!ledBlueState) {
    display.clear();
  }
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  while (true) {
    digitalWrite(LED_YELLOW, HIGH);
    delayWithButton(500);
    digitalWrite(LED_YELLOW, LOW);
    delayWithButton(500);
  }
}

// ================== 1 CHU KỲ ĐÈN ==================
void trafficOneCycle() {

  // ===== XANH =====
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  for (int i = TIME_GREEN; i >= 0; i--) {
    if (!ledBlueState) {
      display.showNumberDec(i, true);
    }
    delayWithButton(1000);
  }

  // ===== VÀNG =====
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  for (int i = TIME_YELLOW; i >= 0; i--) {
    if (!ledBlueState) {
      display.showNumberDec(i, true);
    }
    delayWithButton(1000);
  }

  // ===== ĐỎ =====
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  for (int i = TIME_RED; i >= 0; i--) {
    if (!ledBlueState) {
      display.showNumberDec(i, true);
    }
    delayWithButton(1000);
  }
}

// ================== SETUP ==================
void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();
}

// ================== LOOP ==================
void loop() {
  // Chạy 2 chu kỳ đầu
  if (cycleCount < 2) {
    trafficOneCycle();
    cycleCount++;
  }
  // Sau 2 chu kỳ -> chỉ nhấp nháy vàng
  else {
    blinkYellow();
  }
}
