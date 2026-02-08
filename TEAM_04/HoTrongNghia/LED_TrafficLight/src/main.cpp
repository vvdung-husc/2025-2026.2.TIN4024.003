#include <Arduino.h>

#define LED_RED_PIN 12
#define LED_YELLOW_PIN 23
#define LED_GREEN_PIN 18


//Non-blocking

// Định nghĩa các trạng thái cho dễ đọc
#define STATE_RED 0
#define STATE_GREEN 1
#define STATE_YELLOW 2

// Thời gian cho từng đèn (miliseconds)
const unsigned long TIME_RED = 2000;
const unsigned long TIME_GREEN = 1000;
const unsigned long TIME_YELLOW = 1000;

// Biến quản lý trạng thái
int currentState = STATE_RED;
unsigned long lastSwitchTime = 0;

void setup() {
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("Traffic Light Started (Non-blocking)");
  
  // Khởi động trạng thái đầu tiên
  digitalWrite(LED_RED_PIN, HIGH);
  lastSwitchTime = millis();
}
// Hàm phụ trợ để chuyển trạng thái và điều khiển đèn
void changeState(int newState) {
  currentState = newState;
  lastSwitchTime = millis(); // Reset đồng hồ

  // Reset tất cả đèn về LOW trước để tránh lỗi chồng đèn
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);

  // Bật đèn tương ứng với trạng thái mới
  if (currentState == STATE_RED) {
    digitalWrite(LED_RED_PIN, HIGH);
    Serial.println("RED ON");
  } 
  else if (currentState == STATE_GREEN) {
    digitalWrite(LED_GREEN_PIN, HIGH);
    Serial.println("GREEN ON");
  } 
  else if (currentState == STATE_YELLOW) {
    digitalWrite(LED_YELLOW_PIN, HIGH);
    Serial.println("YELLOW ON");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    case STATE_RED:
      // Nếu đã hết thời gian Đỏ -> Chuyển sang Xanh
      if (currentMillis - lastSwitchTime >= TIME_RED) {
        changeState(STATE_GREEN);
      }
      break;

    case STATE_GREEN:
      // Nếu đã hết thời gian Xanh -> Chuyển sang Vàng
      if (currentMillis - lastSwitchTime >= TIME_GREEN) {
        changeState(STATE_YELLOW);
      }
      break;

    case STATE_YELLOW:
      // Nếu đã hết thời gian Vàng -> Quay lại Đỏ
      if (currentMillis - lastSwitchTime >= TIME_YELLOW) {
        changeState(STATE_RED);
      }
      break;
  }
}



// BLOCKING

// void setup() {
//   pinMode(LED_RED_PIN, OUTPUT);
//   pinMode(LED_YELLOW_PIN, OUTPUT);
//   pinMode(LED_GREEN_PIN, OUTPUT);
//   Serial.begin(115200);
//   Serial.println("Hello, Wokwi!");
// }
// void loop() {
//   digitalWrite(LED_RED_PIN, HIGH); // Bật đèn đỏ
//   digitalWrite(LED_YELLOW_PIN, LOW);
//   digitalWrite(LED_GREEN_PIN, LOW);
//   delay(20000); // trong vòng 20 giây
//   digitalWrite(LED_RED_PIN, LOW); 
//   digitalWrite(LED_YELLOW_PIN, LOW);
//   digitalWrite(LED_GREEN_PIN, HIGH); // Bật đèn xanh
//   delay(10000);
//   digitalWrite(LED_RED_PIN, LOW); 
//   digitalWrite(LED_YELLOW_PIN, HIGH); // Bật đèn vàng
//   digitalWrite(LED_GREEN_PIN, LOW); 
//   delay(3000);
// }