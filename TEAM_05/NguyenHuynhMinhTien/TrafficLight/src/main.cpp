#include <Arduino.h>
#include <TM1637Display.h>

#define PIN_LED_RED 25
#define PIN_LED_YELLOW 33
#define PIN_LED_GREEN 32
#define CLK 15
#define DIO 2
#define PIN_BUTTON_DISPLAY 23
#define PIN_LED_BLUE 21

TM1637Display display(CLK, DIO);
bool displayOn = false;

// FSM cho Traffic Light
enum State { GREEN, YELLOW, RED };
State currentState = GREEN;
const uint32_t times[] = {7000, 3000, 5000}; // ms
unsigned long stateStart = 0;
int remainingSec = 0;

bool IsReady(unsigned long &timer, uint32_t ms) {
  if (millis() - timer < ms) return false;
  timer = millis();
  return true;
}

const char* StateString() {
  switch (currentState) {
    case GREEN: return "GREEN";
    case YELLOW: return "YELLOW";
    case RED: return "RED";
    default: return "UNKNOWN";
  }
}

void UpdateLEDs() {
  digitalWrite(PIN_LED_GREEN, currentState == GREEN ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, currentState == YELLOW ? HIGH : LOW);
  digitalWrite(PIN_LED_RED, currentState == RED ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  Serial.println("*** OPTIMIZED LED TRAFFIC FSM ***");
  
  pinMode(PIN_LED_RED, OUTPUT); pinMode(PIN_LED_YELLOW, OUTPUT); pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUTTON_DISPLAY, INPUT_PULLUP);  // Thêm PULLUP
  pinMode(PIN_LED_BLUE, OUTPUT);
  display.setBrightness(0x0a);
  display.clear();
  stateStart = millis();
}

void loop() {
  static unsigned long timer500 = 0, timer10 = 0;
  
  // FSM Traffic
  if (IsReady(timer500, 500)) {
    unsigned long elapsed = millis() - stateStart;
    remainingSec = (times[currentState] - elapsed) / 1000;
    
    if (elapsed >= times[currentState]) {
      currentState = static_cast<State>((currentState + 1) % 3);
      stateStart = millis();
      Serial.printf("Switch to %s (%d giây)\n", StateString(), times[currentState]/1000);
    } else if (displayOn) {
      display.showNumberDec(remainingSec > 0 ? remainingSec : 0);
      Serial.printf("[%s] %d giây\n", StateString(), remainingSec);
    }
    
    // Blink LED cuối 2s (optional)
    bool blink = (elapsed > times[currentState] - 2000);
    digitalWrite( (currentState == GREEN ? PIN_LED_GREEN : 
                   currentState == YELLOW ? PIN_LED_YELLOW : PIN_LED_RED), blink ? HIGH : LOW);
  }
  
  // Button (đảo logic vì PULLUP)
  if (IsReady(timer10, 10)) {
    bool pressed = !digitalRead(PIN_BUTTON_DISPLAY);  // LOW khi nhấn
    if (pressed != displayOn) {
      displayOn = pressed;
      digitalWrite(PIN_LED_BLUE, displayOn ? HIGH : LOW);
      Serial.printf("*** DISPLAY %s ***\n", displayOn ? "ON" : "OFF");
      if (!displayOn) display.clear();
    }
  }
}
