#include <Arduino.h>
#include <TM1637Display.h>

#define LED_RED    27
#define LED_YELLOW 26
#define LED_GREEN  25

#define BTN_PIN    23
#define LED_BLUE   21
#define LED_BLUE_ON   HIGH
#define LED_BLUE_OFF  LOW

#define CLK 18
#define DIO 19

#define LDR_PIN 13  // AO của LDR (ADC)

TM1637Display display(CLK, DIO);

enum TrafficState { RED, GREEN, YELLOW };
TrafficState currentState = RED;

int remainSeconds = 3;

// Nút bật/tắt hiển thị + led xanh dương
bool buttonMode = false;
bool lastBtnRaw = HIGH;
bool btnStable = HIGH;
unsigned long lastDebounceMs = 0;
const unsigned long DEBOUNCE_MS = 30;

// Timing
unsigned long lastSecondMillis = 0;

// LDR / Day-Night
bool isNight = false;
unsigned long lastLdrReadMs = 0;
const unsigned long LDR_READ_MS = 100;   // đọc LDR mỗi 100ms

// Ngưỡng (0..4095)
// Với Wokwi/module của bạn: GIÁ TRỊ THƯỜNG "CAO khi tối" => vào đêm khi >= NIGHT, ra ngày khi <= DAY
const int LDR_TH_NIGHT = 2500; // >= là tối
const int LDR_TH_DAY   = 2200; // <= là sáng (hysteresis)  (phải nhỏ hơn NIGHT)

// Nháy vàng ban đêm
unsigned long lastBlinkMs = 0;
const unsigned long BLINK_MS = 500;
bool yellowOn = false;

// Display tối ưu: chỉ vẽ khi số thay đổi
int lastShown = -9999;

void showNumberIfNeeded(int v) {
  if (!buttonMode) return;
  if (v != lastShown) {
    display.showNumberDec(v, true);
    lastShown = v;
  }
}

void applyState(TrafficState s) {
  currentState = s;

  digitalWrite(LED_RED,    s == RED    ? HIGH : LOW);
  digitalWrite(LED_GREEN,  s == GREEN  ? HIGH : LOW);
  digitalWrite(LED_YELLOW, s == YELLOW ? HIGH : LOW);

  if (s == RED)    remainSeconds = 3;
  if (s == GREEN)  remainSeconds = 5;
  if (s == YELLOW) remainSeconds = 2;

  showNumberIfNeeded(remainSeconds);
}

void nextState() {
  if (currentState == RED) applyState(GREEN);
  else if (currentState == GREEN) applyState(YELLOW);
  else applyState(RED);
}

void enterNightMode(unsigned long now) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);

  yellowOn = false;
  lastBlinkMs = now;

  lastShown = -9999;
  showNumberIfNeeded(0);   // nếu đang bật display thì hiện 0
}

void enterDayMode(unsigned long now) {
  lastShown = -9999;
  applyState(RED);
  lastSecondMillis = now;  // reset mốc giây để không bị nhảy số
}

void handleButton(unsigned long now) {
  bool raw = digitalRead(BTN_PIN);

  // debounce không blocking
  if (raw != lastBtnRaw) {
    lastDebounceMs = now;
    lastBtnRaw = raw;
  }

  if (now - lastDebounceMs >= DEBOUNCE_MS) {
    if (btnStable != raw) {
      btnStable = raw;

      // bắt cạnh xuống (nhấn)
      if (btnStable == LOW) {
        buttonMode = !buttonMode;

        if (buttonMode) {
          digitalWrite(LED_BLUE, LED_BLUE_ON);
          lastShown = -9999;
          if (isNight) showNumberIfNeeded(0);
          else showNumberIfNeeded(remainSeconds);
        } else {
          digitalWrite(LED_BLUE, LED_BLUE_OFF);
          display.clear();
          lastShown = -9999;
        }
      }
    }
  }
}

void handleLdr(unsigned long now) {
  if (now - lastLdrReadMs < LDR_READ_MS) return;
  lastLdrReadMs = now;

  int ldr = analogRead(LDR_PIN);

  // hysteresis chống nhấp nháy chế độ (LOGIC ĐÃ ĐẢO: tối -> giá trị cao)
  bool newNight = isNight;
  if (!isNight && ldr >= LDR_TH_NIGHT) newNight = true;      // vào đêm khi ldr cao
  else if (isNight && ldr <= LDR_TH_DAY) newNight = false;   // ra ngày khi ldr thấp

  if (newNight != isNight) {
    isNight = newNight;
    if (isNight) enterNightMode(now);
    else enterDayMode(now);
  }
}

void handleTrafficDay(unsigned long now) {
  if (now - lastSecondMillis >= 1000) {
    lastSecondMillis += 1000;

    remainSeconds--;
    if (remainSeconds <= 0) {
      nextState();
    } else {
      showNumberIfNeeded(remainSeconds);
    }
  }
}

void handleNightBlink(unsigned long now) {
  if (now - lastBlinkMs >= BLINK_MS) {
    lastBlinkMs += BLINK_MS;
    yellowOn = !yellowOn;
    digitalWrite(LED_YELLOW, yellowOn ? HIGH : LOW);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(BTN_PIN, INPUT_PULLUP);

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LED_BLUE_OFF);

  display.setBrightness(7);
  display.clear();

  applyState(RED);
  lastSecondMillis = millis();
}

void loop() {
  unsigned long now = millis();

  handleButton(now);
  handleLdr(now);

  if (isNight) {
    handleNightBlink(now);
  } else {
    handleTrafficDay(now);
  }
}
