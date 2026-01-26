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

uint8_t trafficState = 0;   // 0: RED, 1: GREEN, 2: YELLOW
uint8_t blinkCount = 0;     // số lần đã nhấp nháy
bool ledStatus = false;     // trạng thái ON/OFF hiện tại
unsigned long blinkTimer = 0;

void turnOffAll() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup() {
  printf("Traffic Light Blinking Mode\n");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {

  // Mỗi 500ms đổi trạng thái LED
  if (!IsReady(blinkTimer, 500)) return;

  ledStatus = !ledStatus;   // đảo trạng thái
  if (ledStatus) blinkCount++;  // chỉ tính khi LED bật

  switch (trafficState) {

    // ================= RED =================
    case 0:
      turnOffAll();
      digitalWrite(LED_RED, ledStatus);

      if (blinkCount >= 5) {      // đủ 5 lần
        blinkCount = 0;
        trafficState = 1;
        printf("-> GREEN\n");
      }
      break;

    // ================= GREEN =================
    case 1:
      turnOffAll();
      digitalWrite(LED_GREEN, ledStatus);

      if (blinkCount >= 7) {
        blinkCount = 0;
        trafficState = 2;
        printf("-> YELLOW\n");
      }
      break;

    // ================= YELLOW =================
    case 2:
      turnOffAll();
      digitalWrite(LED_YELLOW, ledStatus);

      if (blinkCount >= 3) {
        blinkCount = 0;
        trafficState = 0;
        printf("-> RED\n");
      }
      break;
  }
}