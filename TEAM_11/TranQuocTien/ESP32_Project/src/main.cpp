#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Ticker.h>

/* --- CẤU HÌNH --- */
#define PIN_LED      2   // D4
#define PIN_DHT      0   // D3
#define PIN_SDA      4   // D2
#define PIN_SCL      5   // D1
#define OLED_ADDR    0x3C // NẾU VẪN TỐI, HÃY ĐỔI THÀNH 0x3D

DHT dht(PIN_DHT, DHT11);
Adafruit_SH1106G display(128, 64, &Wire, -1);
Ticker taskTimer;

// Biến cờ hiệu (Flag)
volatile bool isReadyToUpdate = false;

// Hàm Ticker: Chỉ bật cờ, không làm việc nặng
void triggerUpdate() {
    isReadyToUpdate = true;
}

void setup() {
    Serial.begin(115200); // Khớp với monitor_speed trong file .ini
    pinMode(PIN_LED, OUTPUT);
    
    dht.begin();
    Wire.begin(PIN_SDA, PIN_SCL);

    // Khởi tạo OLED
    if (!display.begin(OLED_ADDR, true)) {
        Serial.println("OLED Fail!");
        while (1) delay(100);
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Khoi tao xong...");
    display.display();

    // Hẹn giờ bật cờ mỗi 2 giây
    taskTimer.attach(2.0, triggerUpdate);

    Serial.println("System Running...");
}

void loop() {
    // Nháy LED độc lập (Ví dụ Non-blocking đơn giản)
    digitalWrite(PIN_LED, (millis() / 500) % 2);

    // Kiểm tra cờ để cập nhật màn hình
    if (isReadyToUpdate) {
        isReadyToUpdate = false; // Hạ cờ

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Nhiet do: "); display.print(t, 1); display.println(" C");
        display.setCursor(0, 15);
        display.print("Do am:    "); display.print(h, 1); display.println(" %");
        
        display.display(); // Vẽ ở đây là an toàn nhất, không gây crash

        Serial.printf("T: %.1f - H: %.1f\n", t, h);
    }
}