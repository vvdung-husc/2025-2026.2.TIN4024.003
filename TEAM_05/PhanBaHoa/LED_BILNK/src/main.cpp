#include <Arduino.h>

// // Define LED pin
// const int ledPin = 17;

// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   // put your setup code here, to run once:
//   pinMode(ledPin, OUTPUT);  // Set LED pin as output
//   int result = myFunction(2, 3);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   digitalWrite(ledPin, HIGH);  // Turn LED on
//   delay(1000);                 // Wait 1 second
//   digitalWrite(ledPin, LOW);   // Turn LED off
//   delay(1000);                 // Wait 1 second
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }



// Định nghĩa chân GPIO
const uint8_t LED_RED = 17;
const uint8_t LED_YELLOW = 16;
const uint8_t LED_GREEN = 4;

// Khai báo các trạng thái đèn
enum TrafficState { RED, GREEN, YELLOW };
TrafficState currentState = RED;

// Timer
unsigned long ulStateTimer = 0;   // Đếm thời gian trạng thái
unsigned long ulBlinkTimer = 0;   // Đếm thời gian blink

bool ledOn = false;

// Hàm kiểm tra thời gian không gây nghẽn
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void AllOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup() {
  Serial.begin(115200);
  printf("Traffic Light System Blink Started\n");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  ulStateTimer = millis();
  ulBlinkTimer = millis();
}

void loop() {
  // ===== BLINK mỗi 500ms =====
  if (IsReady(ulBlinkTimer, 500)) {
    ledOn = !ledOn;   // đảo trạng thái
  }

  switch (currentState) {
    case RED:
      // Blink đèn Đỏ
      digitalWrite(LED_RED, ledOn ? HIGH : LOW);

      // Sau 5 giây -> GREEN
      if (IsReady(ulStateTimer, 5000)) {
        AllOff();
        currentState = GREEN;
        printf("[SWITCH] => GREEN\n");
      }
      break;

    case GREEN:
      // Blink đèn Xanh
      digitalWrite(LED_GREEN, ledOn ? HIGH : LOW);

      // Sau 4 giây -> YELLOW
      if (IsReady(ulStateTimer, 4000)) {
        AllOff();
        currentState = YELLOW;
        printf("[SWITCH] => YELLOW\n");
      }
      break;

    case YELLOW:
      // Blink đèn Vàng
      digitalWrite(LED_YELLOW, ledOn ? HIGH : LOW);

      // Sau 2 giây -> RED
      if (IsReady(ulStateTimer, 2000)) {
        AllOff();
        currentState = RED;
        printf("[SWITCH] => RED\n");
      }
      break;
  }
}