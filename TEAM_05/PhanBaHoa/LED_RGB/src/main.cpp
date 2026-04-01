/*
 * Dự án: Hệ thống đèn LED RGB thông minh (ESP32 + WS2812)
 * Tác giả: Phan Bá Hóa
 * Tính năng: Nhấp nháy, mỗi LED 1 màu, đổi màu sau mỗi 10 giây
 */

#include <Adafruit_NeoPixel.h>

#define PIN        5    
#define NUMPIXELS  5    

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// --- Biến quản lý thời gian ---
unsigned long lastChangeTime = 0; 
const unsigned long intervalChange = 10000; // 10 giây đổi màu một lần

unsigned long lastBlinkTime = 0;
const unsigned long intervalBlink = 500;    // 0.5 giây chớp tắt một lần

bool ledState = true; // Trạng thái Đang sáng (true) hoặc Đang tắt (false)

// Mảng lưu trữ giá trị màu cho từng LED
byte ledR[NUMPIXELS];
byte ledG[NUMPIXELS];
byte ledB[NUMPIXELS];

int brightness = 200; 

// Hàm tạo màu ngẫu nhiên mới
void generateRandomColors() {
  Serial.println(">>> Dang doi bo mau moi...");
  for(int i = 0; i < NUMPIXELS; i++) {
    ledR[i] = random(0, 256);
    ledG[i] = random(0, 256);
    ledB[i] = random(0, 256);
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0)); 

  pixels.begin();
  pixels.setBrightness(brightness);
  
  generateRandomColors(); // Tạo màu lần đầu
}

void loop() {
  unsigned long currentTime = millis();

  // 1. KIỂM TRA ĐỔI MÀU (Mỗi 10 giây)
  if (currentTime - lastChangeTime >= intervalChange) {
    generateRandomColors();
    lastChangeTime = currentTime;
  }

  // 2. KIỂM TRA NHẤP NHÁY (Mỗi 0.5 giây)
  if (currentTime - lastBlinkTime >= intervalBlink) {
    ledState = !ledState; // Đảo trạng thái: Sáng -> Tắt hoặc Tắt -> Sáng
    lastBlinkTime = currentTime;
  }

  // 3. HIỂN THỊ DỰA TRÊN TRẠNG THÁI ledState
  if (ledState == true) {
    // Nếu trạng thái là Sáng: Hiện màu trong mảng
    for(int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(ledR[i], ledG[i], ledB[i]));
    }
  } else {
    // Nếu trạng thái là Tắt: Xóa sạch màu (Tắt đèn)
    pixels.clear(); 
  }
  
  pixels.show();
}