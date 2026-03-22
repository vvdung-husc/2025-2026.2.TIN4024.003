#include <Arduino.h>
#include <TM1637Display.h>

// ========== ĐỊNH NGHĨA CHÂN LED ==========
const uint8_t LED_RED = 27;
const uint8_t LED_YELLOW = 26;
const uint8_t LED_GREEN = 25;
const uint8_t LED_BLUE = 21;
const bool LED_BLUE_ACTIVE_LOW = false;

// ========== ĐỊNH NGHĨA CHÂN KHÁC ==========
const uint8_t CLK = 18;
const uint8_t DIO = 19;
const uint8_t BUTTON = 23;
const uint8_t LDR_SENSOR = 13;

// ========== THỜI GIAN ĐÈN GIAO THÔNG ==========
const uint32_t RED_DURATION = 10000;     // 10 giây
const uint32_t YELLOW_DURATION = 3000;   // 3 giây
const uint32_t GREEN_DURATION = 7000;    // 7 giây

// ========== TRẠNG THÁI ĐÈN GIAO THÔNG ==========
enum TrafficLightState {
  RED_LIGHT,
  YELLOW_LIGHT,
  GREEN_LIGHT
};

// ========== BIẾN TOÀN CỤC ==========
TrafficLightState currentState = RED_LIGHT;
unsigned long ulTimer = 0;
TM1637Display display(CLK, DIO);
int countdown = 0;
bool isBlinking = false;
unsigned long blinkTimer = 0;
bool blueLedState = false;

// ========== HÀM ĐIỀU KHIỂN ĐÈN XANH DƯƠNG ==========
void setBlueLed(bool on) {
  digitalWrite(LED_BLUE, on ? (LED_BLUE_ACTIVE_LOW ? LOW : HIGH)
                              : (LED_BLUE_ACTIVE_LOW ? HIGH : LOW));
}

// ========== HÀM TẮT TẤT CẢ ĐÈN ==========
void turnOffAllLeds() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// ========== HÀM XỬ LÝ ĐÈN GIAO THÔNG ==========
void updateTrafficLight() {
  unsigned long currentMillis = millis();
  
  switch(currentState) {
    case RED_LIGHT:
      if (currentMillis - ulTimer >= 1000) {
        ulTimer = currentMillis;
        countdown--;
        display.showNumberDec(countdown, false);
        
        if (countdown <= 0) {
          currentState = GREEN_LIGHT;
          countdown = GREEN_DURATION / 1000;
          turnOffAllLeds();
          digitalWrite(LED_GREEN, HIGH);
        }
      }
      break;
      
    case GREEN_LIGHT:
      if (currentMillis - ulTimer >= 1000) {
        ulTimer = currentMillis;
        countdown--;
        display.showNumberDec(countdown, false);
        
        if (countdown <= 0) {
          currentState = YELLOW_LIGHT;
          countdown = YELLOW_DURATION / 1000;
          turnOffAllLeds();
          digitalWrite(LED_YELLOW, HIGH);
        }
      }
      break;
      
    case YELLOW_LIGHT:
      if (currentMillis - ulTimer >= 1000) {
        ulTimer = currentMillis;
        countdown--;
        display.showNumberDec(countdown, false);
        
        if (countdown <= 0) {
          currentState = RED_LIGHT;
          countdown = RED_DURATION / 1000;
          turnOffAllLeds();
          digitalWrite(LED_RED, HIGH);
        }
      }
      break;
  }
}

void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);
  Serial.println("=== ĐÈN GIAO THÔNG ===");
  
  // Cấu hình chân LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  
  // Khởi tạo màn hình 7 đoạn
  display.setBrightness(0x0f);
  
  // Bắt đầu với đèn đỏ
  currentState = RED_LIGHT;
  countdown = RED_DURATION / 1000;
  digitalWrite(LED_RED, HIGH);
  display.showNumberDec(countdown, false);
  ulTimer = millis();
  
  // Tắt đèn xanh dương ban đầu
  isBlinking = false;
  blueLedState = false;
  setBlueLed(false);
}

void loop() {
  // Cập nhật đèn giao thông
  updateTrafficLight();
  
  // Kiểm tra nút nhấn để bật/tắt chế độ nháy
  if (digitalRead(BUTTON) == LOW) {
    isBlinking = !isBlinking;  // Đảo trạng thái
    if (isBlinking) {
      blueLedState = true;
      setBlueLed(true);
      blinkTimer = millis();
      Serial.println("Bật chế độ nháy đèn xanh dương");
    } else {
      setBlueLed(false);
      Serial.println("Tắt chế độ nháy đèn xanh dương");
    }
    delay(300);  // Chống dội
  }
  
  // Xử lý nháy đèn xanh dương
  if (isBlinking) {
    unsigned long currentMillis = millis();
    if (currentMillis - blinkTimer >= 500) {  // Nháy mỗi 0.5 giây
      blinkTimer = currentMillis;
      blueLedState = !blueLedState;
      setBlueLed(blueLedState);
      Serial.print("Nháy: ");
      Serial.println(blueLedState ? "Sáng" : "Tắt");
    }
  }
}