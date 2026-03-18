/*
THÔNG TIN NHÓM 11
1. Trần Quốc Tiến
*/

#define BLYNK_TEMPLATE_ID "TMPL6c0cqfIJ6"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "9gbOkBxTHIn7Iu-tFhjPAKEZDYZjzquO"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// --- Cấu hình chân phần cứng ---
#define DHTPIN 4
#define DHTTYPE DHT22
#define LED_PIN 23
#define GAS_PIN 34
#define BTN_PIN 22 

// --- KHAI BÁO NGUYÊN MẪU HÀM (Prototypes) ---
// Bước này giúp compiler biết sự tồn tại của hàm trước khi chúng được gọi
void updateUptime();
void updateSensors();
void checkButton();

// --- Khởi tạo đối tượng ---
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool ledStatus = LOW;
bool lastBtnState = HIGH;

// --- ĐỒNG BỘ KHI VỪA KẾT NỐI ---
BLYNK_CONNECTED() {
  // Đồng bộ trạng thái nút nhấn V0 từ Web xuống ESP32 ngay lập tức
  Blynk.syncVirtual(V0); 
  
  // Ép hệ thống gửi dữ liệu ngay lần đầu để Web không bị "đơ"
  updateUptime();
  updateSensors();
  
  Serial.println(">>> System Synchronized with Blynk Cloud <<<");
}

// 1. Cập nhật Uptime (V4) mỗi 1 giây
void updateUptime() {
  Blynk.virtualWrite(V4, millis() / 1000);
}

// 2. Cập nhật cảm biến (V1, V2, V3) mỗi 2 giây
void updateSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gas = analogRead(GAS_PIN);

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V1, t); 
    Blynk.virtualWrite(V2, h); 
  }
  Blynk.virtualWrite(V3, gas); 

  Serial.printf("T: %.1fC | H: %.1f%% | Gas: %d\n", t, h, gas);
  Serial.println("--- Information provided by Team X ---");
}

// 3. Xử lý nút bấm vật lý (GPIO 22) đồng bộ với Switch trên Web (V0)
void checkButton() {
  bool currentBtnState = digitalRead(BTN_PIN);
  if (currentBtnState == LOW && lastBtnState == HIGH) {
    delay(50); // Chống nhiễu
    if (digitalRead(BTN_PIN) == LOW) {
      ledStatus = !ledStatus;
      digitalWrite(LED_PIN, ledStatus);
      
      // Đồng bộ ngược lại Web: Đổi trạng thái Switch V0
      Blynk.virtualWrite(V0, ledStatus);
      
      Serial.print("Physical Button -> LED: ");
      Serial.println(ledStatus ? "ON" : "OFF");
    }
  }
  lastBtnState = currentBtnState;
}

// 4. Nhận lệnh điều khiển từ Web (V0)
BLYNK_WRITE(V0) {
  ledStatus = param.asInt();
  digitalWrite(LED_PIN, ledStatus);
  Serial.print("Web/App Command -> LED: ");
  Serial.println(ledStatus ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP); 
  pinMode(GAS_PIN, INPUT);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");

  // Thiết lập Timer
  timer.setInterval(1000L, updateUptime);  // V4 nhảy mỗi 1s
  timer.setInterval(2000L, updateSensors); // V1, V2, V3 nhảy mỗi 2s
}

void loop() {
  Blynk.run();
  timer.run();
  checkButton(); 
}