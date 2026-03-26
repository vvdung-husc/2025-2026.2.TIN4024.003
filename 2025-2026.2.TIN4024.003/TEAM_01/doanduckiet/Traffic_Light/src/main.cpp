#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa các chân dựa trên diagram.json
#define PIN_RED 27
#define PIN_YELLOW 26
#define PIN_GREEN 25
#define PIN_BLUE 21     // Đèn cho người đi bộ
#define PIN_BUTTON 23   // Nút bấm xin sang đường
#define PIN_LDR 13      // Cảm biến ánh sáng quang trở
#define PIN_CLK 18      // Màn hình LED CLK
#define PIN_DIO 19      // Màn hình LED DIO

// Khởi tạo đối tượng màn hình
TM1637Display display(PIN_CLK, PIN_DIO);

bool pedestrianRequest = false;

void setup() {
  Serial.begin(115200);

  // Cài đặt các chân LED là OUTPUT
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  // Nút bấm dùng điện trở kéo lên nội bộ (nhấn xuống sẽ là LOW)
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  // LDR là đầu vào Analog
  pinMode(PIN_LDR, INPUT);

  // Cài đặt độ sáng màn hình (0x00 đến 0x0f)
  display.setBrightness(0x0f); 
}

// Hàm đếm ngược và kiểm tra nút bấm cùng lúc
void waitSeconds(int seconds) {
  for (int i = seconds; i > 0; i--) {
    // Hiển thị số giây lên màn hình
    display.showNumberDec(i, false, 2, 2); 
    
    // Chia nhỏ 1 giây thành 10 phần để bắt nút bấm nhạy hơn
    for (int j = 0; j < 10; j++) {
      if (digitalRead(PIN_BUTTON) == LOW) {
        pedestrianRequest = true;
      }
      delay(100);
    }
  }
}

void loop() {
  // 1. Đọc và in giá trị cảm biến ánh sáng
  int lightLevel = analogRead(PIN_LDR);
  Serial.print("Muc do anh sang (LDR): ");
  Serial.println(lightLevel);

  // ==========================================
  // PHA 1: ĐÈN XANH LÁ (Bình thường 10 giây)
  // ==========================================
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_YELLOW, LOW);
  digitalWrite(PIN_GREEN, HIGH);
  digitalWrite(PIN_BLUE, LOW); // Tắt đèn người đi bộ

  // Nếu có người đã bấm nút từ trước, ưu tiên chuyển đèn xanh chỉ còn 3 giây
  int greenTime = pedestrianRequest ? 3 : 10;
  pedestrianRequest = false; // Xóa trạng thái nút bấm
  waitSeconds(greenTime);

  // ==========================================
  // PHA 2: ĐÈN VÀNG (3 giây)
  // ==========================================
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_YELLOW, HIGH);
  waitSeconds(3);

  // ==========================================
  // PHA 3: ĐÈN ĐỎ (7 giây)
  // ==========================================
  digitalWrite(PIN_YELLOW, LOW);
  digitalWrite(PIN_RED, HIGH);
  digitalWrite(PIN_BLUE, HIGH); // Bật đèn Báo hiệu an toàn cho người đi bộ
  
  waitSeconds(7);
}