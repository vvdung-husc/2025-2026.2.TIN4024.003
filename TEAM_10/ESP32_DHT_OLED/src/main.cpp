/*
Thông tin nhóm 10:
1. Đinh Tuấn Anh.
2. Lê Trần Hải Đạt.
*/
#include <Arduino.h>
#include <DHTesp.h>

const int DHT_PIN = 15;

DHTesp dhtSensor;

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop() {
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
  Serial.println("---");
  delay(2000); // Wait for a new reading from the sensor (DHT22 has ~0.5Hz sample rate)
}