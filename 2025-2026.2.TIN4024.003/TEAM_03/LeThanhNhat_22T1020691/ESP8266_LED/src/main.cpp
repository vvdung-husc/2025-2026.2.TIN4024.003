#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define DHTPIN D3
#define DHTTYPE DHT22

#define LED_PIN D4

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  Wire.begin(D2, D1);

  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.print("Temp: ");
  Serial.println(temp);

  display.clearDisplay();

  display.setCursor(0, 10);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.setCursor(0, 30);
  display.print("Hum: ");
  display.print(hum);
  display.println(" %");

  display.display();

  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}