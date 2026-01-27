#include <Arduino.h>
// --- KHAI BÁO CHÂN (Pin Definitions) ---
const int PIN_RED    = 21;
const int PIN_YELLOW = 19;
const int PIN_GREEN  = 18;

// --- CẤU HÌNH THỜI GIAN ---
const long TIME_PHASE_LONG = 3000;  // Thời gian giữ 1 pha đèn (3 giây)
const long TIME_PHASE_SHORT = 2000; // Thời gian pha đèn vàng (2 giây)
const long TIME_BLINK = 200;        // Tốc độ nhấp nháy (200ms: chớp nhanh)

// --- CÁC BIẾN QUẢN LÝ THỜI GIAN ---
unsigned long timePhaseStart = 0;   // Mốc thời gian bắt đầu của pha hiện tại
unsigned long timeBlinkStart = 0;   // Mốc thời gian bắt đầu của lần chớp tắt

// --- TRẠNG THÁI ---
int currentPhase = 0; // 0: Đỏ, 1: Xanh, 2: Vàng
bool ledState = LOW;  // Trạng thái bật/tắt của đèn để nhấp nháy

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // Tắt hết ban đầu
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_YELLOW, LOW);
  digitalWrite(PIN_GREEN, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. XÁC ĐỊNH ĐÈN NÀO ĐANG HOẠT ĐỘNG VÀ THỜI GIAN PHA LÀ BAO LÂU
  int activePin;
  long phaseDuration;

  if (currentPhase == 0) {
    activePin = PIN_RED;
    phaseDuration = TIME_PHASE_LONG; // Đỏ nháy 3s
  } else if (currentPhase == 1) {
    activePin = PIN_GREEN;
    phaseDuration = TIME_PHASE_LONG; // Xanh nháy 3s
  } else {
    activePin = PIN_YELLOW;
    phaseDuration = TIME_PHASE_SHORT; // Vàng nháy 2s
  }

  // 2. XỬ LÝ NHẤP NHÁY (BLINK)
  // Cứ mỗi 200ms thì đảo trạng thái đèn 1 lần
  if (currentMillis - timeBlinkStart >= TIME_BLINK) {
    timeBlinkStart = currentMillis; // Reset đồng hồ blink
    ledState = !ledState;           // Đảo ngược trạng thái (Đang tắt -> Bật, Đang bật -> Tắt)
    
    // Chỉ bật/tắt đèn đang active, các đèn khác phải tắt
    digitalWrite(activePin, ledState ? HIGH : LOW);
  }

  // 3. XỬ LÝ CHUYỂN PHA (CHANGE PHASE)
  // Nếu đã nhấp nháy đủ thời gian của pha này -> Chuyển sang đèn khác
  if (currentMillis - timePhaseStart >= phaseDuration) {
    timePhaseStart = currentMillis; // Reset đồng hồ pha
    
    // Quan trọng: Tắt đèn hiện tại trước khi chuyển sang đèn mới
    digitalWrite(activePin, LOW);
    ledState = LOW; // Reset trạng thái chớp

    // Tăng pha
    currentPhase++;
    if (currentPhase > 2) {
      currentPhase = 0; // Quay về Đỏ
    }
    
    // In ra Serial để kiểm tra
    if(currentPhase == 0) Serial.println("Chuyển sang: ĐỎ nhấp nháy");
    else if(currentPhase == 1) Serial.println("Chuyển sang: XANH nhấp nháy");
    else Serial.println("Chuyển sang: VÀNG nhấp nháy");
  }
}