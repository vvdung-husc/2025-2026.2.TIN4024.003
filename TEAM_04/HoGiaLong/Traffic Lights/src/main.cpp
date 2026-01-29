#include <Arduino.h>
#include <TM1637Display.h>

// --- Định nghĩa chân kết nối ---
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_STREET  21  
#define BTN_PIN     22  
#define LDR_PIN     14  

// Chân màn hình TM1637
#define CLK 18
#define DIO 19

// --- Cấu hình thời gian & Ngưỡng ---
const unsigned long TIME_RED = 10000;
const unsigned long TIME_GREEN = 10000;
const unsigned long TIME_YELLOW = 3000;
const int LDR_THRESHOLD = 2000; // Ngưỡng xác định sáng/tối (Tùy chỉnh)

// --- Khởi tạo đối tượng màn hình ---
TM1637Display display(CLK, DIO);

// --- Các biến trạng thái ---
enum TrafficState { STATE_RED, STATE_GREEN, STATE_YELLOW };
TrafficState currentState = STATE_RED;

unsigned long stateStartTime = 0;
unsigned long currentDuration = TIME_RED;
bool isNightMode = false; // Biến theo dõi trạng thái Đêm/Ngày

// Biến cho đèn vàng nhấp nháy ban đêm
unsigned long lastBlinkTime = 0;
bool blinkState = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_STREET, OUTPUT);

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(0x0f);
  
  // Khởi động trạng thái ban đầu
  changeState(STATE_RED);
}

// Hàm chuyển đổi trạng thái đèn giao thông
void changeState(TrafficState newState) {
  currentState = newState;
  stateStartTime = millis();

  // Reset đèn
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  switch (currentState) {
    case STATE_RED:
      digitalWrite(LED_RED, HIGH);
      currentDuration = TIME_RED;
      break;
    case STATE_GREEN:
      digitalWrite(LED_GREEN, HIGH);
      currentDuration = TIME_GREEN;
      break;
    case STATE_YELLOW:
      digitalWrite(LED_YELLOW, HIGH);
      currentDuration = TIME_YELLOW;
      break;
  }
}

// Hàm xử lý chế độ ban ngày (Hoạt động bình thường)
void runDayMode() {
  // 1. Nếu vừa chuyển từ Đêm -> Ngày, reset lại từ đầu
  if (isNightMode) {
    isNightMode = false;
    changeState(STATE_RED); // Reset về đèn đỏ cho an toàn
    Serial.println("--- CHUYỂN SANG CHẾ ĐỘ NGÀY ---");
  }

  // 2. Logic đèn giao thông bình thường
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - stateStartTime;

  int remainingSeconds = (currentDuration - elapsedTime) / 1000;
  if (remainingSeconds < 0) remainingSeconds = 0;
  display.showNumberDec(remainingSeconds, false, 2, 2); 

  // Xử lý nút nhấn (Chỉ hoạt động ban ngày)
  if (digitalRead(BTN_PIN) == LOW && currentState == STATE_GREEN && elapsedTime > 1000) {
    changeState(STATE_YELLOW);
    return;
  }

  // Chuyển trạng thái khi hết giờ
  if (elapsedTime >= currentDuration) {
    switch (currentState) {
      case STATE_RED:    changeState(STATE_GREEN); break;
      case STATE_GREEN:  changeState(STATE_YELLOW); break;
      case STATE_YELLOW: changeState(STATE_RED); break;
    }
  }
}

// Hàm xử lý chế độ ban đêm (Đèn đường ON, Đèn vàng nhấp nháy)
void runNightMode() {
  if (!isNightMode) {
    isNightMode = true;
    Serial.println("--- CHUYỂN SANG CHẾ ĐỘ ĐÊM ---");
    // Tắt hết đèn tín hiệu và màn hình khi bắt đầu vào đêm
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    display.clear(); 
  }

  // Nhấp nháy đèn vàng mỗi 500ms
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= 500) {
    lastBlinkTime = currentMillis;
    blinkState = !blinkState;
    digitalWrite(LED_YELLOW, blinkState ? HIGH : LOW);
  }
  
  // Đảm bảo đèn Đỏ và Xanh luôn tắt
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void loop() {
  // Đọc cảm biến ánh sáng
  int ldrValue = analogRead(LDR_PIN);
  
  // Kiểm tra ngưỡng Sáng/Tối
  // LƯU Ý: Giá trị này phụ thuộc vào cách mắc mạch và môi trường. 
  // Thường thì: Giá trị cao = Tối, Giá trị thấp = Sáng.
  
  if (ldrValue > LDR_THRESHOLD) { 
    // === TRỜI TỐI ===
    digitalWrite(LED_STREET, HIGH); // Bật đèn đường
    runNightMode();                 // Chạy chế độ cảnh báo đêm
  } else {
    // === TRỜI SÁNG ===
    digitalWrite(LED_STREET, LOW);  // Tắt đèn đường
    runDayMode();                   // Chạy chế độ giao thông thường
  }
  
  delay(50); // Delay nhỏ
}