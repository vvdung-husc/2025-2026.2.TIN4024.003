#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân cắm
#define RED_LED 25
#define YELLOW_LED 33
#define GREEN_LED 32
#define BLUE_STATUS_LED 21 
#define BUTTON_PIN 23    

#define CLK 15            
#define DIO 2             

TM1637Display display(CLK, DIO);

// Thời gian cấu hình
const int GREEN_TIME = 5;
const int YELLOW_TIME = 2; // Thường đèn vàng sẽ ngắn hơn
const int RED_TIME = 5;

// Biến quản lý
bool isDisplayOn = true; 
unsigned long previousMillis = 0;
int state = 2; // Bắt đầu từ 2 (Xanh) theo yêu cầu của bạn
int countdown = GREEN_TIME; 
const long interval = 1000; 

void updateTrafficLights();

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_STATUS_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(0x0f);
  updateTrafficLights();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Xử lý nút bấm (Bật/Tắt màn hình và đèn báo)
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isDisplayOn = !isDisplayOn; 
    delay(50); 
  }
  lastButtonState = currentButtonState;

  // Cập nhật đèn báo trạng thái theo màn hình
  digitalWrite(BLUE_STATUS_LED, isDisplayOn ? HIGH : LOW);

  // 2. Logic đếm ngược và chuyển trạng thái
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    countdown--;

    if (countdown < 0) {
      // Logic chuyển trạng thái: Xanh (2) -> Vàng (1) -> Đỏ (0)
      if (state == 2) { // Đang Xanh
        state = 1;      // Sang Vàng
        countdown = YELLOW_TIME;
      } 
      else if (state == 1) { // Đang Vàng
        state = 0;           // Sang Đỏ
        countdown = RED_TIME;
      } 
      else if (state == 0) { // Đang Đỏ
        state = 2;           // Quay lại Xanh
        countdown = GREEN_TIME;
      }
      
      updateTrafficLights();
    }

    // 3. Hiển thị lên TM1637
    if (isDisplayOn) {
      display.showNumberDec(countdown);
    } else {
      display.clear();
    }
  }
}

void updateTrafficLights() {
  // Tắt tất cả
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  // Bật đèn theo trạng thái mới
  switch (state) {
    case 0: digitalWrite(RED_LED, HIGH); break;    // Đỏ
    case 1: digitalWrite(YELLOW_LED, HIGH); break; // Vàng
    case 2: digitalWrite(GREEN_LED, HIGH); break;  // Xanh
  }
}