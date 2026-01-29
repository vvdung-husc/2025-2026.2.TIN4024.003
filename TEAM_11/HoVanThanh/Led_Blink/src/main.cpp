#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- SỬA LẠI CHÂN CHO KHỚP SƠ ĐỒ ---
#define green_led  32  // Sơ đồ nối chân 32
#define red_led    25  // Sơ đồ nối chân 25
#define yellow_led 33  // Sơ đồ nối chân 33
// -----------------------------------

int colors = 1;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 SSDScreen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void count(String color, int x, int pox ){
  int i = 1;
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
    
    // Nháy đèn theo chân pox
    digitalWrite(pox, HIGH);
    delay(50);
    digitalWrite(pox, LOW);
    delay(50);
    digitalWrite(pox, HIGH);
    delay(50);
    digitalWrite(pox, LOW);
    delay(50);
    i--;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);

  // Khởi động màn hình, nếu lỗi thì in ra nhưng KHÔNG TREO code nữa để test đèn
  if(!SSDScreen.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("Khong tim thay man hinh, nhung van chay den..."));
    // for(;;); // Đã bỏ dòng treo máy này đi để đèn vẫn nháy dù không có màn
  }

  SSDScreen.clearDisplay();
  SSDScreen.setTextSize(2);
  SSDScreen.setTextColor(WHITE);
}

void screen(String color, int x, int pox){
  SSDScreen.clearDisplay();
  SSDScreen.setCursor(0,10);
  count(color, x, pox);
}

void loop() {
  if(colors == 1){
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);
    screen("Green", 40, green_led); // Truyền đúng chân LED vào
    colors++;
  }
  else if(colors == 3){
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, HIGH);
    digitalWrite(yellow_led, LOW);
    screen("Red", 50, red_led);
    colors = 1;
  }
  else if(colors == 2){
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, HIGH);
    screen("Yellow", 35, yellow_led);
    colors++;
  }
}