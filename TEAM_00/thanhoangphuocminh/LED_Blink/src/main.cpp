#include <Arduino.h>

// Định nghĩa chân
#define LED_RED 17
#define POT_PIN 34 // Chân GPIO 34 nối với chân SIG của biến trở

// Biến toàn cục
unsigned long ulTimer = 0;
bool bLEDStatus = false;

// Hàm kiểm tra thời gian (Non-blocking) - Rất tốt, giữ nguyên!
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  // Khởi tạo Serial để xem log
  Serial.begin(115200); 
  printf("Welcome IoT - System Started\n");

  pinMode(LED_RED, OUTPUT); // Cấu hình chân LED ra
  pinMode(POT_PIN, INPUT);  // Cấu hình chân Biến trở vào
}

void loop() {
  // Logic: Cứ mỗi 5000ms (5 giây) thì thực hiện khối lệnh này
  // Trong lúc chờ 5s, chip hoàn toàn rảnh để làm việc khác (nếu có)
  if (IsReady(ulTimer, 5000)) {
    
    // 1. Xử lý Đèn LED (Đảo trạng thái)
    bLEDStatus = !bLEDStatus;
    digitalWrite(LED_RED, bLEDStatus ? HIGH : LOW);

    // 2. Đọc biến trở ngay lúc đó
    int potValue = analogRead(POT_PIN);

    // 3. In thông tin ra màn hình console
    if (bLEDStatus) {
      printf("-> 5s da qua: LED SANG | Gia tri bien tro: %d\n", potValue);
    } else {
      printf("-> 5s da qua: LED TAT  | Gia tri bien tro: %d\n", potValue);
    }
  }
}