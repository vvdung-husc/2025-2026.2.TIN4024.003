#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h> // Thư viện để vẽ lên màn hình
#include <Adafruit_SSD1306.h>

#define green_led  23
#define red_led    19 
#define yellow_led 18
int colors = 1;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 SSDScreen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
void count(String color, int x){
  int i = 3;
  while(i >= 0){
    SSDScreen.clearDisplay(); 
    SSDScreen.setCursor(x, 10);
    SSDScreen.setTextSize(2);      
    SSDScreen.setTextColor(WHITE);
    SSDScreen.println(color);
    SSDScreen.setCursor(60, 30);
    SSDScreen.setTextSize(3); 
    SSDScreen.print(i);
    SSDScreen.display();
    delay(1000);
    i--;
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);

  if(!SSDScreen.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("Chưa bật màn hình"));
    for(;;); // Treo màn hình
  }

  // SSDScreen.setCursor(0,10);
  SSDScreen.clearDisplay();
  SSDScreen.setTextSize(2);      
  SSDScreen.setTextColor(WHITE);
}

void screen(String color, int x){
  SSDScreen.clearDisplay();
  SSDScreen.setCursor(0,10);
  count(color, x);
}

void loop() {
  if(colors == 1){
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);
    screen("Green", 40);
    colors++;
  }
  else if(colors == 3){
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, HIGH);
    digitalWrite(yellow_led, LOW);
    screen("Red", 50);
    colors = 1;
  }
  else if(colors == 2){
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, HIGH);
    screen("Yellow", 35);
    colors ++;
  }
}