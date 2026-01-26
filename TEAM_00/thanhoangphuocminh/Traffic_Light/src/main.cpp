#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN (Khớp với Diagram của bạn) ---
#define CLK 18 
#define DIO 19

#define RED_PIN    27
#define YELLOW_PIN 26
#define GREEN_PIN  25

#define BUTTON_PIN 23  // Chân nút bấm (Theo sơ đồ nối vào GPIO 23)

// Khởi tạo màn hình
TM1637Display display(CLK, DIO);

// Biến trạng thái hoạt động (False = Đang dừng, True = Đang chạy)
bool isRunning = false; 

void setup() {
  Serial.begin(115200);

  // Cấu hình chân đèn
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  // Cấu hình nút bấm (INPUT_PULLUP để không cần điện trở ngoài)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Cấu hình màn hình
  display.setBrightness(0x0f);
  
  // Ban đầu hiển thị 0000 hoặc gạch ngang để báo hiệu đang chờ
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff }; // Bật hết các thanh (8888)
  display.setSegments(data);
}

// --- HÀM KIỂM TRA NÚT BẤM ---
void checkButton() {
  // Nếu nút được nhấn (trạng thái LOW do nối đất)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Chống rung phím (Debounce)
    if (digitalRead(BUTTON_PIN) == LOW) {
      // Đảo trạng thái: Đang chạy -> Dừng, Đang dừng -> Chạy
      isRunning = !isRunning;
      Serial.println(isRunning ? "System STARTED" : "System PAUSED");
      
      // Chờ người dùng thả nút ra để không bị nhấn liên tục
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

// --- HÀM XỬ LÝ ĐÈN THÔNG MINH ---
// Hàm này thay thế delay(1000). Nó sẽ chờ 1 giây nhưng vẫn kiểm tra nút bấm liên tục
void smartDelay(int seconds) {
  for (int i = seconds; i > 0; i--) {
    // Hiển thị số giây còn lại
    display.showNumberDec(i);

    // Trong vòng 1 giây (1000ms), chia nhỏ ra để kiểm tra nút bấm
    unsigned long startMillis = millis();
    while (millis() - startMillis < 1000) {
      checkButton(); // Kiểm tra nút liên tục

      // Nếu hệ thống bị tạm dừng (isRunning = false)
      while (!isRunning) {
        checkButton(); // Vẫn phải kiểm tra nút để xem có bấm "Chạy tiếp" không
        // Có thể nhấp nháy màn hình hoặc giữ nguyên số để báo hiệu đang Pause
        // Ở đây mình giữ nguyên số đang đếm dở
      }
    }
  }
}

void loop() {
  // Nếu chưa bấm nút Bắt đầu thì không làm gì cả, tắt hết đèn
  if (!isRunning) {
    checkButton();
    // Tắt hết đèn khi chưa chạy
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    return; // Quay lại đầu hàm loop
  }

  // --- QUY TRÌNH: ĐỎ -> VÀNG -> XANH (Theo yêu cầu của bạn) ---

  // 1. ĐÈN ĐỎ (10 giây)
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  smartDelay(10); // Chờ 10s (có thể pause)

  // 2. ĐÈN VÀNG (3 giây)
  // Yêu cầu: "Đèn đỏ xong chuyển sang vàng"
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  smartDelay(3); 

  // 3. ĐÈN XANH (7 giây)
  // Yêu cầu: "Đèn vàng xong chuyển sang xanh"
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  smartDelay(7);
  
  // Sau đó vòng lặp loop() sẽ quay lại đầu -> Bật Đèn Đỏ lại.
}