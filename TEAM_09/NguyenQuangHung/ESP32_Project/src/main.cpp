#include <Arduino.h>
#include "DHT.h"

// Dựa theo sơ đồ V3 bạn gửi: DHT nối vào D2 (GPIO 4)
#define DHTPIN 4     
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println(F("\n--- Bat dau do nhiet do ---"));
}

void loop() {
  delay(2000); // DHT11 cần 2 giây để đo một lần

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Loi: Khong doc duoc DHT! Kiem tra day tai chan D2"));
  } else {
    Serial.print(F("Naaaaaaaaa: ")); Serial.print(t);
    Serial.print(F("C | Do am: ")); Serial.print(h); Serial.println(F("%"));
  }
}