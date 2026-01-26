// #include <Arduino.h>

// uint8_t LED_RED = 17;

// //Non-blocking
// bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
//   if (millis() - ulTimer < millisecond) return false;
//   ulTimer = millis();
//   return true;
// }

// void setup() {
//   // put your setup code here, to run once:
//   printf("Welcome IoT\n");
//   pinMode(LED_RED, OUTPUT); // Set GPIO18 as an output pin
// }

// // unsigned long ulTimer = 0;
// // bool bLEDStatus = false;
// // void loop() {
// //   if (IsReady(ulTimer,1000)){
// //     bLEDStatus = !bLEDStatus;
// //     digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW); 
// //   }
// // }
// void loop() {
//   // put your main code here, to run repeatedly:
//   printf("[LED_RED] => HIGH\n");
//   digitalWrite(LED_RED, HIGH); // Turn LED ON
//   delay(500); // Wait for 500 milliseconds
//   printf("[LED_RED] => LOW\n");
//   digitalWrite(LED_RED, LOW); // Turn LED OFF
//   delay(500); // Wait for 500 milliseconds  
// }
#include <Arduino.h>

// Định nghĩa chân GPIO
const uint8_t LED_RED = 17;
const uint8_t LED_YELLOW = 16;
const uint8_t LED_GREEN = 4;

// Khai báo các trạng thái đèn
enum TrafficState { RED, GREEN, YELLOW };
TrafficState currentState = RED;

unsigned long ulTimer = 0;

// Hàm kiểm tra thời gian không gây nghẽn (của bạn)
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  Serial.begin(115200);
  printf("Traffic Light System Started\n");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  // Trạng thái ban đầu: Đỏ
  digitalWrite(LED_RED, HIGH);
}

void loop() {
  switch (currentState) {
    case RED:
      // Đèn Đỏ sáng trong 5000ms (5 giây)
      if (IsReady(ulTimer, 5000)) {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        currentState = GREEN;
        printf("[SWITCH] => GREEN\n");
      }
      break;

    case GREEN:
      // Đèn Xanh sáng trong 4000ms (4 giây)
      if (IsReady(ulTimer, 4000)) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        currentState = YELLOW;
        printf("[SWITCH] => YELLOW\n");
      }
      break;

    case YELLOW:
      // Đèn Vàng sáng trong 2000ms (2 giây)
      if (IsReady(ulTimer, 2000)) {
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, HIGH);
        currentState = RED;
        printf("[SWITCH] => RED\n");
      }
      break;
  }
}

