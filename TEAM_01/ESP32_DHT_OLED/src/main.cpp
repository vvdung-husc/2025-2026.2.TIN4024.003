/*
THÔNG TIN NHÓM X
1. Trần Hoàng Lộc - 22T1020654
2. 
3. 
*/

#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHT_PIN     16
#define DHT_TYPE    DHT22

#define LED_GREEN   15   
#define LED_YELLOW  2
#define LED_RED     4

#define OLED_SDA    13
#define OLED_SCL    12
#define OLED_ADDR   0x3C

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(128, 64, &Wire);

unsigned long lastBlink = 0;
bool ledState = false;
int activeLed = -1;

void updateLED(float temp);
void blinkActiveLED();
void updateOLED(float temp, float hum, const char* status);

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  updateLED(temperature);
  blinkActiveLED();

  const char* status;
  if (temperature < 13) status = "TOO COLD";
  else if (temperature < 20) status = "COLD";
  else if (temperature < 25) status = "COOL";
  else if (temperature < 30) status = "WARM";
  else if (temperature < 35) status = "HOT";
  else status = "TOO HOT";

  updateOLED(temperature, humidity, status);

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C | Hum: ");
  Serial.print(humidity);
  Serial.print(" % | Status: ");
  Serial.println(status);

  delay(500);
}


void updateLED(float temp) {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  if (temp < 20) activeLed = LED_GREEN;
  else if (temp < 30) activeLed = LED_YELLOW;
  else activeLed = LED_RED;
}

void blinkActiveLED() {
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    ledState = !ledState;
    if (activeLed != -1) {
      digitalWrite(activeLed, ledState);
    }
  }
}

void updateOLED(float temp, float hum, const char* status) {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("Temp: ");
  display.print(temp, 1);
  display.println(" C");

  display.print("Humi: ");
  display.print(hum, 1);
  display.println(" %");

  display.println();
  display.print("Status: ");
  display.println(status);

  display.display();
}
