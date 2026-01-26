#include <Arduino.h>
#include <TM1637Display.h>

// ========== ĐỊNH NGHĨA CHÂN ==========
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21
#define LDR_PIN     13
#define BUTTON_PIN  23
#define CLK_PIN     18
#define DIO_PIN     19

// ========== THỜI GIAN ĐÈN GIAO THÔNG (ms) ==========
#define TIME_RED      5000  // 10 giây
#define TIME_YELLOW   2000   // 3 giây
#define TIME_GREEN    4000   // 7 giây

// ========== NGƯỠNG LDR ==========
#define LDR_NIGHT_THRESHOLD 2000  // Giá trị dưới ngưỡng = ban đêm

// ========== CẤU TRÚC ==========
enum TrafficState {
  STATE_RED,
  STATE_YELLOW,
  STATE_GREEN
};

enum SystemMode {
  MODE_NORMAL,      // Hoạt động bình thường
  MODE_EMERGENCY,   // Chế độ khẩn cấp (nhấp nháy vàng)
  MODE_NIGHT        // Chế độ ban đêm (chu kỳ nhanh hơn)
};

// ========== BIẾN TOÀN CỤC ==========
TM1637Display display(CLK_PIN, DIO_PIN);

TrafficState currentState = STATE_RED;
SystemMode systemMode = MODE_NORMAL;

unsigned long ulTimerState = 0;      // Timer cho đổi trạng thái đèn
unsigned long ulTimerDisplay = 0;    // Timer cập nhật màn hình
unsigned long ulTimerBlink = 0;      // Timer nhấp nháy đèn vàng (chế độ ban đêm)
unsigned long ulTimerBlueBlink = 0;  // Timer nhấp nháy đèn xanh dương
unsigned long ulTimerLDR = 0;        // Timer đọc LDR
unsigned long ulTimerButton = 0;     // Timer debounce button

int timeRemaining = TIME_RED / 1000;
bool blinkState = false;
bool blueBlinkState = false;  // Trạng thái nhấp nháy đèn xanh dương
bool buttonPressed = false;
bool lastButtonState = HIGH;
int cycleCount = 0;  // Đếm số chu kỳ đã hoàn thành
unsigned long nightModeStartTime = 0;  // Thời điểm bắt đầu chế độ ban đêm
const uint32_t NIGHT_MODE_DURATION = 10000;  // Chế độ ban đêm kéo dài 10 giây
bool displayEnabled = true;  // Trạng thái bật/tắt màn hình và đèn xanh dương

// ========== HÀM TIMER KHÔNG CHẶN ==========
bool isReady(unsigned long &ulTimer, uint32_t milliseconds) {
  if (millis() - ulTimer >= milliseconds) {
    ulTimer = millis();
    return true;
  }
  return false;
}

// ========== HÀM ĐIỀU KHIỂN ĐÈN ==========
void setTrafficLight(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_YELLOW, yellow ? HIGH : LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
}

void turnOffAllLights() {
  setTrafficLight(false, false, false);
}

// ========== HÀM LẤY THỜI GIAN THEO TRẠNG THÁI ==========
uint32_t getStateTime(TrafficState state) {
  // Nếu chế độ ban đêm, giảm thời gian xuống 50%
  float multiplier = (systemMode == MODE_NIGHT) ? 0.5 : 1.0;
  
  switch (state) {
    case STATE_RED:    return TIME_RED * multiplier;
    case STATE_YELLOW: return TIME_YELLOW * multiplier;
    case STATE_GREEN:  return TIME_GREEN * multiplier;
    default:           return TIME_RED;
  }
}

// ========== HÀM XỬ LÝ CHUYỂN TRẠNG THÁI ==========
void handleTrafficLightNormal() {
  // Cập nhật đèn theo trạng thái hiện tại
  switch (currentState) {
    case STATE_RED:
      setTrafficLight(true, false, false);
      break;
    case STATE_YELLOW:
      setTrafficLight(false, true, false);
      break;
    case STATE_GREEN:
      setTrafficLight(false, false, true);
      break;
  }
  
  // Kiểm tra chuyển trạng thái
  if (isReady(ulTimerState, getStateTime(currentState))) {
    switch (currentState) {
      case STATE_RED:
        currentState = STATE_GREEN;
        timeRemaining = getStateTime(STATE_GREEN) / 1000;
        break;
      case STATE_GREEN:
        currentState = STATE_YELLOW;
        timeRemaining = getStateTime(STATE_YELLOW) / 1000;
        break;
      case STATE_YELLOW:
        currentState = STATE_RED;
        timeRemaining = getStateTime(STATE_RED) / 1000;
        cycleCount++;
        
        // Sau khi hoàn thành 1 chu kỳ, chuyển sang chế độ ban đêm
        if (cycleCount >= 1) {
          systemMode = MODE_NIGHT;
          nightModeStartTime = millis();
          cycleCount = 0;
          Serial.println(">>> Chuyển sang chế độ BAN ĐÊM");
        }
        break;
    }
  }
}

