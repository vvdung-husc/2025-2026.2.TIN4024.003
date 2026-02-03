/*
Thông tin nhóm 10:
1. Đinh Tuấn Anh.
2. Lê Trần Hải Đạt.
3. Phan Thanh Vũ.
*/
#include <Arduino.h>
#include <DHTesp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

unsigned long time_DHT = 0;
unsigned long time_LED = 0; 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 16
DHTesp dht;

#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4

int chon_LED = -1;
String thongtin = "";
bool trang_thai = false;  

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void Led_Off() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

void NhapNhayLED()
{
  if (IsReady(time_LED, 500)) {
    if (chon_LED != -1) {
      trang_thai = !trang_thai;
      Led_Off(); 
      if (trang_thai) {
        digitalWrite(chon_LED, HIGH);
      } else {
        digitalWrite(chon_LED, LOW);
      }
    } else {
      Led_Off();
    }
  }
}
void LogicNhapNhayLED(int nhiet_do)
{
  if (nhiet_do < 13) {
      thongtin = "TOO COLD";
      chon_LED = LED_GREEN;
    }
    else if (nhiet_do < 20) {
      thongtin = "COLD";
      chon_LED = LED_GREEN;
    }
    else if (nhiet_do < 25) {
      thongtin = "COOL";
      chon_LED = LED_YELLOW;
    }
    else if (nhiet_do < 30) {
      thongtin = "WARM";
      chon_LED = LED_YELLOW;
    }
    else if (nhiet_do <= 35) {
      thongtin = "HOT";
      chon_LED = LED_RED;
    }
    else {
      thongtin = "TOO HOT";
      chon_LED = LED_RED;
    }
}
void setup() {
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  Led_Off();
  dht.setup(DHTPIN, DHTesp::DHT22);
  Wire.begin(13, 12); 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED error");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();
}

void loop() {
  if (IsReady(time_DHT, 1000)) { 
    TempAndHumidity data = dht.getTempAndHumidity();
    float nhiet_do = data.temperature;
    float do_am = data.humidity;

    LogicNhapNhayLED(nhiet_do);
    NhapNhayLED();

    // Hiển thị thông tin lên OLED viet tiep
    if (isnan(nhiet_do) || isnan(do_am)) {
      Serial.println("DHT error");
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("DHT Error!");
      display.display();
      chon_LED = -1;
      return; 
    }
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Temperature:");
    display.print(thongtin); 
    display.setTextSize(2);
    display.setCursor(0,15);
    display.print(nhiet_do, 1);
    display.print(" C");

    display.setTextSize(1);
    display.setCursor(0,40);
    display.print("Humidity:");
    
    display.setTextSize(2);
    display.setCursor(0,50);
    display.print(do_am, 1);
    display.print(" %");

    display.display();

    Serial.print("Temp: "); Serial.print(nhiet_do);
    Serial.print(" - LED Pin: "); Serial.println(chon_LED);
  }
}