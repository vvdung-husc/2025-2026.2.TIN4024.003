#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa các chân cắm
#define RED_LED 25
#define YELLOW_LED 33
#define GREEN_LED 32
#define BLUE_LED 21      
#define BUTTON_PIN 23    
#define CLK 15           
#define DIO 2            

TM1637Display display(CLK, DIO);

// --- CÁC BIẾN QUẢN LÝ ---
bool displayEnabled = true;
unsigned long previousMillis = 0;
int countdown = 10;
int currentStage = 0; 
const long interval = 1000; 

// --- KHAI BÁO NGUYÊN MẪU HÀM (Để sửa lỗi biên dịch) ---
void updateLEDs();

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(0x0f); 
  updateLEDs(); // Bây giờ trình biên dịch đã biết hàm này là gì
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Xử lý nút bấm (Bật/Tắt màn hình)
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    displayEnabled = !displayEnabled; 
    delay(50); // Debounce
  }
  lastButtonState = currentButtonState;

  // 2. Đèn xanh đơn báo trạng thái màn hình
  digitalWrite(BLUE_LED, displayEnabled ? HIGH : LOW);

  // 3. Logic đếm ngược
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    countdown--;

    if (countdown < 0) {
      countdown = 10;
      currentStage = (currentStage + 1) % 3;
      updateLEDs();
    }

    if (displayEnabled) {
      display.showNumberDec(countdown);
    } else {
      display.clear(); 
    }
  }
}

// --- ĐỊNH NGHĨA HÀM ---
void updateLEDs() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  if (currentStage == 0) digitalWrite(RED_LED, HIGH);      // Đỏ
  else if (currentStage == 1) digitalWrite(YELLOW_LED, HIGH); // Vàng
  else if (currentStage == 2) digitalWrite(GREEN_LED, HIGH); // Xanh
}