/*
 * ESP8266 DHT + OLED Project - Debug Version
 * 
 * CAU HINH CHAN - THU CAC GIA TRI NAY NEU KHONG HOAT DONG:
 * 
 * OLED (I2C):
 *   - SDA: D2 (GPIO4) - mac dinh
 *   - SCL: D1 (GPIO5) - mac dinh
 * 
 * DHT Sensor:
 *   - DATA: D4 (GPIO2) - dang dung
 *   - Neu khong hoat dong, thu:
 *     + D3 (GPIO0)
 *     + D5 (GPIO14) 
 *     + D6 (GPIO12)
 *     + D7 (GPIO13)
 * 
 * LED Built-in:
 *   - GPIO2 (D4) - hoac GPIO1 tuy board
 * 
 * LUU Y:
 *   - DHT can dien tro keo len 10K ohm giua DATA va VCC
 *   - OLED dung nguon 3.3V hoac 5V
 *   - DHT dung nguon 3.3V-5V
 *   - Kiem tra Serial Monitor (115200 baud) de xem log debug
 */

#include <Wire.h>
#include <U8g2lib.h>

// --- Chọn cảm biến ---
#define USE_DHT         // Dùng DHT11/DHT22
// #define USE_DS18B20  // Dùng DS18B20

// --- Cấu hình chân theo bo mạch thực tế ---
// I2C cho OLED (chuẩn cho NodeMCU)
const uint8_t OLED_SDA = D2; // GPIO4
const uint8_t OLED_SCL = D1; // GPIO5

#ifdef USE_DHT
  #include <DHT.h>
  // Thử các chân khác nhau để tìm chân đúng
  #define DHTTYPE DHT22  // Đổi thành DHT11 nếu bạn dùng DHT11
  const uint8_t DHT_PIN = D3; // D3 (GPIO0)
  DHT dht(DHT_PIN, DHTTYPE);
#endif

#ifdef USE_DS18B20
  #include <OneWire.h>
  #include <DallasTemperature.h>
  const uint8_t DS_PIN = D6; // GPIO12
  OneWire oneWire(DS_PIN);
  DallasTemperature sensors(&oneWire);
#endif

// U8g2 constructor cho SH1106 I2C 128x64, full buffer
// Thử cả địa chỉ 0x3C (mặc định) và 0x3D
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA);

const unsigned long TEMP_INTERVAL = 2000; // đọc nhiệt độ mỗi 2s
const unsigned long BLINK_INTERVAL = 500; // nhấp nháy LED 500ms

unsigned long lastTempMillis = 0;
unsigned long lastBlinkMillis = 0;
bool ledState = false;

void setup() {
  // Serial cho debug
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("=====================================");
  Serial.println("ESP8266 DHT + OLED Debug Mode");
  Serial.println("=====================================");
  
  // In thông tin chân
  Serial.println("\n*** CAU HINH CHAN ***");
  Serial.print("OLED_SDA: D");
  Serial.print((OLED_SDA - D0));
  Serial.print(" (GPIO");
  Serial.print(OLED_SDA);
  Serial.println(")");
  Serial.print("OLED_SCL: D");
  Serial.print((OLED_SCL - D0));
  Serial.print(" (GPIO");
  Serial.print(OLED_SCL);
  Serial.println(")");
  
  #ifdef USE_DHT
    Serial.print("DHT_PIN: D");
    Serial.print((DHT_PIN - D0));
    Serial.print(" (GPIO");
    Serial.print(DHT_PIN);
    Serial.println(")");
    Serial.print("DHT Type: ");
    Serial.println(DHTTYPE == DHT22 ? "DHT22" : "DHT11");
  #endif

  // Khởi tạo I2C
  Serial.println("\n*** KHOI TAO I2C ***");
  Wire.begin(OLED_SDA, OLED_SCL);
  delay(100);
  
  // Scan I2C devices
  Serial.println("Scanning I2C bus...");
  byte count = 0;
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at address 0x");
      if (i < 16) Serial.print("0");
      Serial.println(i, HEX);
      count++;
    }
  }
  if (count == 0) {
    Serial.println("WARNING: Khong tim thay thiet bi I2C nao!");
    Serial.println("- Kiem tra day noi SDA/SCL");
    Serial.println("- Kiem tra nguon cho OLED");
  } else {
    Serial.print("Tim thay ");
    Serial.print(count);
    Serial.println(" thiet bi I2C");
  }

  // Khởi tạo màn hình OLED
  Serial.println("\n*** KHOI TAO OLED ***");
  if (u8g2.begin()) {
    Serial.println("OLED: Khoi tao THANH CONG!");
  } else {
    Serial.println("OLED: LOI khoi tao!");
  }

  // Khởi tạo cảm biến
  #ifdef USE_DHT
    Serial.println("\n*** KHOI TAO DHT SENSOR ***");
    dht.begin();
    Serial.println("DHT: Da khoi tao");
    delay(2000); // Đợi DHT ổn định
    
    // Test đọc DHT
    Serial.println("Testing DHT sensor...");
    float testTemp = dht.readTemperature();
    float testHum = dht.readHumidity();
    
    if (isnan(testTemp) || isnan(testHum)) {
      Serial.println("DHT: LOI - Khong doc duoc du lieu!");
      Serial.println("- Kiem tra day noi chan DATA");
      Serial.println("- Kiem tra nguon VCC/GND");
      Serial.print("- Thu doi chan khac: D3(GPIO0), D5(GPIO14), D6(GPIO12), D7(GPIO13)");
    } else {
      Serial.println("DHT: Doc du lieu THANH CONG!");
      Serial.print("- Nhiet do: "); Serial.print(testTemp); Serial.println(" *C");
      Serial.print("- Do am: "); Serial.print(testHum); Serial.println(" %");
    }
  #endif
  
  #ifdef USE_DS18B20
    Serial.println("\n*** KHOI TAO DS18B20 ***");
    sensors.begin();
    Serial.print("Found ");
    Serial.print(sensors.getDeviceCount());
    Serial.println(" DS18B20 devices");
  #endif

  // LED built-in
  Serial.println("\n*** KHOI TAO LED ***");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Tắt LED (active LOW trên ESP8266)
  Serial.print("LED_BUILTIN: GPIO");
  Serial.println(LED_BUILTIN);
  
  Serial.println("\n=====================================");
  Serial.println("Setup hoan tat! Bat dau loop...");
  Serial.println("=====================================\n");
}

