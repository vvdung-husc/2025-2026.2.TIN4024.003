#define BLYNK_TEMPLATE_ID "TMPL6dTZ_kmtH"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "bP7dycWlHq9SsjnXaIy_Ssu2zjeMirdr"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

// Thông tin WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cấu hình chân Pin
#define LED_PIN 21
#define BTN_PIN 23
#define CLK_PIN 18
#define DIO_PIN 19
#define DHTPIN  16
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK_PIN, DIO_PIN);
BlynkTimer timer;

bool ledState = false;
bool running = false;
unsigned long startMs = 0;
int lastBtn = HIGH;

// Hàm gửi dữ liệu cảm biến lên Blynk mỗi 2 giây
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, t); // Nhiệt độ
    Blynk.virtualWrite(V3, h); // Độ ẩm
  }
}

// Hàm hiển thị thời gian lên LED 7 đoạn
void showMMSS(unsigned long sec) {
  int mm = (sec / 60) % 100;
  int ss = sec % 60;
  display.showNumberDecEx(mm * 100 + ss, 0b01000000, true, 4, 0);
  Blynk.virtualWrite(V4, sec); // Gửi giây lên V4
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  display.setBrightness(7, true);
  dht.begin();

  // Kết nối WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  
  // Kết nối Blynk (Dùng IP để ổn định trên Wokwi)
  Blynk.config(BLYNK_AUTH_TOKEN, "128.199.144.129", 80);
  Blynk.connect();

  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();

  // Xử lý nút nhấn vật lý
  int currentBtn = digitalRead(BTN_PIN);
  if (lastBtn == HIGH && currentBtn == LOW) { // Nhấn nút
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V1, ledState); // Đồng bộ lên App
    
    running = ledState;
    if (running) startMs = millis();
    else display.clear();
    
    delay(200); // Chống dội đơn giản
  }
  lastBtn = currentBtn;

  // Cập nhật đồng hồ nếu đang chạy
  if (running) {
    showMMSS((millis() - startMs) / 1000);
  }
}

// Điều khiển từ App Blynk xuống
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
  running = ledState;
  
  if (running) {
    startMs = millis();
  } else {
    display.clear();
    Blynk.virtualWrite(V4, 0);
  }
}