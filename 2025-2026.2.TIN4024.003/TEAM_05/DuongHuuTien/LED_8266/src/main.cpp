#include <Arduino.h>

// 1. Khai báo chân chuẩn xác theo bo mạch của bạn
#define RELAY1 15      // Chân D8 - Relay 1
#define RELAY2 13      // Chân D7 - Relay 2
#define GAS_SENSOR A0  // Chân A0 - Cảm biến khí Gas/Khói MQ

void setup() {
    Serial.begin(115200); // Bật Serial Monitor với tốc độ 115200
    
    // Cài đặt 2 Relay là OUTPUT
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
}

void loop() {
    // 2. Đọc giá trị từ cảm biến Analog (ESP8266 đọc từ 0 đến 1023)
    int rawValue = analogRead(GAS_SENSOR);

    // Hiển thị giá trị cảm biến Gas lên Serial Monitor
    Serial.print("Gia tri cam bien khoi/gas: ");
    Serial.println(rawValue);

    // 3. Nháy 2 cái Relay lần lượt (Thay vì nháy 3 LED)
    // Bạn nhớ cấp nguồn 12V cho mạch để nghe Relay kêu "tạch tạch" nhé
    digitalWrite(RELAY1, HIGH);
    delay(500);
    digitalWrite(RELAY1, LOW);

    digitalWrite(RELAY2, HIGH);
    delay(500);
    digitalWrite(RELAY2, LOW);

    // Chờ 0.5s trước khi lặp lại để dễ đọc Serial Monitor
    delay(500); 
}