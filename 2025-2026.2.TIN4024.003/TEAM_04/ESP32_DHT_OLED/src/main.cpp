/*
THÔNG TIN NHÓM 4
1. Hồ Gia Long
2. Hồ Trọng Nghĩa
3. Phạm Văn Quân
4. Nguyễn Thắng
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
<<<<<<< HEAD
#include "DHT.h"

/* ================== KHAI BÁO CHÂN ================== */
// DHT22
#define DHTPIN 16
#define DHTTYPE DHT22

// LED
#define LED_RED    4
#define LED_YELLOW 2
#define LED_BLUE   15   // LED màu cyan

// OLED I2C
#define OLED_SDA 13
#define OLED_SCL 12
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/* ================== KHỞI TẠO THIẾT BỊ ================== */
DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

/* ================== HÀM SETUP ================== */
void setup() {
  Serial.begin(115200);
  delay(500);

  // Khởi tạo LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE, LOW);

  // Khởi tạo DHT22
  dht.begin();

  // Khởi tạo I2C cho OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // Khởi tạo OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ Không tìm thấy OLED SSD1306");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ESP32 DHT22 OLED");
  display.println("System Ready...");
  display.display();

  Serial.println("✅ System initialized");
}

/* ================== HÀM LOOP ================== */
void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius

  // Kiểm tra lỗi DHT
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ Loi doc DHT22");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("DHT22 ERROR!");
    display.display();

    delay(2000);
    return;
  }

  /* ======= IN SERIAL ======= */
  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" *C | Do am: ");
  Serial.print(humidity);
  Serial.println(" %");

  /* ======= HIỂN THỊ OLED ======= */
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("ESP32 - DHT22");

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(temperature, 1);
  display.print(" C");

  display.setTextSize(1);
  display.setCursor(0, 42);
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.print(" %");

  display.display();

  /* ======= LOGIC LED ======= */
  /*
    - LED XANH: Nhiệt độ < 25°C
    - LED VÀNG: 25°C – 30°C
    - LED ĐỎ : > 30°C
  */

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE, LOW);

  if (temperature < 25) {
    digitalWrite(LED_BLUE, HIGH);
  }
  else if (temperature < 30) {
    digitalWrite(LED_YELLOW, HIGH);
  }
  else {
    digitalWrite(LED_RED, HIGH);
  }

  delay(2000); // đọc mỗi 2 giây
}
=======
#include <DHT.h>

// --- CẤU HÌNH PHẦN CỨNG ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C
#define OLED_SDA 13
#define OLED_SCL 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 16     
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

#define LED_RED_PIN    4  
#define LED_YELLOW_PIN 2  
#define LED_GREEN_PIN  15 

// --- CÁC BIẾN QUẢN LÝ THỜI GIAN (NON-BLOCKING) ---
unsigned long previousSensorMillis = 0; // Lưu thời gian lần đọc cảm biến trước
const long sensorInterval = 2000;       // Đọc cảm biến mỗi 2000ms (2 giây)

unsigned long previousBlinkMillis = 0;  // Lưu thời gian lần chớp đèn trước
const long blinkInterval = 500;         // Chớp tắt mỗi 500ms

// --- BIẾN TRẠNG THÁI ---
int activeLed = -1;      // Lưu chân LED đang cần bật (theo nhiệt độ)
int ledState = LOW;      // Trạng thái hiện tại của đèn (Tắt/Bật)

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("OLED Error"));
    for(;;);
  }
  
  display.cp437(true);
}

void loop() {
  unsigned long currentMillis = millis(); // Lấy thời gian hiện tại

  // ============================================================
  // TÁC VỤ 1: ĐỌC CẢM BIẾN & CẬP NHẬT MÀN HÌNH (Mỗi 2 giây)
  // ============================================================
  if (currentMillis - previousSensorMillis >= sensorInterval) {
    previousSensorMillis = currentMillis; // Cập nhật thời gian
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Sensor Error!");
      display.display();
    } else {
      // 1. Logic phân loại nhiệt độ
      String statusText = "";
      int newActiveLed = -1;

      if (t < 13.0) {
        statusText = "TOO COLD *"; 
        newActiveLed = LED_GREEN_PIN;
      } 
      else if (t >= 13.0 && t < 20.0) {
        statusText = "COLD '";     
        newActiveLed = LED_GREEN_PIN;
      }
      else if (t >= 20.0 && t < 25.0) {
        statusText = "COOL -";    
        newActiveLed = LED_YELLOW_PIN;
      }
      else if (t >= 25.0 && t < 30.0) {
        statusText = "WARM *";     
        newActiveLed = LED_YELLOW_PIN;
      }
      else if (t >= 30.0 && t <= 35.0) {
        statusText = "HOT !";      
        newActiveLed = LED_RED_PIN;
      }
      else { 
        statusText = "TOO HOT !!"; 
        newActiveLed = LED_RED_PIN;
      }

      // Xử lý chuyển đổi LED:
      // Nếu LED cần bật thay đổi, ta phải tắt ngay LED cũ để tránh nó bị "treo" trạng thái sáng
      if (newActiveLed != activeLed) {
        digitalWrite(LED_RED_PIN, LOW);
        digitalWrite(LED_YELLOW_PIN, LOW);
        digitalWrite(LED_GREEN_PIN, LOW);
        activeLed = newActiveLed;
        ledState = LOW; // Reset trạng thái nhấp nháy
      }

      // 2. Hiển thị lên OLED
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);

      display.setCursor(0, 0);
      display.setTextSize(1);
      display.print("Temp: ");
      display.print(statusText);

      display.setCursor(0, 16);
      display.setTextSize(2);
      display.print(t, 1);
      display.write(248); 
      display.print("C");

      display.setCursor(0, 38);
      display.setTextSize(1);
      display.print("Humidity:");

      display.setCursor(0, 48);
      display.setTextSize(2);
      display.print(h, 1);
      display.print(" %");

      display.display();
    }
  }

  // ============================================================
  // TÁC VỤ 2: NHẤP NHÁY ĐÈN (Mỗi 500ms)
  // ============================================================
  if (currentMillis - previousBlinkMillis >= blinkInterval) {
    previousBlinkMillis = currentMillis; // Cập nhật thời gian

    // Chỉ nhấp nháy nếu có LED được chọn (activeLed != -1)
    if (activeLed != -1) {
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }
      digitalWrite(activeLed, ledState);
    } else {
      // Nếu không có LED nào active, đảm bảo tắt hết
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_GREEN_PIN, LOW);
    }
  }
}
>>>>>>> f41be4aa6fe85225993beff762612d64b89070de
