#include <Arduino.h>
#include <TM1637Display.h>

// ====================== PIN CONFIG (ESP32) ======================
static const int LED_RED    = 25;
static const int LED_YELLOW = 26;
static const int LED_GREEN  = 27;

static const int BUTTON_PIN = 14;   // PushButton (nút xanh) - dùng INPUT_PULLUP
static const int INDICATOR_LED = 33; // LED xanh nước báo trạng thái chạy/pause

static const int TM_CLK = 18;       // TM1637 CLK
static const int TM_DIO = 19;       // TM1637 DIO

static const int LDR_PIN = 34;      // Photoresistor sensor (Analog input only)

// ====================== TM1637 ======================
TM1637Display display(TM_CLK, TM_DIO);

// ====================== TRAFFIC LIGHT TIMING (seconds) ======================
static const uint16_t T_RED_S    = 3;
static const uint16_t T_YELLOW_S = 2;
static const uint16_t T_GREEN_S  = 4;

// ====================== LDR THRESHOLD ======================
// Giá trị analog ESP32: 0..4095
// Bạn có thể chỉnh các ngưỡng này theo thực tế trong Wokwi.
static const int LDR_ON_THRESHOLD  = 2000; // <= ngưỡng này coi là TỐI -> bật hệ thống
static const int LDR_OFF_THRESHOLD = 2300; // >= ngưỡng này coi là SÁNG -> tắt hệ thống (hysteresis)

// ====================== STATE MACHINE ======================
enum Phase : uint8_t { PHASE_RED, PHASE_YELLOW, PHASE_GREEN };

static Phase currentPhase = PHASE_RED;

// running = true: hệ thống đang chạy (không pause)
static bool running = true;

// enabledByLdr = true: LDR cho phép chạy (trời tối), false: trời sáng -> tắt hệ thống
static bool enabledByLdr = true;

// Thời điểm bắt đầu phase (millis)
static uint32_t phaseStartMs = 0;

// ====================== BUTTON DEBOUNCE ======================
static bool lastButtonRead = HIGH;
static bool buttonStableState = HIGH;
static uint32_t lastDebounceMs = 0;
static const uint32_t DEBOUNCE_MS = 40;

// ====================== DISPLAY UPDATE ======================
static uint32_t lastDisplayUpdateMs = 0;
static const uint32_t DISPLAY_UPDATE_MS = 150;

// ====================== HELPERS ======================
static void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

static uint16_t phaseDurationSeconds(Phase p) {
  switch (p) {
    case PHASE_RED:    return T_RED_S;
    case PHASE_YELLOW: return T_YELLOW_S;
    case PHASE_GREEN:  return T_GREEN_S;
  }
  return 1;
}

static void setPhaseLights(Phase p) {
  allOff();
  switch (p) {
    case PHASE_RED:    digitalWrite(LED_RED, HIGH); break;
    case PHASE_YELLOW: digitalWrite(LED_YELLOW, HIGH); break;
    case PHASE_GREEN:  digitalWrite(LED_GREEN, HIGH); break;
  }
}

static void nextPhase() {
  if (currentPhase == PHASE_RED) currentPhase = PHASE_YELLOW;
  else if (currentPhase == PHASE_YELLOW) currentPhase = PHASE_GREEN;
  else currentPhase = PHASE_RED;

  phaseStartMs = millis();
  setPhaseLights(currentPhase);
}

static void showCountdownSeconds(uint16_t sec) {
  // TM1637 4 digits: hiển thị dạng 0003, 0012, 0123, 1234...
  if (sec > 9999) sec = 9999;
  display.showNumberDec(sec, true); // leading zeros = true
}

static void displayOff() {
  display.clear();
  display.setBrightness(0, false); // tắt display
}

static void displayOn() {
  display.setBrightness(7, true);  // bật display, độ sáng 0..7
}

static void applyRunningIndicator() {
  // LED xanh nước báo trạng thái: chạy -> sáng, pause -> tắt
  digitalWrite(INDICATOR_LED, running ? HIGH : LOW);
}

