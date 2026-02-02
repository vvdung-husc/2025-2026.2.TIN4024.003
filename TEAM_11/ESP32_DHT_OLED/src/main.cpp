/*
THÔNG TIN NHÓM 11
1. Trần Quốc Tiến
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

}

float getTempC() {
    return dht.readTemperature(); 
}

float gitHumid() {
    return dht.readHumidity();
}

String gettemp(float C){

}

void screenWrite(float C, float H){

}

void Check(){
  
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

