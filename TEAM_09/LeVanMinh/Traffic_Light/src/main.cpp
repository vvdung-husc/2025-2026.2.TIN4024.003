#include <Arduino.h>
#include <TM1637Display.h>

// --- ĐỊNH NGHĨA CHÂN CẮM (Khớp 100% sơ đồ JSON của bạn) ---
#define PIN_LED_RED    27  // Sửa từ 25 thành 27
#define PIN_LED_YELLOW 26  // Sửa từ 33 thành 26
#define PIN_LED_GREEN  25  // Sửa từ 32 thành 25

#define CLK 18             // Sửa từ 15 thành 18
#define DIO 19             // Sửa từ 2 thành 19

#define PIN_BUTTON_DISPLAY 23
#define PIN_LED_BLUE       22  // Sửa từ 21 thành 22

TM1637Display display(CLK, DIO);
int valueButtonDisplay = LOW;

// --- HÀM HỖ TRỢ ---
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

const char* LEDString(uint8_t pin) {
  switch (pin) {
    case PIN_LED_RED:    return "RED";
    case PIN_LED_YELLOW: return "YELLOW";
    case PIN_LED_GREEN:  return "GREEN";
    default:             return "UNKNOWN";
  }
}

// --- KHỞI TẠO ---
void setup() {
  // ESP32 dùng Serial.begin để terminal có thể nhận lệnh
  Serial.begin(115200);
  delay(500);
  Serial.println("*** PROJECT LED TRAFFIC STARTING ***");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  
  // Nút nhấn nên dùng INPUT_PULLUP nếu sơ đồ nối vào GND
  pinMode(PIN_BUTTON_DISPLAY, INPUT_PULLUP); 

  display.setBrightness(0x0a);
  display.clear();
}

// --- LOGIC ĐIỀU KHIỂN ---

// Xử lý nút bấm màu xanh để bật/tắt màn hình
void ProcessButtonPressed() {
  static unsigned long ulTimer = 0;
  if (!IsReady(ulTimer, 50)) return; // Debounce 50ms

  // Đọc giá trị và đảo ngược vì dùng INPUT_PULLUP (Nhấn là LOW)
  int reading = digitalRead(PIN_BUTTON_DISPLAY);
  int newState = (reading == LOW) ? HIGH : LOW; 

  if (newState != valueButtonDisplay) {
    valueButtonDisplay = newState;
    if (valueButtonDisplay == HIGH) {
      digitalWrite(PIN_LED_BLUE, HIGH);
      Serial.println(">>> DISPLAY ON");
    } else {
      digitalWrite(PIN_LED_BLUE, LOW);
      display.clear();
      Serial.println(">>> DISPLAY OFF");
    }
  }
}

// Xử lý đèn giao thông và đếm ngược
void ProcessLEDTrafficWaitTime() {
  static unsigned long ulTimer = 0;
  static uint8_t idxLED = 0; 
  static uint8_t LEDs[3] = {PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED};
  static uint32_t waitTime[3] = {7000, 3000, 5000}; // Green, Yellow, Red
  static int32_t count = waitTime[idxLED];
  static int secondCount = 0;

  // Cập nhật mỗi 1000ms (1 giây) để Terminal và Display nhảy đều
  if (!IsReady(ulTimer, 1000)) return;

  if (count <= 0) {
    idxLED = (idxLED + 1) % 3;
    count = waitTime[idxLED];
  }

  secondCount = (count / 1000);

  // Điều khiển đèn LED
  for (size_t i = 0; i < 3; i++) {
    digitalWrite(LEDs[i], (i == idxLED) ? HIGH : LOW);
  }

  // Xuất thông tin ra Terminal
  Serial.print("[" + String(LEDString(LEDs[idxLED])) + "] Giay: ");
  Serial.println(secondCount);

  // Điều khiển màn hình TM1637
  if (valueButtonDisplay == HIGH) {
    display.showNumberDec(secondCount);
  } else {
    display.clear();
  }

  count -= 1000;
}

void loop() {
  ProcessButtonPressed();
  ProcessLEDTrafficWaitTime();
}