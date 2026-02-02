#include <Arduino.h>
#include <TM1637Display.h>

// ===== CẤU HÌNH CHÂN CẮM =====
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_BLUE 21
#define BUTTON_PIN 23
#define LDR_PIN 13 
#define CLK 18
#define DIO 19

// ===== CẤU HÌNH THÔNG SỐ =====
#define DARK_THRESHOLD 2000 
TM1637Display display(CLK, DIO);

const int TIMES[] = {5, 3, 7}; // R, Y, G
unsigned long blinkInterval = 300;

// ===== BIẾN TRẠNG THÁI =====
enum LightState { RED, YELLOW, GREEN, NIGHT_MODE };
LightState currentState = RED;
LightState previousState = RED; 

unsigned long stateStartTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastButtonTime = 0;
unsigned long lastTickTime = 0; 
bool ledStatus = false;
int remainingSeconds = 0;
bool displayEnabled = true;

void turnOffTrafficLED() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void updateTerminal(int ldr, bool isDark) {
  Serial.print("\033[H"); // Đưa con trỏ về đầu dòng
  Serial.println("======= TRAFFIC CONTROL SYSTEM =======");
  Serial.print("Anh sang: "); Serial.print(ldr);
  Serial.print(" | Che do: "); 
  if (isDark) Serial.println("\033[1;33mBAN DEM (NHAY VANG)\033[0m");
  else {
    Serial.print("BAN NGAY | Den: ");
    if (currentState == RED) Serial.println("\033[1;31m[ DO ]\033[0m");
    else if (currentState == YELLOW) Serial.println("\033[1;33m[ VANG ]\033[0m");
    else Serial.println("\033[1;32m[ XANH ]\033[0m");
  }
  Serial.print("Dem nguoc: "); Serial.print(remainingSeconds); Serial.println(" s   ");
  Serial.println("======================================");
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  display.setBrightness(7);
  
  remainingSeconds = TIMES[currentState];
  stateStartTime = millis();
}

void loop() {
  unsigned long now = millis();
  int ldrValue = analogRead(LDR_PIN);
  bool isDark = (ldrValue > DARK_THRESHOLD);

  if (isDark) {
    if (currentState != NIGHT_MODE) {
      previousState = (currentState == NIGHT_MODE) ? RED : currentState;
      currentState = NIGHT_MODE;
      turnOffTrafficLED();
    }
    if (now - lastBlinkTime >= 500) {
      lastBlinkTime = now;
      ledStatus = !ledStatus;
      digitalWrite(LED_YELLOW, ledStatus ? HIGH : LOW);
    }
    display.clear();
    digitalWrite(LED_BLUE, LOW);
    if (now - lastTickTime > 1000) { updateTerminal(ldrValue, true); lastTickTime = now; }
    return; 
  } 
  else if (currentState == NIGHT_MODE) {
    currentState = previousState;
    remainingSeconds = TIMES[currentState];
    stateStartTime = now;
  }

  // 2. LOGIC ĐẾM NGƯỢC CHUẨN (Mỗi 1000ms thực tế)
  if (now - stateStartTime >= 1000) {
    stateStartTime = now;
    remainingSeconds--;
    
    // Cập nhật Terminal mỗi giây
    updateTerminal(ldrValue, false);

    if (remainingSeconds < 0) {
      if (currentState == RED) currentState = YELLOW;
      else if (currentState == YELLOW) currentState = GREEN;
      else currentState = RED;
      remainingSeconds = TIMES[currentState];
    }
    
    if (displayEnabled) display.showNumberDec(remainingSeconds);
  }

  // 3. NHẤP NHÁY LED THEO TRẠNG THÁI
  if (now - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = now;
    ledStatus = !ledStatus;
    int currentPin = (currentState == RED) ? LED_RED : (currentState == YELLOW ? LED_YELLOW : LED_GREEN);
    turnOffTrafficLED();
    digitalWrite(currentPin, ledStatus ? HIGH : LOW);
  }

  // 4. NÚT BẤM
  if (digitalRead(BUTTON_PIN) == LOW && now - lastButtonTime > 250) {
    displayEnabled = !displayEnabled;
    if (!displayEnabled) display.clear();
    else display.showNumberDec(remainingSeconds);
    lastButtonTime = now;
  }
  digitalWrite(LED_BLUE, displayEnabled ? HIGH : LOW);
}