#include <Arduino.h>
#include <TM1637Display.h>

// --- 1. KHAI BÁO CHÂN KẾT NỐI (Giống hệt file diagram.json) ---
#define CLK_PIN     18  // Chân Clock màn hình
#define DIO_PIN     19  // Chân Data màn hình
#define LED_RED     27  // Đèn Đỏ
#define LED_YELLOW  26  // Đèn Vàng
#define LED_GREEN   25  // Đèn Xanh
#define LED_STREET  21  // Đèn đường (Blue LED)
#define BTN_PIN     4   // Nút nhấn
#define LDR_PIN     13  // Cảm biến ánh sáng

// --- 2. CẤU HÌNH THÔNG SỐ ---
const int TIME_GREEN  = 5; // Thời gian đèn xanh (giây)
const int TIME_YELLOW = 2; // Thời gian đèn vàng
const int TIME_RED    = 5; // Thời gian đèn đỏ

// Ngưỡng ánh sáng (Wokwi: < 1000 là tối, > 1000 là sáng)
const int LIGHT_THRESHOLD = 1000; 

// --- 3. KHỞI TẠO ĐỐI TƯỢNG ---
TM1637Display display(CLK_PIN, DIO_PIN);

// Mảng hiển thị dấu gạch ngang "----" cho chế độ ban đêm
const uint8_t DASHES[] = { SEG_G, SEG_G, SEG_G, SEG_G };

// --- 4. CÁC BIẾN QUẢN LÝ TRẠNG THÁI ---
enum TrafficState { STATE_GREEN, STATE_YELLOW, STATE_RED };
TrafficState currentState = STATE_GREEN;

unsigned long lastTimerUpdate = 0; // Bộ đếm thời gian cho đèn giao thông
unsigned long lastBlinkUpdate = 0; // Bộ đếm cho đèn vàng nhấp nháy
int secondsLeft = TIME_GREEN;      // Giây còn lại
bool isYellowOn = false;           // Trạng thái nhấp nháy

// Khai báo hàm trước (function prototypes)
void runDayMode();
void runNightMode();
void updateTrafficLights(int redState, int yellowState, int greenState);

void setup() {
  Serial.begin(115200);

  // Cấu hình chân Output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_STREET, OUTPUT);

  // Cấu hình chân Input
  pinMode(BTN_PIN, INPUT_PULLUP); // Dùng điện trở kéo lên nội bộ
  pinMode(LDR_PIN, INPUT);

  // Cấu hình màn hình
  display.setBrightness(0x0f); // Độ sáng tối đa
}

void loop() {
  // Đọc giá trị cảm biến ánh sáng
  int lightValue = analogRead(LDR_PIN);

  // Kiểm tra Sáng hay Tối để chọn chế độ
  if (lightValue < LIGHT_THRESHOLD) {
    runNightMode(); // Chạy chế độ ban đêm
  } else {
    runDayMode();   // Chạy chế độ ban ngày
  }
}

// --- LOGIC CHẾ ĐỘ BAN NGÀY ---
void runDayMode() {
  // Tắt đèn đường
  digitalWrite(LED_STREET, LOW);

  // Xử lý đếm ngược thời gian (mỗi 1 giây)
  if (millis() - lastTimerUpdate >= 1000) {
    lastTimerUpdate = millis();
    secondsLeft--;

    // Hiển thị số giây lên màn hình
    display.showNumberDec(secondsLeft, true);

    // Hết giờ thì chuyển đèn
    if (secondsLeft < 0) {
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
  }

  // Cập nhật đèn LED theo trạng thái hiện tại
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

  // Xử lý nút bấm người đi bộ (Chỉ có tác dụng khi đang đèn xanh)
  if (digitalRead(BTN_PIN) == LOW && currentState == STATE_GREEN) {
    if (secondsLeft > 2) {
      secondsLeft = 2; // Rút ngắn thời gian đèn xanh xuống 2s
      display.showNumberDec(secondsLeft, true);
      Serial.println(">> Nguoi di bo bam nut: Rut ngan den xanh!");
      delay(250); // Chống dội phím
    }
  }
}

// --- LOGIC CHẾ ĐỘ BAN ĐÊM ---
void runNightMode() {
  // Bật đèn đường (LED Xanh dương)
  digitalWrite(LED_STREET, HIGH);

  // Hiển thị "----" trên màn hình
  display.setSegments(DASHES);

  // Nhấp nháy đèn vàng mỗi 500ms
  if (millis() - lastBlinkUpdate >= 500) {
    lastBlinkUpdate = millis();
    isYellowOn = !isYellowOn;
    
    // Đỏ tắt, Vàng nháy, Xanh tắt
    updateTrafficLights(LOW, isYellowOn ? HIGH : LOW, LOW);
  }

  // Reset trạng thái về đèn xanh để khi trời sáng thì bắt đầu đúng quy trình
  currentState = STATE_GREEN;
  secondsLeft = TIME_GREEN;
}

// Hàm phụ trợ để bật/tắt nhanh 3 đèn
void updateTrafficLights(int r, int y, int g) {
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}