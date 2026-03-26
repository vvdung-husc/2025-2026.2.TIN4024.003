#define BLYNK_TEMPLATE_ID "TMPL6Tzz3JSOx"
#define BLYNK_TEMPLATE_NAME "BLYNKDHT"
#define BLYNK_AUTH_TOKEN "F7v9ANFxqfsXHR6uSBbjMWIXlcoE2ye5"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// Cấu hình chân Pin
#define PIN_BUTTON 23
#define PIN_LED    21
#define PIN_DHT    16
#define PIN_TM_DIO 19
#define PIN_TM_CLK 18

#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);
TM1637Display display(PIN_TM_CLK, PIN_TM_DIO);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

// Biến trạng thái
bool isRunning = false; // Trạng thái hệ thống (Bật/Tắt)
int secondsCount = 0;   // Biến đếm giây
bool blinkState = false; // Trạng thái nhấp nháy đèn

// Hàm xử lý việc đếm giây và nhấp nháy đèn mỗi 1 giây
void updateSystem() {
  if (isRunning) {
    secondsCount++; // Tăng giây
    blinkState = !blinkState; // Đảo trạng thái đèn để nhấp nháy
    
    digitalWrite(PIN_LED, blinkState ? HIGH : LOW);
    
    // Hiển thị lên màn hình TM1637
    display.showNumberDec(secondsCount, false);
    
    // Gửi số giây lên Blynk (V3 - Thời gian hoạt động)
    Blynk.virtualWrite(V3, secondsCount);
  } else {
    // Nếu tắt: tắt đèn và xóa màn hình
    digitalWrite(PIN_LED, LOW);
    display.clear();
    secondsCount = 0; // Reset lại bộ đếm nếu muốn (hoặc bỏ dòng này nếu muốn giữ số cũ)
  }
}

// Hàm gửi dữ liệu Nhiệt độ/Độ ẩm (chạy độc lập mỗi 2 giây)
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  }
}

// Đồng bộ nút nhấn từ App Blynk (V0)
BLYNK_WRITE(V0) {
  isRunning = param.asInt();
  if (!isRunning) {
    digitalWrite(PIN_LED, LOW);
    display.clear();
    Blynk.virtualWrite(V3, 0); // Reset số trên App khi tắt
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  dht.begin();
  display.setBrightness(0x0f);
  display.clear();
  
  Blynk.begin(auth, ssid, pass);

  // Hẹn giờ cập nhật hệ thống (đếm giây và nháy đèn) mỗi 1 giây
  timer.setInterval(1000L, updateSystem);
  
  // Hẹn giờ gửi dữ liệu cảm biến mỗi 2 giây
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();

  // Kiểm tra nút nhấn vật lý
  if (digitalRead(PIN_BUTTON) == LOW) {
    delay(200); // Chống dội nút
    isRunning = !isRunning;
    Blynk.virtualWrite(V0, isRunning); // Cập nhật trạng thái nút trên App
    
    if (!isRunning) {
      digitalWrite(PIN_LED, LOW);
      display.clear();
    }
  }
}