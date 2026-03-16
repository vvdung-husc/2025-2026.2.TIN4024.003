#include <DHT.h>

#define DHTTYPE DHT11

// Thử lần lượt các chân từ D1 đến D8
int pins[] = {5, 4, 0, 2, 14, 12, 13, 15}; // GPIO tương ứng với D1, D2, D3, D4, D5, D6, D7, D8

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < 8; i++) {
    DHT dht(pins[i], DHTTYPE);
    dht.begin();
    delay(2000); // Đợi cảm biến khởi động
    float t = dht.readTemperature();
    
    if (!isnan(t)) {
      Serial.print("TIM THAY CHAN DUNG! Chan GPIO: ");
      Serial.print(pins[i]);
      Serial.print(" | Nhiet do: ");
      Serial.println(t);
      delay(5000);
    }
  }
}