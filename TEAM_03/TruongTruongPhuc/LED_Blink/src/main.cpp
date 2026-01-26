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
const uint32_t BLINK_INTERVAL = 250;     // Nhấp nháy mỗi 250ms

// ========== TRẠNG THÁI ĐÈN GIAO THÔNG ==========
enum TrafficLightState {
  RED_LIGHT,
  YELLOW_LIGHT,
  GREEN_LIGHT
};

// ========== BIẾN TOÀN CỤC ==========
TrafficLightState currentState = RED_LIGHT;
unsigned long ulStateTimer = 0;    // Timer cho chuyển trạng thái
unsigned long ulBlinkTimer = 0;    // Timer cho nhấp nháy
bool bLEDStatus = false;

// ========== HÀM TIMER KHÔNG CHẶN ==========
bool isReady(unsigned long &ulTimer, uint32_t milliseconds) {
  if (millis() - ulTimer >= milliseconds) {
    ulTimer = millis();
    return true;
  }
  return false;
}

// ========== HÀM TẮT TẤT CẢ ĐÈN ==========
void turnOffAllLEDs() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// ========== HÀM SETUP ==========
void setup() {
  Serial.begin(115200);
  
  // Cấu hình chân OUTPUT
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  // Tắt tất cả đèn ban đầu
  turnOffAllLEDs();
  
  Serial.println("=================================");
  Serial.println("   ĐÈN GIAO THÔNG NHẤP NHÁY");
  Serial.println("=================================");
  Serial.println("Đỏ: 5s | Vàng: 2s | Xanh: 5s");
  Serial.println("=================================");
  
  // Khởi động với đèn đỏ
  currentState = RED_LIGHT;
  Serial.println("🔴 ĐÈN ĐỎ NHẤP NHÁY - DỪNG LẠI!");
}

// ========== HÀM LOOP ==========
void loop() {
  // Xử lý nhấp nháy
  if (isReady(ulBlinkTimer, BLINK_INTERVAL)) {
    bLEDStatus = !bLEDStatus;
    
    // Nhấp nháy đèn theo trạng thái hiện tại
    turnOffAllLEDs();
    if (bLEDStatus) {
      switch (currentState) {
        case RED_LIGHT:
          digitalWrite(LED_RED, HIGH);
          break;
        case YELLOW_LIGHT:
          digitalWrite(LED_YELLOW, HIGH);
          break;
        case GREEN_LIGHT:
          digitalWrite(LED_GREEN, HIGH);
          break;
      }
    }
  }
  
  // Xử lý chuyển trạng thái
  switch (currentState) {
    case RED_LIGHT:
      if (isReady(ulStateTimer, RED_DURATION)) {
        currentState = GREEN_LIGHT;
        Serial.println("🟢 ĐÈN XANH NHẤP NHÁY - ĐI!");
      }
      break;
    
    case GREEN_LIGHT:
      if (isReady(ulStateTimer, GREEN_DURATION)) {
        currentState = YELLOW_LIGHT;
        Serial.println("🟡 ĐÈN VÀNG NHẤP NHÁY - CHÚ Ý!");
      }
      break;
    
    case YELLOW_LIGHT:
      if (isReady(ulStateTimer, YELLOW_DURATION)) {
        currentState = RED_LIGHT;
        Serial.println("🔴 ĐÈN ĐỎ NHẤP NHÁY - DỪNG LẠI!");
      }
      break;
  }
}