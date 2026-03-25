#define BLYNK_TEMPLATE_ID "TMPL6MClQgeDD"
#define BLYNK_TEMPLATE_NAME "ProjectByPhuocMinh"
#define BLYNK_AUTH_TOKEN "IVfPIDJFw3MqACR5QCMIN-Tu2ZlEMoK3"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- ĐÃ SỬA LẠI CHÂN KẾT NỐI CHO KHỚP VỚI DIAGRAM.JSON ---
#define DHTPIN 16       // Chân Data của DHT22 (Theo file JSON là 16)
#define DHTTYPE DHT22   // Loại cảm biến
#define CLK 18          // Chân CLK của màn hình TM1637 (Theo file JSON là 18)
#define DIO 19          // Chân DIO của màn hình TM1637 (Theo file JSON là 19)
#define LED_PIN 21      // Đèn LED (Theo file JSON là 21)

// Khởi tạo các đối tượng
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// Thông tin WiFi (Dành cho giả lập Wokwi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

int operatingTime = 0; // Biến lưu thời gian hoạt động (giây)

// Hàm 1: Xử lý đếm thời gian (Chạy mỗi 1 giây)
void updateTimer() {
  operatingTime++;
  
  // Gửi số giây lên Blynk (chân V3)
  Blynk.virtualWrite(V3, operatingTime); 
  
  // Hiển thị ra màn hình TM1637
  display.showNumberDec(operatingTime, false); 
}

// Hàm 2: Đọc DHT22 và gửi dữ liệu (Chạy mỗi 2 giây)
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi đọc cảm biến DHT22!");
    return;
  }

  Blynk.virtualWrite(V1, t); // V1: Nhiệt độ
  Blynk.virtualWrite(V2, (int)h); // V2: Độ ẩm
  
  Serial.print("Nhiệt độ: "); Serial.print(t);
  Serial.println(" °C");
}

// Hàm 3: Lắng nghe lệnh Bật/Tắt đèn từ App (V0)
BLYNK_WRITE(V0) {
  int pinValue = param.asInt(); // Lấy giá trị từ nút gạt trên App (0 hoặc 1)
  digitalWrite(LED_PIN, pinValue);

  Serial.print("Blynk gửi lệnh Bật/Tắt đèn: ");
  Serial.println(pinValue);
}

void setup() {
  Serial.begin(115200);
  
  // Cấu hình phần cứng
  pinMode(LED_PIN, OUTPUT);
  display.setBrightness(0x0f); // Độ sáng tối đa
  display.clear();

  // THÊM: Chờ 2 giây để cảm biến DHT22 ổn định nguồn
  delay(2000);
  dht.begin();

  // Kết nối với Blynk
  Serial.println("Đang kết nối WiFi và Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Đặt lịch chạy cho các hàm
  timer.setInterval(1000L, updateTimer);     // Cứ 1 giây gọi hàm updateTimer
  timer.setInterval(2000L, sendSensorData);  // Cứ 2 giây gọi hàm sendSensorData
}

void loop() {
  Blynk.run();
  timer.run();
}