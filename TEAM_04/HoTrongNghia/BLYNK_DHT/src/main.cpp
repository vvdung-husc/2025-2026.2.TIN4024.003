// --- CẤU HÌNH BLYNK (BẮT BUỘC ĐẶT Ở DÒNG ĐẦU TIÊN) ---
#define BLYNK_TEMPLATE_ID "TMPL6VDR3fWoz"
#define BLYNK_TEMPLATE_NAME "Blynk DHT"
#define BLYNK_AUTH_TOKEN "HgVeqSpp3RqKaX34T-LawCpcwzNXna_z"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- CẤU HÌNH WIFI (Dành riêng cho Wokwi) ---
char ssid[] = "Wokwi-GUEST";
char pass[] = ""; // Mật khẩu để trống

// --- CẤU HÌNH PHẦN CỨNG ---
#define BUTTON_PIN 23   // Nút nhấn
#define LED_PIN    21   // Đèn LED
#define DHTPIN     16   // Cảm biến DHT22
#define DHTTYPE    DHT22
#define CLK_PIN    18   // Chân CLK của màn hình 4 số TM1637
#define DIO_PIN    19   // Chân DIO của màn hình 4 số TM1637

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK_PIN, DIO_PIN);
BlynkTimer timer;

// --- BIẾN TRẠNG THÁI ---
int ledState = LOW;
int lastButtonState = HIGH; 

// ============================================================
// 1. NHẬN LỆNH TỪ APP BLYNK (Khi nhấn nút V0 trên điện thoại)
// ============================================================
BLYNK_WRITE(V0) {
  ledState = param.asInt();         
  digitalWrite(LED_PIN, ledState);  
}

// ============================================================
// 2. GỬI TÊN LÊN BLYNK KHI VỪA KẾT NỐI THÀNH CÔNG
// ============================================================
BLYNK_CONNECTED() {
  // BẠN SỬA DÒNG CHỮ TRONG NGOẶC KÉP DƯỚI ĐÂY THÀNH TÊN CỦA BẠN NHÉ
  Blynk.virtualWrite(V4, "Hồ Trọng Nghĩa"); 
}

// ============================================================
// 3. ĐỌC CẢM BIẾN & ĐẨY LÊN BLYNK (Chạy mỗi 2 giây)
// ============================================================
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi: Không thể đọc dữ liệu từ DHT22!");
    return;
  }

  // --- Gửi dữ liệu lên Blynk.cloud ---
  Blynk.virtualWrite(V1, t); // V1: Nhiệt độ hiển thị trên Digit Display
  Blynk.virtualWrite(V2, t); // V2: Nhiệt độ 
  Blynk.virtualWrite(V3, h); // V3: Độ ẩm

  // --- Hiển thị lên màn hình 4 số TM1637 ---
  display.showNumberDec((int)t, false, 2, 0); 
  
  // In ra Serial để kiểm tra
  Serial.print("Nhiệt độ: "); Serial.print(t); Serial.print("°C\t");
  Serial.print("Độ ẩm: "); Serial.print(h); Serial.println("%");
}

void setup() {
  Serial.begin(115200);

  // --- Thiết lập chân phần cứng ---
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // --- Khởi tạo cảm biến và màn hình ---
  dht.begin();
  display.setBrightness(0x0f); 
  display.clear();

  // --- Kết nối Blynk ---
  Serial.println("Đang kết nối WiFi và Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // --- Cài đặt Timer ---
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run(); 
  timer.run(); 

  // ============================================================
  // 4. XỬ LÝ NÚT NHẤN VẬT LÝ
  // ============================================================
  int buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState == LOW && lastButtonState == HIGH) {
    ledState = !ledState;               
    digitalWrite(LED_PIN, ledState);    
    Blynk.virtualWrite(V0, ledState);   
    
    delay(50); 
  }
  
  lastButtonState = buttonState; 
}