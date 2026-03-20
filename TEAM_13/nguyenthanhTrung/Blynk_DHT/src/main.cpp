#define BLYNK_TEMPLATE_ID "TMPL6vHvLwCwh"
#define BLYNK_TEMPLATE_NAME "BlynkDHT"
#define BLYNK_AUTH_TOKEN "p0LevEusuqqXNeo3Xa1MnvGKrBeYKeOA"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// Thông tin kết nối WiFi giả lập của Wokwi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define DHTPIN 16
#define DHTTYPE DHT22
#define CLK 18
#define DIO 19
#define BUTTON_PIN 23
#define LED_PIN 21

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

bool ledState = LOW;
bool lastButtonState = HIGH;

// Hàm gửi Nhiệt độ & Độ ẩm lên Blynk (V2, V3)
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
  }
}

// Hàm gửi Thời gian hoạt động (V0) và hiển thị TM1637
void updateDisplay() {
  long uptime = millis() / 1000;
  Blynk.virtualWrite(V0, uptime); // Gửi lên ô "Thời gian hoạt động"

  int minutes = (uptime / 60) % 100;
  int seconds = uptime % 60;
  display.showNumberDecEx((minutes * 100) + seconds, 0b01000000, true);
}

// Điều khiển từ Switch trên Blynk (V1)
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  dht.begin();
  display.setBrightness(0x0f);

  // Kết nối vào server Blynk qua WiFi ảo
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, updateDisplay);
}

void loop() {
  Blynk.run();
  timer.run();

  // Nút nhấn vật lý trên Wokwi
  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    delay(50); 
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V1, ledState); // Cập nhật ngược lại Switch trên App
  }
  lastButtonState = currentButtonState;
}