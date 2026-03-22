/*
THÔNG TIN NHÓM TEAM_03
1. Trương Trường Phúc - 22T1020328
2. Phan Văn Hữu Thắng - 22T1020427
3. Hoàng Văn Thắng - 22T1020742
4. Nguyễn Đức Dương - 22T1020087
<<<<<<< HEAD
=======
5. Lê Thanh Nhật - 22T1020691
>>>>>>> f41be4aa6fe85225993beff762612d64b89070de
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Cấu hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
#define OLED_SDA 13
#define OLED_SCL 12

// Cấu hình DHT22
#define DHTPIN 16
#define DHTTYPE DHT22

// Cấu hình LED pins
#define LED_GREEN  15
#define LED_YELLOW 2
#define LED_RED    4

// Khởi tạo đối tượng
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

// Biến toàn cục
unsigned long lastBlinkTime = 0;
const long blinkInterval = 500;
bool ledState = false;

// Cấu trúc lưu trạng thái nhiệt độ
struct TempStatus {
  String message;
  int ledPin;
};

void setup() {
  // Khởi tạo I2C
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Khởi tạo LED pins
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  
  // Khởi tạo DHT22
  dht.begin();
  
  // Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }

  display.setRotation(0);
  display.setTextWrap(false);
  display.cp437(true);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(46, 15);
  display.println(F("IOT"));
  display.setCursor(16, 35);
  display.println(F("WELCOME"));
  
  display.display();
  delay(2000);
}

// Hàm xác định trạng thái nhiệt độ
TempStatus getTempStatus(float temp) {
  TempStatus status;
  
  if (temp < 13) {
    status.message = "TOO COLD";
    status.ledPin = LED_GREEN;
  } else if (temp >= 13 && temp < 20) {
    status.message = "COLD";
    status.ledPin = LED_GREEN;
  } else if (temp >= 20 && temp < 25) {
    status.message = "COOL";
    status.ledPin = LED_YELLOW;
  } else if (temp >= 25 && temp < 30) {
    status.message = "WARM";
    status.ledPin = LED_YELLOW;
  } else if (temp >= 30 && temp <= 35) {
    status.message = "HOT";
    status.ledPin = LED_RED;
  } else {
    status.message = "TOO HOT";
    status.ledPin = LED_RED;
  }
  
  return status;
}

// Hàm điều khiển LED nhấp nháy
void controlLED(int activePin) {
  unsigned long currentTime = millis();
  
  if (currentTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentTime;
    ledState = !ledState;
    
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    
    if (ledState) {
      digitalWrite(activePin, HIGH);
    }
  }
}

// Hàm hiển thị lên OLED theo mẫu hình ảnh
void displayData(float temp, float humidity, const String &status) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  // Dòng 1: "Temperature:" (trái) + trạng thái (phải)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Temperature:"));

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(status, 0, 0, &x1, &y1, &w, &h);
  int16_t statusX = (int16_t)SCREEN_WIDTH - (int16_t)w;
  if (statusX < 0) statusX = 0;
  display.setCursor(statusX, 0);
  display.print(status);

  // Dòng 2: Nhiệt độ lớn: "53.7 °C"
  display.setTextSize(2);
  display.setCursor(0, 12);
  char tempStr[12];
  dtostrf(temp, 0, 1, tempStr);
  display.print(tempStr);
  display.print(' ');
  display.write((uint8_t)248); // ° trong CP437
  display.print(F("C"));

  // Dòng 3: "Humidity:"
  display.setTextSize(1);
  display.setCursor(0, 36);
  display.print(F("Humidity:"));

  // Dòng 4: Độ ẩm lớn: "17.50 %" (2 chữ số thập phân)
  display.setTextSize(2);
  display.setCursor(0, 48);
  char humStr[12];
  dtostrf(humidity, 0, 2, humStr);
  display.print(humStr);
  display.print(F(" %"));

  display.display();
}

void loop() {
  // Đọc cảm biến DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Kiểm tra lỗi đọc cảm biến
  if (isnan(humidity) || isnan(temperature)) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.println(F("Sensor Error!"));
    display.display();
    
    digitalWrite(LED_RED, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW);
    delay(900);
    return;
  }
  
  // Lấy trạng thái nhiệt độ
  TempStatus tempStatus = getTempStatus(temperature);
  
  // Hiển thị dữ liệu lên OLED
  displayData(temperature, humidity, tempStatus.message);
  
  // Điều khiển LED nhấp nháy
  controlLED(tempStatus.ledPin);
  
  delay(100);
<<<<<<< HEAD
}
=======
}
>>>>>>> f41be4aa6fe85225993beff762612d64b89070de
