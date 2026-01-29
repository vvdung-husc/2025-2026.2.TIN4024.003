#include <Arduino.h>

const int LED_RED_PIN    = 15;
const int LED_GREEN_PIN  = 2;   
const int LED_YELLOW_PIN = 4;
const int BAO_HIEU_PIN   = 17;
const int NUT_BAM   = 23;  

const int SEG_PINS[7] = {13,12,14,27,26,25,33};
const int DIG_PINS[2] = {18,19};

const int TIME_RED    = 11;
const int TIME_GREEN  = 10;
const int TIME_YELLOW = 5;


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
  static int lastButtonState = HIGH; // Lưu trạng thái trước đó của nút bấm
  
  if (!IsReady(ulTimer, 50)) return; // Tăng debounce lên 50ms để chống nhiễu tốt hơn
  
  int currentButtonState = digitalRead(NUT_BAM);

  // Kiểm tra sự kiện nhấn nút (Phát hiện cạnh xuống: từ HIGH sang LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isDisplayOn = !isDisplayOn; // Đảo trạng thái: nếu đang true thì thành false và ngược lại

    if (!isDisplayOn) {
      // KHI TẮT BẢNG TIN
      digitalWrite(DIG_PINS[0], LOW); // Tắt LED 7 đoạn (Cathode chung dùng LOW)
      digitalWrite(DIG_PINS[1], LOW);
      digitalWrite(BAO_HIEU_PIN, HIGH); // Đèn màu xanh dương SÁNG
    } else {
      // KHI BẬT BẢNG TIN
      digitalWrite(BAO_HIEU_PIN, LOW);  // Đèn màu xanh dương TẮT
    }
  }

  lastButtonState = currentButtonState; // Cập nhật trạng thái nút cho lần quét sau
}

void setup() {
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(BAO_HIEU_PIN, OUTPUT);
  pinMode(NUT_BAM, INPUT_PULLUP);

  for(int i=0;i<7;i++) pinMode(SEG_PINS[i], OUTPUT);
  for(int i=0;i<2;i++) pinMode(DIG_PINS[i], OUTPUT);

  digitalWrite(DIG_PINS[0], HIGH);
  digitalWrite(DIG_PINS[1], HIGH);
  digitalWrite(BAO_HIEU_PIN, HIGH);
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