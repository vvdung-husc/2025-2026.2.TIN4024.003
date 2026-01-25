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

// unsigned long ulTimer = 0;
// bool bLEDStatus = false;
// void loop() {
//   if (IsReady(ulTimer,1000)){
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW); 
//   }
// }
// // void loop() {
// //   // put your main code here, to run repeatedly:
// //   printf("[LED_RED] => HIGH\n");
// //   digitalWrite(LED_RED, HIGH); // Turn LED ON
// //   delay(500); // Wait for 500 milliseconds
// //   printf("[LED_RED] => LOW\n");
// //   digitalWrite(LED_RED, LOW); // Turn LED OFF
// //   delay(500); // Wait for 500 milliseconds  
// // }

#include <Arduino.h>

// Khai báo chân LED
uint8_t LED_RED    = 17;
uint8_t LED_YELLOW = 16;
uint8_t LED_GREEN  = 15;

// Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  printf("Welcome IoT - Traffic Light\n");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Ban đầu bật đèn đỏ
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// 0: Đỏ | 1: Xanh | 2: Vàng
uint8_t trafficState = 0;
unsigned long ulTimer = 0;

void loop() {
  switch (trafficState) {

    // 🔴 ĐÈN ĐỎ - 30s
    case 0:
      if (IsReady(ulTimer, 10000)) {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        trafficState = 1;
        printf("GREEN\n");
      }
      break;

    // 🟢 ĐÈN XANH - 27s
    case 1:
      if (IsReady(ulTimer, 7000)) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        trafficState = 2;
        printf("YELLOW\n");
      }
      break;

    // 🟡 ĐÈN VÀNG - 3s
    case 2:
      if (IsReady(ulTimer, 3000)) {
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, HIGH);
        trafficState = 0;
        printf("RED\n");
      }
      break;
  }
}

