#include <Arduino.h>

const int LED_RED_PIN    = 15;
const int LED_GREEN_PIN  = 2;   
const int LED_YELLOW_PIN = 4;
const int BAO_HIEU_PIN   = 5;
const int LDR_PIN       = 35;
const int NUT_BAM   = 21;  

const int SEG_PINS[7] = {13,12,14,27,26,25,33};
const int DIG_PINS[2] = {18,19};

const int TIME_RED    = 11;
const int TIME_GREEN  = 10;
const int TIME_YELLOW = 5;
const int LIGHT_THRESHOLD = 2000; // Ngưỡng ánh sáng để bật/tắt đèn báo hiệu


const byte digitPatterns[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1}, {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};


bool IsReady(unsigned long &t, uint32_t ms){
  if(millis() - t < ms) return false;
  t += ms;
  return true;
}

unsigned long tSecond = 0;
unsigned long tMux = 0;


enum {RED, GREEN, YELLOW};
int trafficState = RED;
int remainingTime = TIME_RED;
int valueButtonDisplay = HIGH;
bool isDisplayOn = true;

void ClickButtonDisplay(){
  static unsigned long ulTimer = 0;
  
  if (!IsReady(ulTimer, 100)) return;
  
  int currentButtonState = digitalRead(NUT_BAM);

  if (valueButtonDisplay == HIGH && currentButtonState == LOW) {
    isDisplayOn = !isDisplayOn;

    if (!isDisplayOn) {
      digitalWrite(DIG_PINS[0], HIGH);
      digitalWrite(DIG_PINS[1], HIGH);
      digitalWrite(BAO_HIEU_PIN, HIGH);
    } else {
      digitalWrite(BAO_HIEU_PIN, LOW); 
    }
  }

  valueButtonDisplay = currentButtonState;
}
// void checkLightLevel(){
//   int lightLevel = analogRead(LDR_PIN);
//   if(lightLevel < LIGHT_THRESHOLD){
//     digitalWrite(BAO_HIEU_PIN, LOW); // Bật đèn báo hiệu
//   } else {
//     digitalWrite(BAO_HIEU_PIN, HIGH); // Tắt đèn báo hiệu
//   }
// }
void setup() {
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(BAO_HIEU_PIN, OUTPUT);
  pinMode(NUT_BAM, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  Serial.begin(115200);

  for(int i=0;i<7;i++) pinMode(SEG_PINS[i], OUTPUT);
  for(int i=0;i<2;i++) pinMode(DIG_PINS[i], OUTPUT);

  digitalWrite(DIG_PINS[0], HIGH);
  digitalWrite(DIG_PINS[1], HIGH);
  // digitalWrite(BAO_HIEU_PIN, LOW);
}

void setSegments(int num){
  for(int i=0;i<7;i++)
    digitalWrite(SEG_PINS[i], digitPatterns[num][i]);
}

void refreshDisplay(){
  if(!isDisplayOn){
    return;
  }
  static uint8_t digit = 0;

  int val = remainingTime;
  if(val < 0) val = 0;
  if(val > 99) val = 99;

  int chuc = val / 10;
  int donvi = val % 10;

  digitalWrite(DIG_PINS[0], HIGH);
  digitalWrite(DIG_PINS[1], HIGH);

  if(digit == 0){
    setSegments(chuc);
    digitalWrite(DIG_PINS[0], LOW);
  } else {
    setSegments(donvi);
    digitalWrite(DIG_PINS[1], LOW);
  }

  digit ^= 1;
}


void loop() {
  int lightLevel = analogRead(LDR_PIN);
  if(lightLevel < LIGHT_THRESHOLD)
  {
    digitalWrite(BAO_HIEU_PIN, LOW);
    digitalWrite(DIG_PINS[0], HIGH);
    digitalWrite(DIG_PINS[1], HIGH);

    if(IsReady(tSecond, 1000)){
      digitalWrite(LED_YELLOW_PIN, HIGH);
      digitalWrite(LED_RED_PIN,    LOW);
      digitalWrite(LED_GREEN_PIN,  LOW);
    }
  }else{
    
    if(IsReady(tSecond, 1000)){
      remainingTime--;

      if(remainingTime < 0){
        if(trafficState == RED){
          trafficState = GREEN;
          remainingTime = TIME_GREEN;
        }
        else if(trafficState == GREEN){
          trafficState = YELLOW;
          remainingTime = TIME_YELLOW;
        }
        else{
          trafficState = RED;
          remainingTime = TIME_RED;
        }
      }
    }
    
    digitalWrite(LED_RED_PIN,    trafficState == RED);
    digitalWrite(LED_GREEN_PIN,  trafficState == GREEN);
    digitalWrite(LED_YELLOW_PIN, trafficState == YELLOW);

    if(IsReady(tMux, 1)){
      refreshDisplay();
    }
    ClickButtonDisplay();
  }
}