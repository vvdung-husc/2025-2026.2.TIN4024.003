#include <Arduino.h>
#include <TM1637Display.h>
#include <Wire.h>
#include <RTClib.h> 

// --- KHAI BÁO CHÂN ---
int8_t red = 25;    
int8_t green = 32;  
int8_t yellow = 33; 
int8_t blue = 26;   
int8_t button = 23;
int8_t AO = 13;

bool checkBtn = true;
int Brightness = 0;

const int FREQ = 5000;  
const int RES = 8;      

#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

RTC_DS1307 rtc; 

// --- CÁC HÀM CẤU HÌNH ---
void setup_Button(){
  pinMode(button, INPUT_PULLUP);
} 

void setup_screen(){
  display.setBrightness(7);
  display.clear();
  display.showNumberDecEx(0, 0x40, true);
}

void turn_on_Off(bool turn){
  if (turn == true)
    digitalWrite(blue, HIGH);
  else
    digitalWrite(blue, LOW);
  display.setBrightness(7, turn);
}


void night(){
  ledcWrite(0, 0); 
  ledcWrite(1, 0); 
  display.clear(); 
  
  int trang_thai = (millis() / 500) % 2; 
  ledcWrite(2, trang_thai * 255); 
}

void led_traffict(int light1, int light2, int light3){
  ledcWrite(0, light1); 
  ledcWrite(1, light2); 
  ledcWrite(2, light3); 
}

bool setTime() {
  if (Serial.available() > 0) {
    int gio_moi = Serial.parseInt();
    int phut_moi = Serial.parseInt();

   
    while(Serial.available()) Serial.read(); 

    if (gio_moi >= 0 && gio_moi <= 23 && phut_moi >= 0 && phut_moi <= 59) {
      DateTime now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), gio_moi, phut_moi, 0));
      
      Serial.print(">>> DA CAP NHAT: ");
      Serial.print(gio_moi);
      Serial.print(":");
      Serial.println(phut_moi);
      return true; 
    }
  }
  return false; 
}

void count(int light1, int light2, int light3, int thoi_gian){
  led_traffict(light1, light2, light3);
  
  for(int s = thoi_gian; s >= 0; s--){
    
    DateTime now = rtc.now();
    if (now.hour() >= 22 || now.hour() < 5) return; 

    if (checkBtn == true) {
      display.showNumberDec(s, true);
    } else {
      display.clear(); 
    }

    for (int k = 0; k < 50; k++) {
       
       if (setTime()) {
          return;
       }
       
       if (digitalRead(button) == 0) { 
          checkBtn = !checkBtn;
          turn_on_Off(checkBtn);
          while(digitalRead(button) == 0); 
       }
       delay(20); 
    }
  }
  
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
}

// --- SETUP ---
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(150); 
  pinMode(blue, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  
  ledcSetup(0, FREQ, RES);
  ledcSetup(1, FREQ, RES);
  ledcSetup(2, FREQ, RES);
  ledcAttachPin(red, 0);
  ledcAttachPin(green, 1);
  ledcAttachPin(yellow, 2);

  setup_screen();
  digitalWrite(blue, HIGH);

  if (! rtc.begin()) {
    Serial.println("Khong tim thay RTC");
    while (1);
  }
}
// --- LOOP ---
void loop() {
  setTime(); 
 
  DateTime now = rtc.now();
  int gio = now.hour();
  int phut = now.minute();

  if (gio >= 22 || gio < 5) {
    night();
    delay(50); 
  } 
  else {
    count(0, 255, 0, 10); // Xanh
    count(0, 0, 255, 3);  // Vàng
    count(255, 0, 0, 10); // Đỏ
  
    
  }
}