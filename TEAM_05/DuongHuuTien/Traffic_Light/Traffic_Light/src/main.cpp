#include <Arduino.h>
#include <TM1637Display.h>

// --- 1. ĐỊNH NGHĨA CHÂN (Theo JSON) ---
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_PED     21  // Đèn xanh dương (người đi bộ)
#define BTN_PIN     23
#define LDR_PIN     13  // Cảm biến ánh sáng
#define CLK_PIN     18
#define DIO_PIN     19

// --- 2. CẤU HÌNH THỜI GIAN (mili giây) ---
const unsigned long TIME_RED    = 5000;
const unsigned long TIME_YELLOW = 2000;
const unsigned long TIME_GREEN  = 5000;
const unsigned long BLINK_INTERVAL = 500; // Nhấp nháy vàng mỗi 0.5s

// Ngưỡng ánh sáng (Wokwi: kéo sang trái là sáng, phải là tối hoặc ngược lại tùy cấu hình)
// Thông thường LDR kéo lên 5V, khi tối điện trở tăng -> áp tại chân 13 giảm.
const int LDR_THRESHOLD = 1000; 

TM1637Display display(CLK_PIN, DIO_PIN);

// --- 3. QUẢN LÝ TRẠNG THÁI ---
enum TrafficState {
  STATE_GREEN,
  STATE_YELLOW,
  STATE_RED,
  STATE_NIGHT
};

TrafficState currentState = STATE_GREEN;
TrafficState preNightState = STATE_GREEN; // Lưu trạng thái trước khi vào chế độ đêm

// Biến quản lý thời gian (Timer)
unsigned long stateStartTime = 0;       // Thời điểm bắt đầu trạng thái hiện tại
unsigned long lastBlinkTime = 0;        // Timer cho nhấp nháy đèn vàng
unsigned long lastBtnCheck = 0;         // Timer cho chống dội nút nhấn
unsigned long currentDuration = TIME_GREEN; // Thời gian của trạng thái hiện tại

bool yellowState = false; // Trạng thái bật/tắt của đèn vàng khi nhấp nháy

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_PED, OUTPUT);
  
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(0x0f);
  
  // Khởi tạo trạng thái đầu
  stateStartTime = millis();
  currentState = STATE_GREEN;
  currentDuration = TIME_GREEN;
}

// Hàm tắt hết đèn để chuyển trạng thái sạch sẽ
void turnOffAll() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_PED, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- TÁC VỤ 1: ĐỌC CẢM BIẾN ÁNH SÁNG ---
  int ldrValue = analogRead(LDR_PIN);
  
  // Logic chuyển sang chế độ đêm
  if (ldrValue < LDR_THRESHOLD) {
    if (currentState != STATE_NIGHT) {
      preNightState = currentState; // Lưu trạng thái cũ nếu cần (tùy chọn)
      currentState = STATE_NIGHT;
      turnOffAll();
      display.clear(); // Tắt màn hình khi đêm
    }
  } else {
    // Nếu trời sáng lại
    if (currentState == STATE_NIGHT) {
      currentState = STATE_RED; // Reset về Đỏ cho an toàn
      stateStartTime = currentMillis;
      currentDuration = TIME_RED;
      turnOffAll();
    }
  }

  // --- TÁC VỤ 2: XỬ LÝ NÚT NHẤN (Không delay) ---
  // Kiểm tra mỗi 50ms để chống dội (debounce)
  if (currentMillis - lastBtnCheck > 50) {
    lastBtnCheck = currentMillis;
    if (digitalRead(BTN_PIN) == LOW) {
      // Nếu đang đèn Xanh và còn nhiều thời gian (>2s) -> Rút ngắn còn 2s
      if (currentState == STATE_GREEN) {
        unsigned long elapsedTime = currentMillis - stateStartTime;
        unsigned long remaining = currentDuration - elapsedTime;
        
        if (remaining > 2000) {
           // Dời điểm bắt đầu về quá khứ để giả lập là đã chạy gần xong
           stateStartTime = currentMillis - (currentDuration - 2000);
           Serial.println("Button Pressed: Rút ngắn đèn xanh!");
        }
      }
    }
  }

  // --- TÁC VỤ 3: MÁY TRẠNG THÁI ĐÈN ---
  if (currentState == STATE_NIGHT) {
    // Chế độ đêm: Nhấp nháy đèn vàng
    if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
      lastBlinkTime = currentMillis;
      yellowState = !yellowState;
      digitalWrite(LED_YELLOW, yellowState);
    }
  } 
  else {
    // Chế độ ngày: Tính toán thời gian trôi qua
    unsigned long elapsedTime = currentMillis - stateStartTime;
    
    // Xử lý chuyển trạng thái khi hết giờ
    if (elapsedTime >= currentDuration) {
      stateStartTime = currentMillis; // Reset timer
      turnOffAll();

      switch (currentState) {
        case STATE_GREEN:
          currentState = STATE_YELLOW;
          currentDuration = TIME_YELLOW;
          break;
          
        case STATE_YELLOW:
          currentState = STATE_RED;
          currentDuration = TIME_RED;
          break;
          
        case STATE_RED:
          currentState = STATE_GREEN;
          currentDuration = TIME_GREEN;
          break;
      }
    }

    // --- TÁC VỤ 4: ĐIỀU KHIỂN ĐÈN & MÀN HÌNH ---
    // Cập nhật đèn theo trạng thái hiện tại
    switch (currentState) {
      case STATE_GREEN:
        digitalWrite(LED_GREEN, HIGH);
        break;
      case STATE_YELLOW:
        digitalWrite(LED_YELLOW, HIGH);
        break;
      case STATE_RED:
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_PED, HIGH); // Đèn đi bộ sáng cùng đèn đỏ
        break;
    }

    // Hiển thị đếm ngược (Chuyển mili giây sang giây)
    int secondsRemaining = (currentDuration - elapsedTime) / 1000 + 1;
    // +1 để tránh hiển thị số 0 quá lâu trước khi chuyển
    display.showNumberDec(secondsRemaining);
  }
}