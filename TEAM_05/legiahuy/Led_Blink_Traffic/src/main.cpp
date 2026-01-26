#include <Arduino.h>

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21   // Đèn báo hiệu (người đi bộ/âm thanh)
#define BTN_PIN     23   // Nút nhấn
#define MIC_PIN     34   // Chân Analog đọc cảm biến âm thanh (Biến trở)

#define CLK 18
#define DIO 19


const int SOUND_THRESHOLD = 2000; 

void setup() {
  // Cấu hình các chân LED là Output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // Cấu hình nút nhấn (INPUT_PULLUP: không nhấn = HIGH, nhấn = LOW)
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // Cấu hình chân mic
  pinMode(MIC_PIN, INPUT);

}

// Hàm kiểm tra các cảm biến (Nút nhấn & Âm thanh)
void checkSensors() {
  // 1. Đọc nút nhấn
  bool isButtonPressed = (digitalRead(BTN_PIN) == LOW);

  // 2. Đọc cảm biến âm thanh (Giả lập bằng biến trở trong Wokwi)
  // Giá trị analog từ 0 đến 4095
  int soundLevel = analogRead(MIC_PIN);
  bool isLoud = (soundLevel > SOUND_THRESHOLD);

  // Nếu nhấn nút HOẶC ồn -> Bật đèn Blue
  if (isButtonPressed || isLoud) {
    digitalWrite(LED_BLUE, HIGH);
  } else {
    digitalWrite(LED_BLUE, LOW);
  }
}

// Hàm chạy một pha đèn giao thông
// ledPin: Chân đèn cần bật
// seconds: Thời gian sáng (giây)
void trafficPhase(int ledPin, int seconds) {
  digitalWrite(ledPin, HIGH); // Bật đèn hiện tại

  // Vòng lặp đếm ngược từng giây
  for (int i = seconds; i >= 0; i--) {

    // Trong 1 giây chờ đợi (delay 1000ms), ta chia nhỏ ra
    // để kiểm tra nút bấm liên tục (giúp phản hồi nhạy hơn)
    for (int j = 0; j < 100; j++) {
      checkSensors(); // Kiểm tra nút/mic
      delay(10);      // 10ms * 100 lần = 1000ms = 1 giây
    }
  }

  digitalWrite(ledPin, LOW); // Tắt đèn sau khi hết giờ
}

void loop() {
  // 1. Đèn ĐỎ sáng 10 giây
  trafficPhase(LED_RED, 10);

  // 2. Đèn XANH sáng 10 giây
  trafficPhase(LED_GREEN, 10);

  // 3. Đèn VÀNG sáng 3 giây
  trafficPhase(LED_YELLOW, 3);
}