static void updateLdrEnable() {
  int ldr = analogRead(LDR_PIN);

  // Hysteresis: tránh nhấp nháy on/off khi gần ngưỡng
  if (enabledByLdr) {
    // đang bật, nếu quá sáng thì tắt
    if (ldr >= LDR_OFF_THRESHOLD) enabledByLdr = false;
  } else {
    // đang tắt, nếu đủ tối thì bật lại
    if (ldr <= LDR_ON_THRESHOLD) enabledByLdr = true;
  }
}

static void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonRead) {
    lastDebounceMs = millis();
    lastButtonRead = reading;
  }

  if ((millis() - lastDebounceMs) > DEBOUNCE_MS) {
    if (reading != buttonStableState) {
      buttonStableState = reading;

      // Nút nhấn kiểu INPUT_PULLUP: nhấn = LOW
      if (buttonStableState == LOW) {
        // Toggle pause/resume
        running = !running;

        // Khi resume: cần “bù” lại phaseStartMs để thời gian không bị nhảy.
        // Cách làm: nếu pause thì lưu lại remaining; nhưng đơn giản nhất:
        // ta dùng biến phaseStartMs và khi pause không cập nhật, khi resume ta reset start = now - elapsedPaused?
        // -> dễ hơn: khi pause, ta lưu remainingSec; khi resume, ta start lại phase với đúng remaining.
        // Ở đây dùng cách: giữ nguyên phaseStartMs và có biến frozenElapsedMs.

        // Vì ta không dùng frozenElapsedMs ở trên, nên ta xử lý đơn giản:
        // Khi pause: lưu lại phần đã chạy (elapsedMs)
        // Khi resume: đặt phaseStartMs = now - elapsedMs đã lưu
        static uint32_t savedElapsedMs = 0;

        if (!running) {
          // chuyển sang pause: lưu elapsed
          savedElapsedMs = millis() - phaseStartMs;
        } else {
          // resume: khôi phục start để elapsed tiếp tục đúng chỗ
          phaseStartMs = millis() - savedElapsedMs;
        }

        applyRunningIndicator();
      }
    }
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(INDICATOR_LED, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // ESP32 ADC input: không cần pinMode cho analogRead, nhưng vẫn ok
  // pinMode(LDR_PIN, INPUT);

  allOff();

  displayOn();
  display.clear();

  currentPhase = PHASE_RED;
  phaseStartMs = millis();
  setPhaseLights(currentPhase);

  running = true;
  enabledByLdr = true;
  applyRunningIndicator();
}

void loop() {
  handleButton();
  updateLdrEnable();

  // Nếu LDR đang báo "trời sáng" -> tắt toàn bộ
  if (!enabledByLdr) {
    allOff();
    displayOff();
    // LED báo nút vẫn phản ánh running/pause, bạn có thể đổi nếu muốn:
    applyRunningIndicator();
    delay(30); // nhẹ CPU
    return;
  } else {
    displayOn();
  }

  // Nếu đang chạy thì cập nhật phase theo thời gian
  if (running) {
    uint32_t now = millis();
    uint32_t elapsedMs = now - phaseStartMs;
    uint32_t durationMs = (uint32_t)phaseDurationSeconds(currentPhase) * 1000UL;

    if (elapsedMs >= durationMs) {
      nextPhase();
    }
  }

  // Cập nhật hiển thị countdown theo phase hiện tại
  if (millis() - lastDisplayUpdateMs >= DISPLAY_UPDATE_MS) {
    lastDisplayUpdateMs = millis();

    uint32_t now = millis();
    uint32_t elapsedMs = now - phaseStartMs;
    uint32_t durationMs = (uint32_t)phaseDurationSeconds(currentPhase) * 1000UL;

    uint32_t remainingMs = 0;
    if (elapsedMs >= durationMs) remainingMs = 0;
    else remainingMs = durationMs - elapsedMs;

    uint16_t remainingSec = (uint16_t)((remainingMs + 999) / 1000); // làm tròn lên để thấy 3,2,1
    showCountdownSeconds(remainingSec);
  }

  delay(5);
}
