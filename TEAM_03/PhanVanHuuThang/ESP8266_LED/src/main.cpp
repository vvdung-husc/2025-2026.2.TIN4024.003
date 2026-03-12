#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

/*
 * =============================================
 * DỰ ÁN: ESP8266 - LED + DHT + OLED SH1106
 * =============================================
 * 
 * CHỨC NĂNG:
 * 1. Điều khiển LED nhấp nháy
 * 2. Đọc nhiệt độ và độ ẩm từ DHT11/DHT22
 * 3. Hiển thị lên màn hình OLED SH1106
 * 
 * SƠ ĐỒ ĐẤU NỐI:
 * 
 * DHT11/DHT22:
 *   - VCC  -> 3.3V
 *   - GND  -> GND
 *   - DATA -> D3 (GPIO0)
 * 
 * OLED SH1106 (I2C):
 *   - VCC -> 3.3V
 *   - GND -> GND
 *   - SCL -> D1 (GPIO5)
 *   - SDA -> D2 (GPIO4)
 * 
 * LED:
 *   - D5 (GPIO14) -> 220Ω -> LED+ -> LED- -> GND
 */

// ===== CẤU HÌNH PHẦN CỨNG =====
#define LED_PIN D5           // Chân LED ngoài
#define LED_BUILTIN_PIN 2    // LED onboard ESP8266 (GPIO2)
#define DHT_PIN D3           // Chân DHT sensor
#define DHT_TYPE DHT22       // DHT11 hoặc DHT22

// Thời gian cập nhật
#define LED_INTERVAL 1000     // LED nhấp nháy mỗi 1 giây
#define DHT_INTERVAL 2000     // Đọc DHT mỗi 2 giây

// ===== KHỞI TẠO CÁC MODULE =====
// OLED SH1106 128x64 I2C (0x3C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// DHT Sensor
DHT dht(DHT_PIN, DHT_TYPE);

// ===== BIẾN TOÀN CỤC =====
unsigned long previousLedMillis = 0;
unsigned long previousDhtMillis = 0;
bool ledState = false;

float temperature = 0.0;
float humidity = 0.0;
bool dhtError = false;

// ===== HÀM HIỂN THỊ LÊN OLED =====
void displayOnOLED() {
    u8g2.clearBuffer();
    
    // Tiêu đề
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(10, 15, "ESP8266 IOT");
    
    // Vẽ đường kẻ
    u8g2.drawLine(0, 20, 128, 20);
    
    // Hiển thị nhiệt độ
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5, 35, "Temp:");
    
    if (!dhtError) {
        char tempStr[10];
        dtostrf(temperature, 5, 1, tempStr);
        strcat(tempStr, " C");
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.drawStr(50, 37, tempStr);
    } else {
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(50, 35, "ERROR");
    }
    
    // Hiển thị độ ẩm
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5, 50, "Hum:");
    
    if (!dhtError) {
        char humStr[10];
        dtostrf(humidity, 5, 1, humStr);
        strcat(humStr, " %");
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.drawStr(50, 52, humStr);
    } else {
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(50, 50, "ERROR");
    }
    
    // Hiển thị trạng thái LED
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5, 63, "LED:");
    if (ledState) {
        u8g2.drawStr(40, 63, "ON");
        u8g2.drawDisc(60, 60, 3);  // Vẽ chấm tròn
    } else {
        u8g2.drawStr(40, 63, "OFF");
        u8g2.drawCircle(60, 60, 3); // Vẽ vòng tròn rỗng
    }
    
    u8g2.sendBuffer();
}

// ===== HÀM ĐỌC DHT SENSOR =====
void readDHTSensor() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (isnan(h) || isnan(t)) {
        dhtError = true;
        Serial.println("❌ Lỗi đọc DHT sensor!");
    } else {
        dhtError = false;
        temperature = t;
        humidity = h;
        
        Serial.println("=============================");
        Serial.print("🌡️  Nhiệt độ: ");
        Serial.print(temperature);
        Serial.println(" °C");
        Serial.print("💧 Độ ẩm: ");
        Serial.print(humidity);
        Serial.println(" %");
        Serial.println("=============================");
    }
}

// ===== SETUP =====
void setup() {
    // Khởi tạo Serial
    Serial.begin(115200);
    delay(500);
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════╗");
    Serial.println("║  ESP8266 IOT PROJECT               ║");
    Serial.println("║  LED + DHT + OLED SH1106           ║");
    Serial.println("╚════════════════════════════════════╝");
    
    // Khởi tạo LED
    pinMode(LED_PIN, OUTPUT);
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_BUILTIN_PIN, HIGH);  // LED onboard tắt (active LOW)
    Serial.println("✓ LED initialized on D5 (External) + GPIO2 (Onboard)");
    
    // Khởi tạo DHT
    dht.begin();
    Serial.println("✓ DHT sensor initialized on D3");
    
    // Khởi tạo OLED
    u8g2.begin();
    u8g2.setContrast(255); // Độ sáng tối đa
    Serial.println("✓ OLED SH1106 initialized");
    
    // Hiển thị màn hình khởi động
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.drawStr(10, 30, "ESP8266");
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(25, 50, "Starting...");
    u8g2.sendBuffer();
    
    delay(2000);
    
    // Đọc dữ liệu DHT lần đầu
    readDHTSensor();
    displayOnOLED();
    
    Serial.println("\n✓ System Ready!\n");
}

// ===== LOOP =====
void loop() {
    unsigned long currentMillis = millis();
    
    // Điều khiển LED nhấp nháy (không dùng delay)
    if (currentMillis - previousLedMillis >= LED_INTERVAL) {
        previousLedMillis = currentMillis;
        ledState = !ledState;
        
        if (ledState) {
            // BẬT LED
            digitalWrite(LED_PIN, HIGH);           // LED ngoài: HIGH = bật
            digitalWrite(LED_BUILTIN_PIN, LOW);     // LED onboard: LOW = bật
            Serial.println("💡 LED: ON");
        } else {
            // TẮT LED
            digitalWrite(LED_PIN, LOW);            // LED ngoài: LOW = tắt
            digitalWrite(LED_BUILTIN_PIN, HIGH);
                Serial.println("💡 LED: OFF");
        }
        
        // Cập nhật hiển thị
        displayOnOLED();
    }
    
    // Đọc DHT sensor định kỳ
    if (currentMillis - previousDhtMillis >= DHT_INTERVAL) {
        previousDhtMillis = currentMillis;
        readDHTSensor();
        displayOnOLED();
    }
}