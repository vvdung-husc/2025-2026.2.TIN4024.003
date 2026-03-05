#define BLYNK_TEMPLATE_ID "TMPL6AJWRTv7y"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic DHT sensor"
#define BLYNK_AUTH_TOKEN "EZPKS73v1Rey8uiWh1M34pFWEIMu4Thz"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cấu hình chân theo diagram.json
#define DHTPIN 16
#define DHTTYPE DHT22
#define CLK 18
#define DIO 19
#define LED_PIN 21
#define BTN_PIN 23

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

int uptimeSeconds = 0;
bool counterActive = true; // Trạng thái đếm (nút nhấn điều khiển)
bool lastButtonState = HIGH;

// Hàm đọc cảm biến và gửi lên Blynk
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V1, t); // Nhiệt độ
    Blynk.virtualWrite(V2, h); // Độ ẩm
    
    // Hiển thị nhiệt độ lên TM1637
    display.showNumberDec((int)t, false, 2, 0); 
    uint8_t deg[] = { 0x39 }; // Ký tự 'C'
    display.setSegments(deg, 1, 3);
  }
}

// Hàm đếm thời gian và điều khiển LED
void countTime() {
  if (counterActive) {
    uptimeSeconds++;
    Blynk.virtualWrite(V0, uptimeSeconds);
    
    // Hiệu ứng LED chớp nhẹ khi đang đếm (tùy chọn) hoặc bật sáng
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    Blynk.virtualWrite(V3, digitalRead(LED_PIN) ? 255 : 0);
  } else {
    digitalWrite(LED_PIN, LOW);
    Blynk.virtualWrite(V3, 0);
  }
}

// Kiểm tra nút nhấn vật lý để bật/tắt đếm
void checkButton() {
  bool currentState = digitalRead(BTN_PIN);
  if (currentState == LOW && lastButtonState == HIGH) { // Nhấn nút (Active Low)
    counterActive = !counterActive;
    Serial.println(counterActive ? "Counter: ON" : "Counter: OFF");
    delay(50); // Debounce nhẹ
  }
  lastButtonState = currentState;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  dht.begin();
  display.setBrightness(0x0f);
  
  // Kết nối Blynk
  Blynk.begin(auth, ssid, pass);
  
  // Thiết lập DNS để tránh lỗi DNS Failed trên Wokwi
  //IPAddress dns(8, 8, 8, 8);
  //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);

  // Khởi tạo Timer
  timer.setInterval(2000L, sendSensorData); // Đọc cảm biến mỗi 2 giây
  timer.setInterval(1000L, countTime);      // Đếm giây mỗi 1 giây
}

void loop() {
  Blynk.run();
  timer.run();
  checkButton(); // Kiểm tra nút nhấn liên tục
}

// Cho phép bật/tắt đếm từ App Blynk qua Switch V4
BLYNK_WRITE(V4) {
  counterActive = param.asInt();
}