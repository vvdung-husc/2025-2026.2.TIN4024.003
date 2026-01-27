#include <Arduino.h>

#include <TM1637Display.h>

// Định nghĩa chân
#define CLK 18
#define DIO 19
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25

TM1637Display display(CLK, DIO);

// Các trạng thái của đèn
enum TrafficState {
  STATE_RED,
  STATE_GREEN,
  STATE_YELLOW
};

TrafficState currentState = STATE_RED; 

// Biến quản lý thời gian
unsigned long stateStartTime = 0;
int lastShownSecond = -1;

// Thời gian (miligiây)
const unsigned long RED_TIME = 3000;
const unsigned long GREEN_TIME = 2000;
const unsigned long YELLOW_TIME = 2000;

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  display.setBrightness(0x0f);
  
  // BẮT ĐẦU: Bật đèn Đỏ trước tiên, tắt các đèn khác
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  
  currentState = STATE_RED;
  stateStartTime = millis(); 
}

// Hàm tiện ích: Tắt đèn cũ và Bật đèn mới ngay lập tức
void switchLight(int pinOff, int pinOn) {
  digitalWrite(pinOff, LOW); // Tắt đèn vừa hết giờ
  digitalWrite(pinOn, HIGH); // Bật đèn tiếp theo
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - stateStartTime;
  unsigned long currentDuration;

  // 1. XỬ LÝ CHUYỂN TRẠNG THÁI
  // Logic: Kiểm tra nếu hết giờ -> Tắt đèn hiện tại -> Bật đèn kế tiếp -> Reset giờ
  switch (currentState) {
    case STATE_RED:
      currentDuration = RED_TIME;
      if (elapsedTime >= RED_TIME) {
        // Hết giờ Đỏ -> Sang Xanh
        switchLight(LED_RED, LED_GREEN); 
        currentState = STATE_GREEN;
        stateStartTime = currentMillis;
        elapsedTime = 0; 
        lastShownSecond = -1; // Reset để cập nhật màn hình ngay
      }
      break;

    case STATE_GREEN:
      currentDuration = GREEN_TIME;
      if (elapsedTime >= GREEN_TIME) {
        // Hết giờ Xanh -> Sang Vàng
        switchLight(LED_GREEN, LED_YELLOW);
        currentState = STATE_YELLOW;
        stateStartTime = currentMillis;
        elapsedTime = 0;
        lastShownSecond = -1;
      }
      break;

    case STATE_YELLOW:
      currentDuration = YELLOW_TIME;
      if (elapsedTime >= YELLOW_TIME) {
        // Hết giờ Vàng -> Quay lại Đỏ
        switchLight(LED_YELLOW, LED_RED);
        currentState = STATE_RED;
        stateStartTime = currentMillis;
        elapsedTime = 0;
        lastShownSecond = -1;
      }
      break;
  }

  // 2. XỬ LÝ HIỂN THỊ MÀN HÌNH (Đã tối ưu, không spam lệnh)
  // Tính số giây còn lại
  int remainingSeconds = (currentDuration - elapsedTime) / 1000;
  
  // Tránh hiển thị số âm (nếu vi xử lý chạy lố vài mili giây)
  if (remainingSeconds < 0) remainingSeconds = 0;

  // Chỉ gửi lệnh ra màn hình khi con số thực sự thay đổi (Ví dụ: từ 3 xuống 2)
  if (remainingSeconds != lastShownSecond) {
    display.showNumberDec(remainingSeconds);
    lastShownSecond = remainingSeconds;
  }
}