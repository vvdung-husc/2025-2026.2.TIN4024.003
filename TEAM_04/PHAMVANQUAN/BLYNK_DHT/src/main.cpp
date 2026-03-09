#define BLYNK_TEMPLATE_ID "TMPL6XWrYxWXZ"
#define BLYNK_TEMPLATE_NAME "ESP32 DHT"
#define BLYNK_AUTH_TOKEN "wVRepimK-oqZePwd70K-a37huvNmCDBl"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";    

const int8_t blue_led = 21;
const int8_t button = 23;

#define DHTPIN  16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define CLK 18
#define DIO 19
TM1637Display tm_display(CLK, DIO);

bool check = true;
int counterValue = 0;
int phase = 0; // Biến điều phối pha
BlynkTimer timer;

// --- HÀM ĐIỀU PHỐI HỆ THỐNG (MỖI 500MS) ---
void systemScheduler() {
  phase++;

  // PHA 1 (Sau mỗi 1000ms): Cập nhật đồng hồ
  if (phase % 2 == 0) {
    if (check) {
      counterValue++;
      tm_display.showNumberDec(counterValue, false);
      if (Blynk.connected()) {
        Blynk.virtualWrite(V3, counterValue);
      }
    }
  }

  // PHA 2 (Lệch 500ms so với Pha 1): Cập nhật Cảm biến
  // Cứ mỗi 2 giây (4 phase) sẽ đọc cảm biến một lần tại thời điểm "rảnh"
  if (phase % 4 == 1) { 
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if (!isnan(t) && !isnan(h) && Blynk.connected()) {
      Blynk.virtualWrite(V1, t);
      Blynk.virtualWrite(V0, h);
    }
  }

  if (phase >= 4) phase = 0; // Reset vòng lặp pha
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V2, check);
  Blynk.virtualWrite(V3, check ? counterValue : 0);
}

BLYNK_WRITE(V2) {
  check = param.asInt();
  digitalWrite(blue_led, check ? HIGH : LOW);
  tm_display.setBrightness(7, check);
  if (check) {
    counterValue = 0;
    tm_display.showNumberDec(0);
  } else {
    tm_display.clear();
    counterValue = 0;
  }
  if (Blynk.connected()) Blynk.virtualWrite(V3, 0);
}

void setup() {
  Serial.begin(9600);
  pinMode(blue_led, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  digitalWrite(blue_led, HIGH);

  tm_display.setBrightness(7);
  tm_display.clear();
  dht.begin();
  
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);

  // Nhịp tim hệ thống: 500ms
  timer.setInterval(500L, systemScheduler); 
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  timer.run();

  // Nút nhấn vật lý
  static int lastButt = 1;
  int currentButt = digitalRead(button);
  if (currentButt == 0 && lastButt == 1) {
    check = !check;
    digitalWrite(blue_led, check ? HIGH : LOW);
    tm_display.setBrightness(7, check);
    
    counterValue = 0;
    if (check) tm_display.showNumberDec(0);
    else tm_display.clear();

    if (Blynk.connected()) {
      Blynk.virtualWrite(V2, check);
      Blynk.virtualWrite(V3, 0);
    }
    delay(10); 
  }
  lastButt = currentButt;
}