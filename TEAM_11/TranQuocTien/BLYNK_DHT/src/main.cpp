#include <Arduino.h>
#include <DHT.h>
#include <TM1637Display.h>

const int8_t blue_led = 21;
const int8_t button = 23;

// --- Cấu hình cảm biến ---
#define DHTPIN  16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Cấu hình màn hình TM1637 ---
#define CLK 18
#define DIO 19
TM1637Display tm_display(CLK, DIO);

// Hàm cấu hình riêng cho màn hình
void setup_screen() {
  tm_display.setBrightness(7);
  tm_display.clear();

  // tm_display.showNumberDecEx(0, 0x40, true, 4, 0); 
}

// Hàm cấu hình riêng cho cảm biến
void setup_sense() {
  dht.begin();
}

// DHT22 trả về số thập phân, nên dùng float để không mất dữ liệu
float readTem() {
  return dht.readTemperature();
}

float readHum() {
  return dht.readHumidity();
}

// Hàm hiển thị số nguyên lên màn hình 7 đoạn
void showScreen(int i) {
  // false: Ẩn các số 0 vô nghĩa ở đầu (ví dụ in '  12' thay vì '0012')
  tm_display.showNumberDec(i, false);
}

void turn_on_Off(bool turn) {
  if (turn == true)
    digitalWrite(blue_led, HIGH);
  else
    digitalWrite(blue_led, LOW);
  tm_display.setBrightness(7, turn);
}

void run(){
  bool check= true;
  int i=-1;
  float tem = readTem();
  float hum = readHum();
  showScreen(i+1);
  int lastbutt= 1;
  while (true)
  {
    for(int j=0; j< 100; j++){
      int currentbutt= digitalRead(button);
      if(currentbutt == 0 && lastbutt == 1){
        check= !check;
        turn_on_Off(check);
        i=-1;
      }
      lastbutt= currentbutt;
      
      delay(10);
    }
    i++;
    showScreen(i+1);
  }
  
}

void setup() {
  Serial.begin(9600);
  
  pinMode(blue_led, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  
  digitalWrite(blue_led, HIGH);

  setup_screen();
  setup_sense();
  
  delay(1000);
}

void loop() {
  setup();
  run();
}