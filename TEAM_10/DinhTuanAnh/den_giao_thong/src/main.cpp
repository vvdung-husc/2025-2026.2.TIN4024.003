#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân cắm theo diagram của bạn
#define RED_LED 25
#define YELLOW_LED 33
#define GREEN_LED 32
#define BLUE_STATUS_LED 21 // Đèn báo hiệu trạng thái nút bấm
#define BUTTON_PIN 23    

#define CLK 15           
#define DIO 2            

TM1637Display display(CLK, DIO);

// Cấu hình thời gian cho từng loại đèn
const int RED_TIME = 5;
const int YELLOW_TIME = 5;
const int GREEN_TIME = 5;

// Biến quản lý
bool isDisplayOn = true; 
unsigned long previousMillis = 0;
int countdown = RED_TIME;
int state = 0; // 0: Đỏ, 1: Vàng, 2: Xanh
const long interval = 1000; 

// Khai báo nguyên mẫu hàm
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

  // 1. Xử lý nút bấm: Nhấn để bật/tắt màn hình
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isDisplayOn = !isDisplayOn; 
    delay(50); // Chống rung phím
  }
  lastButtonState = currentButtonState;

  // 2. Đèn xanh đơn báo hiệu trạng thái nút bấm (Màn hình bật -> Đèn sáng)
  digitalWrite(BLUE_STATUS_LED, isDisplayOn ? HIGH : LOW);

  // 3. Logic đếm ngược và chuyển trạng thái đèn giao thông
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    countdown--;

    if (countdown < 0) {
      state = (state + 1) % 3; // Chuyển vòng: Đỏ -> Vàng -> Xanh
      
      // Thiết lập lại thời gian cho từng trạng thái
      if (state == 0) countdown = RED_TIME;
      else if (state == 1) countdown = YELLOW_TIME;
      else if (state == 2) countdown = GREEN_TIME;
      
      updateTrafficLights();
    }

    // 4. Hiển thị lên màn hình TM1637
    if (isDisplayOn) {
      display.showNumberDec(countdown);
    } else {
      display.clear(); // Tắt màn hình khi nhấn nút
    }
  }
}

// Hàm điều khiển bật/tắt các đèn giao thông
void updateTrafficLights() {
  // Tắt tất cả các đèn chính
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  // Bật đèn tương ứng với trạng thái hiện tại
  switch (state) {
    case 0: digitalWrite(RED_LED, HIGH); break;
    case 1: digitalWrite(YELLOW_LED, HIGH); break;
    case 2: digitalWrite(GREEN_LED, HIGH); break;
  }
}