#include <Arduino.h>

const int LED_RED_PIN    = 15;
const int LED_GREEN_PIN  = 2;   
const int LED_YELLOW_PIN = 4;
const int BAO_HIEU_PIN   = 5;  
const int LDR_PIN        = 35;
const int NUT_BAM        = 21;  

const int SEG_PINS[7] = {13, 12, 14, 27, 26, 25, 33};
const int DIG_PINS[2] = {18, 19}; // DIG 1, DIG 2

const int TIME_RED    = 11;
const int TIME_GREEN  = 10;
const int TIME_YELLOW = 3; 
const int LIGHT_THRESHOLD = 2000; 

const byte digitPatterns[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1}, {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};

enum TrafficState {STATE_RED, STATE_GREEN, STATE_YELLOW};
TrafficState trafficState = STATE_RED;
int remainingTime = TIME_RED;

unsigned long tTraffic = 0;
unsigned long tDisplay = 0;
unsigned long tSensor = 0;
unsigned long tButton = 0;
unsigned long tBlink = 0; 

bool isNightMode = false;
bool isDisplayEnabled = true; 
bool blinkState = false; 

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(BAO_HIEU_PIN, OUTPUT);
  pinMode(NUT_BAM, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  for(int i=0; i<7; i++) pinMode(SEG_PINS[i], OUTPUT);
  for(int i=0; i<2; i++) pinMode(DIG_PINS[i], OUTPUT);

  digitalWrite(DIG_PINS[0], HIGH);
  digitalWrite(DIG_PINS[1], HIGH);
}

// Hàm hiển thị 1 số ra LED 7 đoạn
void setSegments(int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(SEG_PINS[i], digitPatterns[num][i]);
  }
}

void taskDisplay() {
 
  if (!isDisplayEnabled || isNightMode) {
    digitalWrite(DIG_PINS[0], HIGH);
    digitalWrite(DIG_PINS[1], HIGH);
    return;
  }

  if (millis() - tDisplay >= 5) { 
    tDisplay = millis();
    
    static uint8_t currentDigit = 0;
    
    digitalWrite(DIG_PINS[0], HIGH);
    digitalWrite(DIG_PINS[1], HIGH);

    int val = remainingTime;
    if (val < 0) val = 0;
    if (val > 99) val = 99;

    if (currentDigit == 0) {
      setSegments(val / 10); 
      digitalWrite(DIG_PINS[0], LOW);
    } else {
      setSegments(val % 10); 
      digitalWrite(DIG_PINS[1], LOW);
    }
    
    currentDigit = !currentDigit; 
  }
}

void taskTrafficLight() {

  if (isNightMode) {
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
    
    if (millis() - tBlink >= 500) {
      tBlink = millis();
      blinkState = !blinkState;
      digitalWrite(LED_YELLOW_PIN, blinkState ? HIGH : LOW);
    }
    return;
  }

  if (millis() - tTraffic >= 1000) {
    tTraffic = millis();
    remainingTime--;

    if (remainingTime < 0) {
      switch (trafficState) {
        case STATE_RED:
          trafficState = STATE_GREEN;
          remainingTime = TIME_GREEN;
          break;
        case STATE_GREEN:
          trafficState = STATE_YELLOW;
          remainingTime = TIME_YELLOW;
          break;
        case STATE_YELLOW:
          trafficState = STATE_RED;
          remainingTime = TIME_RED;
          break;
      }
    }
  }

  digitalWrite(LED_RED_PIN,    trafficState == STATE_RED);
  digitalWrite(LED_GREEN_PIN,  trafficState == STATE_GREEN);
  digitalWrite(LED_YELLOW_PIN, trafficState == STATE_YELLOW);
}

void taskSensorsAndInput() {
  if (millis() - tSensor >= 200) {
    tSensor = millis();
    int lightLevel = analogRead(LDR_PIN);
    
    bool newMode = (lightLevel < LIGHT_THRESHOLD);
    
    if (newMode != isNightMode) {
      isNightMode = newMode;
      if (isNightMode) {
         digitalWrite(BAO_HIEU_PIN, LOW); 
      } else {
         digitalWrite(BAO_HIEU_PIN, isDisplayEnabled ? LOW : HIGH); 
      }
    }
  }

  if (millis() - tButton >= 100) {
    tButton = millis();
    static int lastButtonState = HIGH;
    int currentButtonState = digitalRead(NUT_BAM);

    if (lastButtonState == HIGH && currentButtonState == LOW) {
      isDisplayEnabled = !isDisplayEnabled;
      
      if (!isNightMode) {
        digitalWrite(BAO_HIEU_PIN, isDisplayEnabled ? LOW : HIGH);
      }
    }
    lastButtonState = currentButtonState;
  }
}

void loop() {
  taskDisplay();        
  taskTrafficLight();  
  taskSensorsAndInput(); 
}