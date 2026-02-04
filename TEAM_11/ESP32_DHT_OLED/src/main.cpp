/*
THÔNG TIN NHÓM 11
1. Trần Quốc Tiến
2. Trần Đức Quốc Chí
*/

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>

using namespace std;

int8_t red_lead = 4;
int8_t yellow_lead = 2;
int8_t blue_lead = 15;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C


// 1. Cấu hình chân và loại cảm biến
#define DHTPIN 16     
#define DHTTYPE DHT22 

// 2. Khởi tạo đối tượng
DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void turn_led(int led1, int led2, int led3){
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  delay(250);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
}

float getTempC() {
    return dht.readTemperature(); 
}

float gitHumid() {
    return dht.readHumidity();
}

String gettemp(float C){
  if(C < 13) return "TOO COLD";
  else if (C < 20) return "COLD";
  else if (C < 25) return "COOL";
  else if (C < 30) return "WARM";
  else if (C <= 35) return "HOT"; // 30 - 35 : HOT  
  else return "TOO HOT";
}

void screenWrite(float C, float H){
  display.clearDisplay();

  // Dòng 1: "Temperature: HOT"
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.println(gettemp(C));

  // Dòng 2: số to + °C
  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(C, 2);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(String(C, 2), 0, 14, &x1, &y1, &w, &h);
  display.setCursor(w + 6, 14);          // 6px là khoảng cách nhỏ
  display.print((char)247);
  display.print("C");

  // Dòng 3: "Humidity:"
  display.setTextSize(1);
  display.setCursor(0, 38);
  display.println("Humidity:");

  // Dòng 4: số to + %
  display.setTextSize(2);
  display.setCursor(0, 48);
  display.print(H, 2);

  display.getTextBounds(String(H, 2), 0, 48, &x1, &y1, &w, &h);
  display.setCursor(w + 6, 48);
  display.print("%");

  display.display();
}


void Check(){
  float C = getTempC();
  float H = gitHumid();

  if (C < 13){
    turn_led(blue_lead, yellow_lead, red_lead);
    screenWrite(C, H);
  }
  else if (C < 20){
    turn_led(blue_lead, yellow_lead, red_lead);
    screenWrite(C, H);
  }
  else if (C < 25){
    turn_led(yellow_lead, blue_lead, red_lead);
    screenWrite(C, H);
  }
  else if (C < 30){
    turn_led(yellow_lead, blue_lead, red_lead);
    screenWrite(C, H);
  }
  else if (C <= 35){  // 30 - 35 : HOT  
    turn_led(red_lead, yellow_lead, blue_lead);
    screenWrite(C, H);
  }
  else{
    turn_led(red_lead, yellow_lead, blue_lead);
    screenWrite(C, H);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(red_lead, OUTPUT);
  pinMode(yellow_lead, OUTPUT);
  pinMode(blue_lead, OUTPUT);

  Wire.begin(13, 12);
  // SSD1306_SWITCHCAPVCC: Tạo điện áp hiển thị từ nguồn 3.3V bên trong
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed")); 
    for(;;); // Dừng chương trình ngay lập tức
  }
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  dht.begin();
}

void loop() {
  Check();
  delay(500);
}

