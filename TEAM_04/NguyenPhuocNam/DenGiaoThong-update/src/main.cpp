#include <Arduino.h>
#include <TM1637Display.h>

// --- 1. KHAI BÁO CHÂN (Khớp với file diagram.json) ---
#define CLK_PIN     18  // Chân Clock màn hình
#define DIO_PIN     19  // Chân Data màn hình
#define LED_RED     27  // Đèn Đỏ
#define LED_YELLOW  26  // Đèn Vàng
#define LED_GREEN   25  // Đèn Xanh
#define LED_STREET  21  // Đèn đường (Blue LED)
#define BTN_PIN     4   // Nút nhấn
#define LDR_PIN     13  // Cảm biến ánh sáng

// --- 2. CẤU HÌNH THỜI GIAN (Giây) ---
const int TIME_GREEN  = 5;
const int TIME_YELLOW = 2;
const int TIME_RED    = 5;

// Ngưỡng ánh sáng (Wokwi: < 1000 là tối, > 1000 là sáng)
const int LIGHT_THRESHOLD = 1000; 

// --- 3. KHỞI TẠO ĐỐI TƯỢNG ---
TM1637Display display(CLK_PIN, DIO_PIN);
const uint8_t DASHES[] = { SEG_G, SEG_G, SEG_G, SEG_G }; // Ký tự "----"

// --- 4. BIẾN TRẠNG THÁI TOÀN CỤC ---
enum TrafficState { STATE_GREEN, STATE_YELLOW, STATE_RED };
TrafficState currentState = STATE_GREEN;

unsigned long lastTimerUpdate = 0; // Bộ đếm giây
unsigned long lastBlinkUpdate = 0; // Bộ đếm nháy đèn vàng
int secondsLeft = TIME_GREEN;      // Thời gian còn lại
bool isYellowOn = false;           // Trạng thái nháy đèn

// --- 5. KHAI BÁO HÀM (PROTOTYPES) ---
void runDayMode();
void runNightMode();
void updateTrafficLights(int r, int y, int g);
void checkPedestrianButton(); // <--- Hàm nút bấm bạn yêu cầu

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  // Cấu hình Output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_STREET, OUTPUT);

  // Cấu hình Input
  pinMode(BTN_PIN, INPUT_PULLUP); // Quan trọng: Kéo trở nội bộ
  pinMode(LDR_PIN, INPUT);

  // Cấu hình màn hình
  display.setBrightness(0x0f); 
  display.showNumberDec(8888); // Test màn hình lúc khởi động
  delay(1000);
}

// --- LOOP ---
void loop() {
  int lightValue = analogRead(LDR_PIN);

  // Kiểm tra chế độ Ngày/Đêm
  if (lightValue < LIGHT_THRESHOLD) {
    runNightMode();
  } else {
    runDayMode();
  }
}

// ---------------------------------------------------------
// --- ĐỊNH NGHĨA CÁC HÀM CHI TIẾT ---
// ---------------------------------------------------------

// 1. Hàm điều khiển bật tắt 3 đèn giao thông nhanh
void updateTrafficLights(int r, int y, int g) {
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

// 2. Hàm xử lý nút bấm (Đã tách riêng)
void checkPedestrianButton() {
  // Nếu nút được bấm (trạng thái LOW do dùng INPUT_PULLUP)
  if (digitalRead(BTN_PIN) == LOW) {
    // Chỉ có tác dụng khi đang ở đèn Xanh
    if (currentState == STATE_GREEN) {
      // Nếu thời gian còn nhiều hơn 2s thì rút xuống còn 2s
      if (secondsLeft > 2) {
        secondsLeft = 2;
        display.showNumberDec(secondsLeft, true); // Cập nhật màn hình ngay
        Serial.println(">> NUT BAM: Uu tien nguoi di bo -> Rut ngan den Xanh!");
      }
    } else {
        // In ra để debug chơi thôi
        Serial.println(">> NUT BAM: Khong tac dung (Dang den Do hoac Vang)");
    }
    delay(200); // Chống dội phím (Debounce)
  }
}

// 3. Logic Ban Ngày (Chạy đèn giao thông bình thường)
void runDayMode() {
  digitalWrite(LED_STREET, LOW); // Tắt đèn đường

  // Đếm ngược thời gian (Non-blocking delay)
  if (millis() - lastTimerUpdate >= 1000) {
    lastTimerUpdate = millis();
    secondsLeft--;

    if (secondsLeft < 0) {
      // Chuyển trạng thái
      switch (currentState) {
        case STATE_GREEN:
          currentState = STATE_YELLOW;
          secondsLeft = TIME_YELLOW;
          break;
        case STATE_YELLOW:
          currentState = STATE_RED;
          secondsLeft = TIME_RED;
          break;
        case STATE_RED:
          currentState = STATE_GREEN;
          secondsLeft = TIME_GREEN;
          break;
      }
    }
    // Hiển thị thời gian
    display.showNumberDec(secondsLeft, true);
  }

  // Điều khiển đèn LED theo trạng thái
  switch (currentState) {
    case STATE_GREEN:
      updateTrafficLights(LOW, LOW, HIGH);
      break;
    case STATE_YELLOW:
      updateTrafficLights(LOW, HIGH, LOW);
      break;
    case STATE_RED:
      updateTrafficLights(HIGH, LOW, LOW);
      break;
  }

  // --- GỌI HÀM KIỂM TRA NÚT BẤM ---
  checkPedestrianButton();
}

// 4. Logic Ban Đêm (Đèn đường sáng, đèn vàng nhấp nháy)
void runNightMode() {
  digitalWrite(LED_STREET, HIGH); // Bật đèn đường
  display.setSegments(DASHES);    // Hiện "----"

  // Nhấp nháy đèn vàng mỗi 500ms
  if (millis() - lastBlinkUpdate >= 500) {
    lastBlinkUpdate = millis();
    isYellowOn = !isYellowOn;
    updateTrafficLights(LOW, isYellowOn ? HIGH : LOW, LOW);
  }

  // Reset trạng thái về Xanh để khi trời sáng lại thì bắt đầu đúng nhịp
  currentState = STATE_GREEN;
  secondsLeft = TIME_GREEN;
}