#define BLYNK_TEMPLATE_ID "TMPLxxxxxx" 
#define BLYNK_TEMPLATE_NAME "Group8Control"
#define BLYNK_AUTH_TOKEN "YourAuthToken"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// ===== DHT22 =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_RED 4
#define LED_YELLOW 2
#define LED_BLUE 15

// ===== TM1637 (7-segment) =====
#define CLK 12
#define DIO 13
TM1637Display display(CLK, DIO);

BlynkTimer timer;

float temperature = 0;
float humidity = 0;
bool ledState = false;

// Hàm đọc cảm biến và gửi lên Blynk (2 giây/lần)
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    temperature = t;
    humidity = h;
    
    // Gửi lên các chân ảo (Virtual Pins)
    Blynk.virtualWrite(V1, temperature); 
    Blynk.virtualWrite(V2, humidity);
  }
}

// Hàm xử lý LED và hiển thị TM1637 (500ms/lần)
void updateSystem() {
  ledState = !ledState;
  
  // Tắt tất cả LED trước khi cập nhật
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE, LOW);

  if (ledState) {
    if (temperature < 20) {
      digitalWrite(LED_BLUE, HIGH);
    } else if (temperature < 30) {
      digitalWrite(LED_YELLOW, HIGH);
    } else {
      digitalWrite(LED_RED, HIGH);
    }
  }

  // Hiển thị nhiệt độ lên TM1637 (Ví dụ: 25 C)
  int tempInt = (int)temperature;
  display.showNumberDec(tempInt, false, 2, 0); // Hiển thị số ở 2 vị trí đầu
  
  // Hiển thị chữ C ở cuối (tùy biến theo thư viện)
  uint8_t data[] = { 0x39 }; // Mã hex cho chữ 'C'
  display.setSegments(data, 1, 3);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  dht.begin();
  display.setBrightness(0x0f); // Độ sáng tối đa

  // Kết nối Blynk
  Blynk.begin(auth, ssid, pass);

  // Cài đặt Timer thay cho millis()
  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(500L, updateSystem);
}

void loop() {
  Blynk.run();
  timer.run();
}