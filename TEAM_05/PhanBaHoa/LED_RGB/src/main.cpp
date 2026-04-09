/*
 * Dự án: Điều khiển LED RGB qua Blynk Web
 * Tác giả: Phan Bá Hóa
 */

// --- 3 DÒNG NÀY PHẢI Ở TRÊN CÙNG ---
#define BLYNK_TEMPLATE_ID "TMPL6pK8E6c7E"
#define BLYNK_TEMPLATE_NAME "LED RGB SMART"
#define BLYNK_AUTH_TOKEN "O_0jfBD80GMVLDCr_N_ELnwNaKHvCjtK"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_NeoPixel.h>

// --- Cấu hình WiFi cho mô phỏng Wokwi ---
char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";            

// --- Cấu hình LED ---
#define PIN        5    
#define NUMPIXELS  5    
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Biến lưu trạng thái hiện tại (Mặc định sáng màu trắng)
int r = 255, g = 255, b = 255;
int currentBrightness = 255;

// --- HÀM CẬP NHẬT ĐÈN (Phải đặt ở đây để các hàm bên dưới gọi được) ---
void updateLEDs() {
  for(int i=0; i<NUMPIXELS; i++) {
    // Tính toán lại màu dựa trên % độ sáng
    pixels.setPixelColor(i, pixels.Color(r * currentBrightness / 255, g * currentBrightness / 255, b * currentBrightness / 255));
  }
  pixels.show(); // Hiển thị màu sắc mới trên LED
}

// --- BLYNK: Nhận màu từ color picker (Virtual pin V0) ---
BLYNK_WRITE(V0) {
  String color = param.asStr(); // Lấy giá trị màu từ color picker
  long number = strtol(&color[1], NULL, 16); // Chuyển đổi từ hex string sang số nguyên
  r = (number >> 16) & 0xFF; // Lấy giá trị đỏ
  g = (number >> 8) & 0xFF;  // Lấy giá trị xanh lá
  b = number & 0xFF;         // Lấy giá trị xanh dương
  updateLEDs(); // Cập nhật màu sắc cho LED
}

// --- BLYNK: Nhận giá trị độ sáng từ slider (Virtual pin V1) ---
BLYNK_WRITE(V1) {
  currentBrightness = param.asInt(); // Lấy giá trị độ sáng từ slider
  updateLEDs(); // Cập nhật màu sắc cho LED
}

void setup() {
  Serial.begin(115200);
  pixels.begin(); // Khởi tạo thư viện NeoPixel
  updateLEDs();   // Cho đèn sáng ngay lúc vừa khởi động
  
  Serial.println("Dang ket noi den Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// --- HÀM LOOP BẮT BUỘC PHẢI CÓ ĐỂ DUY TRÌ BLYNK ---
void loop() {
  Blynk.run();
}