#include <Arduino.h>
#include <TM1637Display.h>

int8_t red = 27;
int8_t green = 25;
int8_t yellow = 26;
int8_t blue = 21;
int8_t button = 23;
bool checkBtn = true;
int8_t AO = 13;
int Brightness = analogRead(AO);

const int FREQ = 5000;   // Tần số chớp (5000Hz - rất nhanh, mắt ko thấy rung)
const int RES = 8;       // Độ phân giải 8-bit (chỉnh từ 0-255)


#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);


void setup_Button(){
  pinMode(button, INPUT_PULLUP);
} 

void night(){
  display.clear();
  display.setBrightness(0, false); 
  ledcWrite(0, 0);

  ledcWrite(1, 0);
  for (int i = 0; i < 60; i++) {
      ledcWrite(2, 100);
      delay(5);
      Brightness = analogRead(AO);
      if (Brightness <= 2800) return; 
    }

    for (int i = 0; i < 60; i++) {
      ledcWrite(2, 0);
      delay(5);
      Brightness = analogRead(AO);
      if (Brightness <= 2800) return;
    }
}

void led_traffict(int light1, int light2, int light3){
  ledcWrite(0, light1); // đỏ
  ledcWrite(1, light2); // xanh
  ledcWrite(2, light3); // vàng

}

void showScreen(int i){
  display.showNumberDec(i, true);
}

void setup_screen(){
  display.setBrightness(7);
  display.clear();
  display.showNumberDecEx(0, 0x40, true); // Hiển thị dấu 2: ở giữa
  delay(1000);
}

void turn_on_Off(bool turn){
  if (turn == true)
    digitalWrite(blue, HIGH);
  else
    digitalWrite(blue, LOW);
  display.setBrightness(7, turn);
}
  
void checkSensor(int light1, int light2, int light3){
  if(Brightness <= 2800)
    led_traffict(light1, light2, light3); 
  else
    night();
}

void count(int light1, int light2, int light3){
  Brightness = analogRead(AO);
  checkSensor(light1, light2, light3);
  for(int s = 3; s >= 0; s--){
    if (checkBtn == true && Brightness <= 2800) {
      display.showNumberDec(s, true);
    } else {
      display.clear(); 
      display.setBrightness(7, LOW);
    }

    for (int k = 0; k < 50; k++) {
      Brightness = analogRead(AO);
      Serial.println(Brightness);
      checkSensor(light1, light2, light3);
      if (digitalRead(button) == 0) { 
          checkBtn = !checkBtn;
          turn_on_Off(checkBtn);
          if (checkBtn == true ){
            display.showNumberDec(s, true);
          }

          while(digitalRead(button) == 0);
      }

      if(Brightness > 2800){
        digitalWrite(blue, LOW);
        s = 4;
        night();
        // display.setBrightness(7, false); 
      } 
      else {
        if (checkBtn == true)
          digitalWrite(blue, HIGH);
        display.setBrightness(7, true); 
      }
        
      delay(20); 
    }
  }
  
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  delay(50);
}

void setup() {
  
  Serial.begin(9600);
  // pinMode(red, OUTPUT);
  // pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  // pinMode(yellow, OUTPUT);
  setup_screen();
  digitalWrite(blue, HIGH);
  Serial.println(Brightness); // In thử coi sao
  ledcSetup(0, FREQ, RES);
  ledcSetup(1, FREQ, RES);
  ledcSetup(2, FREQ, RES);
  ledcAttachPin(red, 0);
  ledcAttachPin(green, 1);
  ledcAttachPin(yellow, 2);

}

void loop() {
  count(0, 255, 0);

  count(0, 0, 255);

  count(255, 0, 0);
}
