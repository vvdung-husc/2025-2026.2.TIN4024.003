#define BLYNK_TEMPLATE_ID "TMPL6BfEzxW9D"
#define BLYNK_TEMPLATE_NAME "Flood Warning"
#define BLYNK_AUTH_TOKEN "U-q0r9GfLhqgi5zZfprrJis7ZBkVkAId"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Thông tin WiFi ảo của Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN =====
#define SENSOR_PIN 32
#define LED_GREEN   5
#define LED_YELLOW 18
#define LED_RED     2

// ===== NGƯỠNG =====
#define LEVEL_SAFE  1200
#define LEVEL_WARN  2500

unsigned long previousMillis = 0;
bool ledState = false;
int lastState = -1;
int waterLevel = 0;

BlynkTimer timer;

// ===== PHÂN LOẠI TRẠNG THÁI =====
int getState(int value) {
  if (value < LEVEL_SAFE) return 0; // An toàn
  else if (value < LEVEL_WARN) return 1; // Cảnh báo
  else return 2; // Nguy hiểm
}

// ===== GỬI DỮ LIỆU LÊN BLYNK =====
void sendToBlynk() {
  if (Blynk.connected()) { // Chỉ gửi khi đã kết nối
    int percent = map(waterLevel, 0, 4095, 0, 100);
    int state = getState(waterLevel);

    // Gửi phần trăm mực nước lên Widget V0
    Blynk.virtualWrite(V0, percent);

    // Cập nhật đèn LED ảo trên App (V1, V7, V2)
    Blynk.virtualWrite(V1, (state == 0) ? 255 : 0);
    Blynk.virtualWrite(V7, (state == 1) ? 255 : 0);
    Blynk.virtualWrite(V2, (state == 2) ? 255 : 0);

    // Gửi dòng trạng thái lên Widget V3
    if (state == 0) Blynk.virtualWrite(V3, "🟢 An toàn");
    else if (state == 1) Blynk.virtualWrite(V3, "🟡 Cảnh báo");
    else Blynk.virtualWrite(V3, "🔴 NGUY HIỂM!");

    // 🔥 GỬI THÔNG BÁO (LOG EVENT)
    // Chỉ gửi khi mực nước bắt đầu chuyển sang mức Nguy hiểm (state = 2)
    if (state == 2 && lastState != 2) {
      Serial.println("!!! Dang gui thong bao den dien thoai !!!");
      Blynk.logEvent("flood_alert", "Cảnh báo: Mực nước vượt ngưỡng nguy hiểm!");
    }
    
    lastState = state;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Serial.println("Dang ket noi Blynk...");
  
  // Kết nối Blynk (Wokwi sẽ tự giả lập WiFi)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Đợi một chút cho kết nối ổn định rồi mới gửi thông báo khởi động
  timer.setTimeout(5000L, []() {
    if (Blynk.connected()) {
      Blynk.logEvent("flood_alert", "Hệ thống giám sát đã Online!");
      Serial.println("Da gui thong bao khoi dong.");
    }
  });

  // Thiết lập gửi dữ liệu định kỳ mỗi 2 giây
  timer.setInterval(2000L, sendToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();

  // Đọc giá trị cảm biến
  waterLevel = analogRead(SENSOR_PIN);
  int state = getState(waterLevel);
  unsigned long currentMillis = millis();

  // Logic nhấp nháy LED vật lý
  if (currentMillis - previousMillis >= 400) {
    previousMillis = currentMillis;
    ledState = !ledState;
  }

  // Tắt hết LED trước khi cập nhật
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // Bật LED tương ứng với trạng thái
  if (state == 0) digitalWrite(LED_GREEN, ledState);
  else if (state == 1) digitalWrite(LED_YELLOW, ledState);
  else if (state == 2) digitalWrite(LED_RED, ledState);
}