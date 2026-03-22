#define BLYNK_TEMPLATE_ID "TMPL664Sbulzw"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "huA6jxv4NNRwmpLpiErHsN1QRw6GOSBV"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- CẤU HÌNH WI-FI CHO WOKWI ---
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// --- KHAI BÁO CÁC CHÂN GPIO (Từ file JSON của bạn) ---
#define DHTPIN 16
#define DHTTYPE DHT22

#define CLK_PIN 18
#define DIO_PIN 19

#define LED_PIN 21
#define BTN_PIN 23

// --- KHỞI TẠO CÁC ĐỐI TƯỢNG ---
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK_PIN, DIO_PIN);
BlynkTimer timer;

// --- BIẾN TOÀN CỤC ---
bool isSystemActive = false; // Trạng thái hệ thống (Bật/Tắt)
int operatingTime = 0;       // Thời gian hoạt động (giây)

// --- HÀM ĐỌC CẢM BIẾN VÀ GỬI LÊN BLYNK ---
// Giả sử: Nhiệt độ -> Chân Ảo V0 | Độ ẩm -> Chân Ảo V1 | Thời gian -> Chân Ảo V3
// Thêm 2 biến toàn cục để lưu lại giá trị cũ (đặt ở trên cùng hoặc ngay trước hàm này)
float lastT = -999.0;
float lastH = -999.0;

// --- HÀM ĐỌC CẢM BIẾN VÀ CHỈ GỬI KHI CÓ SỰ THAY ĐỔI ---
void sendSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Kiểm tra lỗi nếu cảm biến hỏng/mất kết nối
  if (isnan(t) || isnan(h)) {
    Serial.println("Lỗi: Không thể đọc dữ liệu từ DHT22!");
    return;
  }

  // So sánh giá trị mới đo (t, h) với giá trị cũ (lastT, lastH)
  if (t != lastT || h != lastH) {
    // Nếu có sự thay đổi, tiến hành gửi lên Blynk
    Blynk.virtualWrite(V0, t);
    Blynk.virtualWrite(V1, h);
    
    // In ra Serial Monitor để theo dõi
    Serial.print("CẬP NHẬT MỚI -> Nhiệt độ: "); Serial.print(t);
    Serial.print(" *C | Độ ẩm: "); Serial.print(h); Serial.println(" %");

    // Cập nhật lại "trí nhớ" cho lần kiểm tra tiếp theo
    lastT = t;
    lastH = h;
  }
}
// --- HÀM XỬ LÝ ĐẾM THỜI GIAN ---
void handleTimer() {
  if (isSystemActive) {
    operatingTime++; // Tăng thêm 1 giây
    
    // Hiển thị lên màn hình LED 7 đoạn
    display.showNumberDec(operatingTime, false);
    
    // Gửi thời gian lên App Blynk (Chân Ảo V3)
    Blynk.virtualWrite(V3, operatingTime);
  }
}

// --- HÀM NHẬN LỆNH TỪ NÚT GẠT TRÊN BLYNK (Chân Ảo V2) ---
BLYNK_WRITE(V2) {
  int pinValue = param.asInt(); // Đọc giá trị từ nút gạt (0 hoặc 1)
  
  if (pinValue == 1) {
    isSystemActive = true;
    digitalWrite(LED_PIN, HIGH); // Bật đèn LED xanh
    Serial.println("Hệ thống: BẬT");
  } else {
    isSystemActive = false;
    operatingTime = 0;           // Reset thời gian về 0 khi tắt
    digitalWrite(LED_PIN, LOW);  // Tắt đèn LED xanh
    display.showNumberDec(0, false); // Trả màn hình về 0
    Blynk.virtualWrite(V3, 0);   // Cập nhật lên App
    Serial.println("Hệ thống: TẮT");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Cấu hình chân phần cứng
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // Khởi tạo màn hình và cảm biến
  display.setBrightness(0x0f);
  display.showNumberDec(0, false);
  dht.begin();

  // --- BẮT ĐẦU PHẦN SỬA LỖI DNS ---
  Serial.print("Đang kết nối Wi-Fi...");
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối Wi-Fi thành công!");

  // Ép ESP32 sử dụng DNS của Google (8.8.8.8) để tránh lỗi mạng Wokwi
  IPAddress googleDNS(8, 8, 8, 8);
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), googleDNS);

  // Kết nối vào máy chủ Blynk
  Serial.println("Đang kết nối với Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  // --- KẾT THÚC PHẦN SỬA LỖI DNS ---

  // Cài đặt Timer gọi hàm định kỳ
  timer.setInterval(2000L, sendSensorData); 
  timer.setInterval(1000L, handleTimer);    
}
void loop() {
  Blynk.run();
  timer.run();

  // --- XỬ LÝ NÚT NHẤN VẬT LÝ TRÊN WOKWI ---
  // Biến tĩnh để nhớ trạng thái nút của vòng lặp trước
  static bool lastButtonState = HIGH; 
  bool currentButtonState = digitalRead(BTN_PIN); // Đọc trạng thái nút hiện tại

  // Nếu phát hiện nút vừa được nhấn xuống (chuyển từ HIGH sang LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    delay(50); // Chờ 50ms để chống rung phím cơ học (debounce)
    
    if (digitalRead(BTN_PIN) == LOW) { // Kiểm tra lại chắc chắn nút đang bị đè
      isSystemActive = !isSystemActive; // Đảo trạng thái (Đang Bật thành Tắt, Đang Tắt thành Bật)

      if (isSystemActive) {
        // Hành động khi BẬT
        digitalWrite(LED_PIN, HIGH);
        Blynk.virtualWrite(V2, 1); // Đồng bộ: Tự động gạt nút trên Web Blynk sang BẬT
        Serial.println("Hệ thống: BẬT (lệnh từ nút nhấn Wokwi)");
      } else {
        // Hành động khi TẮT
        operatingTime = 0;
        digitalWrite(LED_PIN, LOW);
        display.showNumberDec(0, false);
        Blynk.virtualWrite(V3, 0); // Reset thời gian đếm trên web
        Blynk.virtualWrite(V2, 0); // Đồng bộ: Tự động gạt nút trên Web Blynk sang TẮT
        Serial.println("Hệ thống: TẮT (lệnh từ nút nhấn Wokwi)");
      }
    }
  }
  
  lastButtonState = currentButtonState; // Cập nhật lại trạng thái cho vòng lặp sau
}