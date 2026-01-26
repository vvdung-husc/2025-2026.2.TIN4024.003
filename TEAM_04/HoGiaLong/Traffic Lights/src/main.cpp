#include <Arduino.h>
#include <TM1637Display.h>

// --- Định nghĩa chân kết nối (Phải khớp với file diagram.json) ---
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_STREET  21  // Đèn xanh dương (giả lập đèn đường)
#define BTN_PIN     22  // Nút nhấn
#define LDR_PIN     14  // Chân Analog của cảm biến ánh sáng

// Chân màn hình TM1637
#define CLK 18
#define DIO 19

// --- Cấu hình thời gian (đơn vị mili giây) ---
const unsigned long TIME_RED = 10000;
const unsigned long TIME_GREEN = 10000;
const unsigned long TIME_YELLOW = 3000;

// --- Khởi tạo đối tượng màn hình ---
TM1637Display display(CLK, DIO);

// --- Các biến trạng thái ---
enum TrafficState { STATE_RED, STATE_GREEN, STATE_YELLOW };
TrafficState currentState = STATE_RED;
unsigned long stateStartTime = 0;
unsigned long currentDuration = TIME_RED;

void changeState(TrafficState newState) {
  currentState = newState;
  stateStartTime = millis();

  // Tắt hết các đèn trước khi bật đèn mới
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  switch (currentState) {
    case STATE_RED:
      digitalWrite(LED_RED, HIGH);
      currentDuration = TIME_RED;
      Serial.println("Trạng thái: ĐÈN ĐỎ");
      break;
    case STATE_GREEN:
      digitalWrite(LED_GREEN, HIGH);
      currentDuration = TIME_GREEN;
      Serial.println("Trạng thái: ĐÈN XANH");
      break;
    case STATE_YELLOW:
      digitalWrite(LED_YELLOW, HIGH);
      currentDuration = TIME_YELLOW;
      Serial.println("Trạng thái: ĐÈN VÀNG");
      break;
  }
}  

void setup() {
  Serial.begin(115200);

  // Cấu hình chân LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_STREET, OUTPUT);

  // Cấu hình nút nhấn và cảm biến
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  // Cấu hình màn hình (độ sáng tối đa)
  display.setBrightness(0x0f);
  
  // Khởi động trạng thái ban đầu
  changeState(STATE_RED);
}

void loop() {
  // 1. Xử lý Logic Đèn Giao Thông
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - stateStartTime;

  // Tính thời gian còn lại (giây) để hiển thị
  int remainingSeconds = (currentDuration - elapsedTime) / 1000;
  if (remainingSeconds < 0) remainingSeconds = 0;

  // Hiển thị lên màn hình 7 thanh (2 số cuối)
  display.showNumberDec(remainingSeconds, false, 2, 2); 

  // Kiểm tra nút nhấn (Ưu tiên người đi bộ)
  // Nếu đang đèn Xanh mà nhấn nút -> Chuyển nhanh sang Vàng
  if (digitalRead(BTN_PIN) == LOW && currentState == STATE_GREEN && elapsedTime > 1000) {
    Serial.println("Nút nhấn: Người đi bộ xin qua đường!");
    changeState(STATE_YELLOW);
    return; // Thoát vòng lặp hiện tại để xử lý ngay
  }

  // Chuyển trạng thái khi hết thời gian
  if (elapsedTime >= currentDuration) {
    switch (currentState) {
      case STATE_RED:
        changeState(STATE_GREEN);
        break;
      case STATE_GREEN:
        changeState(STATE_YELLOW);
        break;
      case STATE_YELLOW:
        changeState(STATE_RED);
        break;
    }
  }

  // 2. Xử lý Cảm biến ánh sáng (Bật đèn đường khi trời tối)
  int ldrValue = analogRead(LDR_PIN);
  // Wokwi LDR: Giá trị cao = Tối, Giá trị thấp = Sáng (hoặc ngược lại tùy module, check Serial)
  // Trong thực tế và Wokwi thường: 
  // Sáng: Value thấp (~500-1000), Tối: Value cao (~3000-4000) hoặc ngược lại tùy cách mắc điện trở kéo.
  // Với sơ đồ hiện tại, ta giả định ngưỡng để bật đèn.
  
  // Debug giá trị cảm biến ra Serial Monitor để cân chỉnh
  // Serial.println(ldrValue); 

  if (ldrValue > 2000) { // Giả sử > 2000 là trời tối (tuỳ chỉnh số này)
    digitalWrite(LED_STREET, HIGH); // Bật đèn đường
  } else {
    digitalWrite(LED_STREET, LOW);  // Tắt đèn đường
  }
  
  delay(100); // Delay nhỏ để giảm tải CPU
}
