#include <Arduino.h>
#include <TM1637Display.h>
// PIN MAP khớp diagram.json
#define LED_R   27
#define LED_Y   26
#define LED_G   25
#define LED_B   21

#define BTN     23
#define LDR_AO  13

#define TM_CLK  18
#define TM_DIO  19

TM1637Display display(TM_CLK, TM_DIO);

// thời gian đèn (giây)
const int T_RED = 10;
const int T_YEL = 3;
const int T_GRN = 10;

// ngưỡng tối
const int LDR_DARK_TH = 1200;

enum State { S_RED, S_GREEN, S_YELLOW };
State state = S_RED;

int remain = T_RED;
bool showCountdown = true;

unsigned long last1s = 0;
unsigned long lastLdr = 0;

bool readButtonPressed() {
  static int lastStable = HIGH;
  static int lastRead = HIGH;
  static unsigned long tChanged = 0;

  int cur = digitalRead(BTN);
  if (cur != lastRead) {
    lastRead = cur;
    tChanged = millis();
  }
  if (millis() - tChanged < 30) return false;

  bool pressed = (lastStable == HIGH && cur == LOW);
  lastStable = cur;
  return pressed;
}

void setTraffic(bool r, bool y, bool g) {
  digitalWrite(LED_R, r);
  digitalWrite(LED_Y, y);
  digitalWrite(LED_G, g);
}

void showRemain(int s) {
  if (!showCountdown) {
    display.clear();
    return;
  }
  if (s < 0) s = 0;
  display.showNumberDec(s, true);
}

void enterState(State s) {
  state = s;
  if (s == S_RED)      { setTraffic(true,  false, false); remain = T_RED; }
  else if (s == S_GREEN){ setTraffic(false, false, true ); remain = T_GRN; }
  else                  { setTraffic(false, true,  false); remain = T_YEL; }
  showRemain(remain);
}

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pinMode(BTN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  enterState(S_RED);
  last1s = millis();
  lastLdr = millis();
}

void loop() {
  // Nút: bật/tắt hiển thị TM1637
  if (readButtonPressed()) {
    showCountdown = !showCountdown;
    showRemain(remain);
  }

  // LDR: tối -> bật LED_B, sáng -> tắt LED_B
  if (millis() - lastLdr >= 200) {
    lastLdr += 200;
    int ldr = analogRead(LDR_AO);
    digitalWrite(LED_B, (ldr < LDR_DARK_TH) ? HIGH : LOW);
  }

  // Đếm ngược + chuyển pha mỗi 1 giây
  if (millis() - last1s >= 1000) {
    last1s += 1000;
    remain--;
    showRemain(remain);

    if (remain <= 0) {
      if (state == S_RED) enterState(S_GREEN);
      else if (state == S_GREEN) enterState(S_YELLOW);
      else enterState(S_RED);
    }
  }
}
