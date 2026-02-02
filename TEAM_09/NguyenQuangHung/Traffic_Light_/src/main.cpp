#include <Arduino.h>
#include <TM1637Display.h>

// ====================== PIN MAP ======================
static const int LED_RED    = 25;
static const int LED_YELLOW = 33;
static const int LED_GREEN  = 32;

static const int BUTTON_PIN    = 23;
static const int INDICATOR_LED = 21;

static const int TM_CLK = 15;
static const int TM_DIO = 2;

static const int LDR_PIN = 13;

// ====================== TM1637 ======================
TM1637Display display(TM_CLK, TM_DIO);

// ====================== TIMING (seconds) ======================
static const uint16_t T_GREEN_S  = 7;
static const uint16_t T_YELLOW_S = 3;
static const uint16_t T_RED_S    = 5;

// ====================== LDR THRESHOLD ======================
static const int LDR_ON_THRESHOLD  = 2000;
static const int LDR_OFF_THRESHOLD = 2300;

// ====================== STATE ======================
enum Phase : uint8_t {
  PHASE_GREEN,
  PHASE_YELLOW,
  PHASE_RED
};

static Phase currentPhase = PHASE_GREEN;
static bool running = true;
static bool enabledByLdr = true;
static uint32_t phaseStartMs = 0;
static uint32_t savedElapsedMs = 0;

// ====================== BUTTON ======================
static bool lastButtonState = HIGH;
static uint32_t lastDebounceTime = 0;
static const uint32_t DEBOUNCE_DELAY = 50;
static bool buttonPressed = false;

// ====================== DISPLAY ======================
static int16_t lastDisplayedSecond = -1;

// ====================== HELPERS ======================
static const char* phaseName(Phase p) {
  switch (p) {
    case PHASE_GREEN:  return "GREEN ";
    case PHASE_YELLOW: return "YELLOW";
    case PHASE_RED:    return "RED   ";
  }
  return "???";
}

static uint16_t phaseDuration(Phase p) {
  switch (p) {
    case PHASE_GREEN:  return T_GREEN_S;
    case PHASE_YELLOW: return T_YELLOW_S;
    case PHASE_RED:    return T_RED_S;
  }
  return 1;
}

static void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

static void setPhase(Phase p) {
  allOff();
  switch (p) {
    case PHASE_GREEN:  digitalWrite(LED_GREEN, HIGH);  break;
    case PHASE_YELLOW: digitalWrite(LED_YELLOW, HIGH); break;
    case PHASE_RED:    digitalWrite(LED_RED, HIGH);    break;
  }
  
  phaseStartMs = millis();
  savedElapsedMs = 0;
  lastDisplayedSecond = -1;  // Force display update
  
  Serial.printf("LED [%s] ON => %d Seconds\n", phaseName(p), phaseDuration(p));
}

static void nextPhase() {
  switch (currentPhase) {
    case PHASE_GREEN:  currentPhase = PHASE_YELLOW; break;
    case PHASE_YELLOW: currentPhase = PHASE_RED;    break;
    case PHASE_RED:    currentPhase = PHASE_GREEN;  break;
  }
  setPhase(currentPhase);
}

// ====================== BUTTON HANDLER ======================
void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);
  
  // Debounce
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // Phát hiện cạnh xuống (nhấn nút)
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      
      // TOGGLE pause/resume
      running = !running;
      
      if (running) {
        // RESUME: Khôi phục thời gian
        phaseStartMs = millis() - savedElapsedMs;
        digitalWrite(INDICATOR_LED, HIGH);
        Serial.println(">>> RESUMED <<<");
      } else {
        // PAUSE: Lưu thời gian
        savedElapsedMs = millis() - phaseStartMs;
        digitalWrite(INDICATOR_LED, LOW);
        Serial.println(">>> PAUSED <<<");
      }
    }
    
    // Phát hiện thả nút
    if (reading == HIGH) {
      buttonPressed = false;
    }
  }
  
  lastButtonState = reading;
}

// ====================== LDR HANDLER ======================
void handleLdr() {
  int ldr = analogRead(LDR_PIN);
  bool prevEnabled = enabledByLdr;
  
  if (enabledByLdr && ldr >= LDR_OFF_THRESHOLD) {
    enabledByLdr = false;
    Serial.println(">>> LDR: System OFF (too bright) <<<");
  } else if (!enabledByLdr && ldr <= LDR_ON_THRESHOLD) {
    enabledByLdr = true;
    Serial.println(">>> LDR: System ON (dark enough) <<<");
    // Reset về trạng thái ban đầu
    currentPhase = PHASE_GREEN;
    running = true;
    setPhase(currentPhase);
  }
}

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n*** PROJECT LED TRAFFIC ***");
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(INDICATOR_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  display.setBrightness(7, true);
  display.clear();
  
  allOff();
  digitalWrite(INDICATOR_LED, HIGH);
  
  currentPhase = PHASE_GREEN;
  running = true;
  setPhase(currentPhase);
}

// ====================== LOOP ======================
void loop() {
  // ✅ QUAN TRỌNG: Check button MỖI VÒNG LẶP (không có delay blocking!)
  handleButton();
  handleLdr();
  
  // Nếu LDR disable
  if (!enabledByLdr) {
    allOff();
    display.clear();
    return;
  }
  
  // Tính thời gian còn lại
  uint32_t elapsedMs;
  if (running) {
    elapsedMs = millis() - phaseStartMs;
  } else {
    elapsedMs = savedElapsedMs;  // Đóng băng khi pause
  }
  
  uint32_t durationMs = (uint32_t)phaseDuration(currentPhase) * 1000UL;
  uint32_t remainingMs = (elapsedMs >= durationMs) ? 0 : (durationMs - elapsedMs);
  int16_t remainingSec = (remainingMs + 999) / 1000;  // Làm tròn lên
  
  // Hiển thị countdown (chỉ khi thay đổi)
  if (remainingSec != lastDisplayedSecond) {
    lastDisplayedSecond = remainingSec;
    display.showNumberDec(remainingSec, true);
    Serial.printf(" [%s] => seconds: %d \n", phaseName(currentPhase), remainingSec);
  }
  
  // Chuyển phase khi hết giờ
  if (running && elapsedMs >= durationMs) {
    nextPhase();
  }
  
  // ✅ KHÔNG DÙNG delay() dài! Chỉ delay rất nhỏ để ổn định
  delay(10);
}