// ========== HÀM XỬ LÝ ĐÈN XANH DƯƠNG NHẤP NHÁY ==========
void handleBlueLEDBlink() {
  if (!displayEnabled) {
    // Nhấp nháy đèn xanh dương khi tắt màn hình
    if (isReady(ulTimerBlueBlink, 250)) {  // Nhấp nháy mỗi 250ms
      blueBlinkState = !blueBlinkState;
      digitalWrite(LED_BLUE, blueBlinkState ? HIGH : LOW);
    }
  } else {
    // Tắt đèn xanh dương khi màn hình bật
    digitalWrite(LED_BLUE, LOW);
  }
}

// ========== HÀM XỬ LÝ CHẾ ĐỘ BAN ĐÊM ==========
void handleNightMode() {
  // Nhấp nháy đèn vàng nhanh
  if (isReady(ulTimerBlink, 250)) {  // Nhấp nháy mỗi 250ms
    blinkState = !blinkState;
    setTrafficLight(false, blinkState, false);  // Chỉ nhấp nháy đèn VÀNG
  }
  
  // Kiểm tra thời gian để chuyển về chế độ bình thường
  if (millis() - nightModeStartTime >= NIGHT_MODE_DURATION) {
    systemMode = MODE_NORMAL;
    ulTimerState = millis();
    currentState = STATE_RED;
    timeRemaining = getStateTime(STATE_RED) / 1000;
    Serial.println(">>> Chuyển sang chế độ BÌNH THƯỜNG");
  }
}

// ========== HÀM CẬP NHẬT HIỂN THỊ ==========
void updateDisplay() {
  if (!displayEnabled) {
    // Tắt màn hình khi displayEnabled = false
    display.clear();
  } else if (systemMode == MODE_NIGHT) {
    // Tắt hiển thị khi ban đêm
    display.clear();
  } else {
    // Hiển thị thời gian đếm ngược (chế độ bình thường)
    if (isReady(ulTimerDisplay, 1000)) {
      timeRemaining--;
      if (timeRemaining < 0) {
        timeRemaining = 0;
      }
      display.showNumberDecEx(timeRemaining, 0b01000000, true);  // Hiển thị với dấu :
    }
  }
}

// ========== HÀM ĐỌC LDR ==========
void checkLightSensor() {
  if (isReady(ulTimerLDR, 2000)) {  // Đọc LDR mỗi 2 giây
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);
  }
}

// ========== HÀM XỬ LÝ NÚT NHẤN ==========
void checkButton() {
  if (isReady(ulTimerButton, 50)) {  // Debounce 50ms
    bool buttonState = digitalRead(BUTTON_PIN);
    
    if (buttonState == LOW && lastButtonState == HIGH) {
      // Nhấn nút - chuyển đổi trạng thái màn hình và đèn xanh dương
      displayEnabled = !displayEnabled;
      
      if (displayEnabled) {
        Serial.println("✅ BẬT màn hình - Tắt đèn xanh dương");
        digitalWrite(LED_BLUE, LOW);
        display.setBrightness(7);
      } else {
        Serial.println("❌ TẮT màn hình - Bật đèn xanh dương nhấp nháy");
        display.clear();
      }
      
      delay(200);  // Chống nhiễu
    }
    
    lastButtonState = buttonState;
  }
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Traffic Light System ===");
  
  // Cấu hình chân
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  
  // Tắt tất cả đèn
  turnOffAllLights();
  digitalWrite(LED_BLUE, LOW);
  
  // Khởi tạo display
  display.setBrightness(7);
  display.showNumberDec(0, false);
  
  // Khởi tạo timer
  ulTimerState = millis();
  ulTimerDisplay = millis();
  ulTimerBlink = millis();
  ulTimerLDR = millis();
  ulTimerButton = millis();
  
  timeRemaining = TIME_RED / 1000;
  
  Serial.println("System Ready!");
}

// ========== LOOP ==========
void loop() {
  // Kiểm tra cảm biến ánh sáng
  checkLightSensor();
  
  // Kiểm tra nút nhấn
  checkButton();
  
  // Xử lý đèn giao thông theo chế độ
  if (systemMode == MODE_NIGHT) {
    handleNightMode();
  } else {
    handleTrafficLightNormal();
  }
  
  // Xử lý đèn xanh dương nhấp nháy
  handleBlueLEDBlink();
  
  // Cập nhật hiển thị
  updateDisplay();
}