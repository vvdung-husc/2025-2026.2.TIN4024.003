#include <Arduino.h>
#include <TM1637Display.h>

int8_t red = 27;
int8_t green = 25;
int8_t yellow = 26;
int8_t blue = 21;
int8_t button = 23;

#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);
 

void Button(){

} 

void Sensor(){

}

void led_traffict(int color1, int color2, int color3){
  digitalWrite(color1, HIGH);
  digitalWrite(color2, LOW);
  digitalWrite(color3, LOW);
}

void showScreen(int i){
  display.showNumberDec(i, true);
}

void setup_screen(){
  display.setBrightness(7);
  display.clear();
  // display.showNumberDec(0, true); // full số
  display.showNumberDecEx(0, 0x40, true); // Hiển thị dấu 2: ở giữa
  delay(1000);
}

void count(int color1, int color2, int color3){
  led_traffict(color1, color2, color3);
  for(int i = 0; i <= 3; i++){
    showScreen(i);
    delay(1000);
  }
  digitalWrite(color1, LOW);
  delay(100);
}
  

void setup() {
  
  Serial.begin(9600);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(yellow, OUTPUT);
  setup_screen();

  

}

void loop() {
  int i = 1;
  if(i == 1){
    count(green, red, yellow);
    i++;
  }
  if(i == 2){
    count(yellow, green, red);
    i++;
  }
  if(i == 3){
    count(red, yellow, green);
    i++;
  } 
}