void loop() {
  unsigned long now = millis();
  static int readCount = 0; // Đếm số lần đọc

  // --- Nhấp nháy LED ---
  if (now - lastBlinkMillis >= BLINK_INTERVAL) {
    lastBlinkMillis = now;
    ledState = !ledState;
    // ESP8266 LED_BUILTIN là active LOW: LOW=sáng, HIGH=tắt
    digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
    // Log LED state occasionally
    if (readCount == 0) {
      Serial.print("[LED] State: ");
      Serial.println(ledState ? "ON (LOW)" : "OFF (HIGH)");
    }
  }

  // --- Đọc nhiệt độ theo interval ---
  if (now - lastTempMillis >= TEMP_INTERVAL) {
    lastTempMillis = now;
    readCount++;

    Serial.println("\n----- Doc du lieu sensor (lan " + String(readCount) + ") -----");
    
    float tempC = NAN;
    float hum = NAN;
    bool ok = false;

    #ifdef USE_DHT
      Serial.println("[DHT] Bat dau doc...");
      
      // Đọc nhiệt độ và độ ẩm
      tempC = dht.readTemperature();
      hum = dht.readHumidity();
      
      // Kiểm tra chi tiết
      Serial.print("[DHT] Temperature raw: ");
      Serial.println(tempC);
      Serial.print("[DHT] Humidity raw: ");
      Serial.println(hum);
      
      if (isnan(tempC)) {
        Serial.println("[DHT] ERROR: Temperature = NAN");
      }
      if (isnan(hum)) {
        Serial.println("[DHT] ERROR: Humidity = NAN");
      }
      
      if (!isnan(tempC) && !isnan(hum)) {
        ok = true;
        Serial.println("[DHT] SUCCESS: Doc du lieu thanh cong!");
      } else {
        Serial.println("[DHT] FAILED: Loi doc du lieu!");
        Serial.println("  -> Kiem tra:");
        Serial.println("     1. Day noi DATA vao chan D4 (GPIO2)");
        Serial.println("     2. Nguon VCC (3.3V hoac 5V)");
        Serial.println("     3. Day GND");
        Serial.println("     4. Dien tro keo len 10K ohm (neu can)");
      }
    #endif

    #ifdef USE_DS18B20
      Serial.println("[DS18B20] Requesting temperatures...");
      sensors.requestTemperatures();
      tempC = sensors.getTempCByIndex(0);
      Serial.print("[DS18B20] Temperature: ");
      Serial.println(tempC);
      
      if (tempC != DEVICE_DISCONNECTED_C && tempC != -127.0) {
        ok = true;
        Serial.println("[DS18B20] SUCCESS!");
      } else {
        Serial.println("[DS18B20] FAILED: Device disconnected!");
      }
    #endif

    // Hiển thị lên Serial
    if (ok) {
      Serial.println("\n=== KET QUA ===");
      Serial.print("Nhiet do: ");
      Serial.print(tempC, 1);
      Serial.println(" *C");
      #ifdef USE_DHT
        Serial.print("Do am: ");
        Serial.print(hum, 1);
        Serial.println(" %");
      #endif
    } else {
      Serial.println("\n=== LOI: Khong doc duoc cam bien! ===");
    }

    // --- Vẽ lên OLED ---
    Serial.println("[OLED] Cap nhat man hinh...");
    u8g2.clearBuffer();
    
    // Tiêu đề - Tên thiết bị
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 8, "ESP8266 IOT");
    
    // Nhiệt độ
    u8g2.setFont(u8g2_font_ncenB10_tr);
    char line1[32];
    if (ok) {
      snprintf(line1, sizeof(line1), "Temp: %.1f C", tempC);
    } else {
      snprintf(line1, sizeof(line1), "Temp: --.- C");
    }
    u8g2.drawStr(0, 23, line1);

    #ifdef USE_DHT
      u8g2.setFont(u8g2_font_7x14_tr);
      char line2[32];
      if (!isnan(hum)) {
        snprintf(line2, sizeof(line2), "Hum: %.1f %%", hum);
      } else {
        snprintf(line2, sizeof(line2), "Hum: --.- %%");
      }
      u8g2.drawStr(0, 40, line2);
    #endif

    // Vẽ trạng thái LED (ledState: true=sáng, false=tắt)
    u8g2.setFont(u8g2_font_6x12_tr);
    if (ledState) {
      u8g2.drawStr(0, 54, "Led On");
      Serial.println("[DISPLAY] Hien thi: Led On");
    } else {
      u8g2.drawStr(0, 54, "Led Off");
      Serial.println("[DISPLAY] Hien thi: Led Off");
    }

    u8g2.sendBuffer();
    Serial.println("[OLED] Da cap nhat!");
    Serial.println("--------------------------------\n");
  }
}