// #include <Arduino.h>

// // Định nghĩa chân LED
// uint8_t LED_RED = 17;

// // Hàm timer không chặn (non-blocking)
// bool isReady (unsigned long &ulTimmer, uint32_t milliseconds) {
//   if (millis() - ulTimmer >= milliseconds) {
//     ulTimmer = millis();
//     return true;
//   }
//   return false;
// }

// void setup() {
//   printf("Hello, World!");
//   pinMode(LED_RED, OUTPUT);
// }

// unsigned long ulTimmer = 0;
// bool bLEDStatus = false;

// void loop() {
//   // Nhấp nháy LED mỗi 500ms
//   if (isReady(ulTimmer, 500)) {
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW);
//     printf("LED_RED %s\n", bLEDStatus ?  "ON" : "OFF");
//   }
// }

#include <Arduino.h>

// ========== ĐỊNH NGHĨA CHÂN LED ==========
const uint8_t LED_RED = 17;
const uint8_t LED_YELLOW = 16;
const uint8_t LED_GREEN = 18;

// ========== THỜI GIAN ĐÈN GIAO THÔNG ==========
const uint32_t RED_DURATION = 5000;      // 5 giây
const uint32_t YELLOW_DURATION = 2000;   // 2 giây
const uint32_t GREEN_DURATION = 5000;    // 5 giây

// ========== TRẠNG THÁI ĐÈN GIAO THÔNG ==========
enum TrafficLightState {
  RED_LIGHT,
  YELLOW_LIGHT,
  GREEN_LIGHT
};

// ========== BIẾN TOÀN CỤC ==========
TrafficLightState currentState = RED_LIGHT;
unsigned long ulTimer = 0;

// ========== HÀM TIMER KHÔNG CHẶN ==========
bool isReady(unsigned long &ulTimer, uint32_t milliseconds) {
  if (millis() - ulTimer >= milliseconds) {
    ulTimer = millis();
    return true;
  }
  return false;
}

// ========== HÀM ĐIỀU KHIỂN ĐÈN ==========
void setTrafficLight(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_YELLOW, yellow ? HIGH :  LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
}

// ========== HÀM SETUP ==========
void setup() {
  Serial.begin(115200);
  
  // Cấu hình chân OUTPUT
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  // Tắt tất cả đèn ban đầu
  setTrafficLight(false, false, false);
  
  Serial.println("=================================");
  Serial.println("   HỆ THỐNG ĐÈN GIAO THÔNG");
  Serial.println("=================================");
  Serial.println("Đỏ: 5s | Vàng: 2s | Xanh: 5s");
  Serial.println("=================================");
  
  // Khởi động với đèn đỏ
  currentState = RED_LIGHT;
  setTrafficLight(true, false, false);
  Serial.println("🔴 ĐÈN ĐỎ - DỪNG LẠI!");
}

// ========== HÀM LOOP ==========
void loop() {
  switch (currentState) {
    
    case RED_LIGHT:
      // Đèn đỏ sáng trong 5 giây
      if (isReady(ulTimer, RED_DURATION)) {
        currentState = GREEN_LIGHT;
        setTrafficLight(false, false, true);
        Serial.println("🟢 ĐÈN XANH - ĐI!");
      }
      break;
    
    case GREEN_LIGHT:
      // Đèn xanh sáng trong 5 giây
      if (isReady(ulTimer, GREEN_DURATION)) {
        currentState = YELLOW_LIGHT;
        setTrafficLight(false, true, false);
        Serial.println("🟡 ĐÈN VÀNG - CHÚ Ý!");
      }
      break;
    
    case YELLOW_LIGHT: 
      // Đèn vàng sáng trong 2 giây
      if (isReady(ulTimer, YELLOW_DURATION)) {
        currentState = RED_LIGHT;
        setTrafficLight(true, false, false);
        Serial.println("🔴 ĐÈN ĐỎ - DỪNG LẠI!");
      }
      break;
  }
}
