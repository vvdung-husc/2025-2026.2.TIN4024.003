#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED_PIN D4
#define DHTPIN  D3
#define DHTTYPE DHT11 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Khởi tạo màn hình
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);

bool ledState = false;
unsigned long lastBlink = 0;
unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  Wire.begin(D2, D1); // SDA, SCL

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    for(;;);
  }

  // --- TỐI ƯU HIỂN THỊ ---
  display.clearDisplay();      // Xóa sạch bộ đệm (đảm bảo nền đen)
  display.setTextColor(SSD1306_WHITE); // Chữ màu trắng trên nền đen
  display.setTextSize(2);      
  display.display();

  Serial.println("System Start...");
}

void loop() {
  // LED NHẤP NHÁY
  if(millis() - lastBlink > 300){
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }

  // ĐỌC DHT MỖI 2 GIÂY
  if(millis() - lastRead > 2000){
    lastRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if(!isnan(h) && !isnan(t)){
      // Xóa màn hình cũ để vẽ chữ mới trên nền đen hoàn toàn
      display.clearDisplay(); 

      // Dòng 1: Nhiệt độ
      display.setCursor(0, 5); 
      display.print("T:");
      display.print(t, 1);
      display.write(247); // Ký hiệu độ (°)
      display.print("C");

      // Dòng 2: Độ ẩm
      display.setCursor(0, 35);
      display.print("H:");
      display.print(h, 1);
      display.print("%");

      display.display(); // Đẩy dữ liệu ra màn hình

      // Terminal xuất dữ liệu
      Serial.printf("Temp: %.1f C | Humi: %.1f %%\n", t, h);
    } else {
      Serial.println("Error reading DHT sensor!");
    }
  }
}