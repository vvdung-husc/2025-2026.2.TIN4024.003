// THAY THẾ 3 DÒNG NÀY BẰNG THÔNG TIN TỪ BLYNK CỦA BẠN
#define BLYNK_TEMPLATE_ID "TMPL6qwWFYTYj"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitor"
#define BLYNK_AUTH_TOKEN "EwcdiO4IeM3ULu3l0MMPA--CO9W0JI0-"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi mô phỏng của Wokwi luôn là Wokwi-GUEST và không có mật khẩu
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

// Hàm sinh dữ liệu ảo và gửi lên Blynk
void sendSimulatedData() {
  // Sinh giá trị CO ngẫu nhiên từ 10.0 đến 30.0 ppm
  float simulatedCO = random(10, 30) + (random(0, 100) / 100.0);
  
  // Sinh giá trị bụi PM2.5 ngẫu nhiên từ 15.0 đến 75.0 µg/m3
  float simulatedPM25 = random(15, 75) + (random(0, 100) / 100.0);

  // In ra Serial Monitor để kiểm tra
  Serial.print("Nồng độ CO mô phỏng: "); 
  Serial.print(simulatedCO); 
  Serial.println(" ppm");

  Serial.print("Nồng độ PM2.5 mô phỏng: "); 
  Serial.print(simulatedPM25); 
  Serial.println(" µg/m3");
  Serial.println("-------------------------");

  // Gửi dữ liệu lên Blynk qua các Virtual Pins đã cài đặt
  Blynk.virtualWrite(V0, simulatedCO);
  Blynk.virtualWrite(V1, simulatedPM25);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("Đang kết nối với mạng WiFi mô phỏng Wokwi...");
  
  // Khởi tạo kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Cài đặt Timer gọi hàm sendSimulatedData mỗi 3 giây (3000ms)
  timer.setInterval(3000L, sendSimulatedData);
}

void loop() {
  Blynk.run();
  timer.run();
}