#include <Arduino.h>

#include <TM1637Display.h>

// Định nghĩa chân nối theo sơ đồ "y đúc"
#define CLK 18
#define DIO 19
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25

// Khởi tạo màn hình
TM1637Display display(CLK, DIO);

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  display.setBrightness(0x0f); // Độ sáng tối đa
}

// Hàm chạy đèn: Nhận vào chân LED và thời gian (giây)
void runTrafficLight(int ledPin, int duration) {
  for (int i = duration; i >= 0; i--) {
    display.showNumberDec(i); // Hiển thị số giây còn lại
    
    // Hiệu ứng nhấp nháy trong 1 giây (4 nhịp 250ms)
    digitalWrite(ledPin, HIGH); delay(250);
    digitalWrite(ledPin, LOW);  delay(250);
  }
}

void loop() {
  // Quy trình: Đỏ -> Xanh -> Vàng -> Lặp lại
  
  // 1. Đèn Đỏ (3 giây)
  runTrafficLight(LED_RED, 3);
  
  // 2. Đèn Xanh (2 giây)
  runTrafficLight(LED_GREEN, 2);

  // 3. Đèn Vàng (2 giây)
  runTrafficLight(LED_YELLOW, 2);
}