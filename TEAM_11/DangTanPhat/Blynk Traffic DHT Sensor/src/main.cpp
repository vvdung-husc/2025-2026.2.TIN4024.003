#define BLYNK_TEMPLATE_ID "TMPL6NqOfu9lC"
#define BLYNK_TEMPLATE_NAME "DHT"
#define BLYNK_AUTH_TOKEN "3HMIgC2Dng_iLoaa5Ef5a62A0X1b-wfQ"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

const int8_t blue_led = 21;
const int8_t button   = 23;

#define DHTPIN  16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define CLK 18
#define DIO 19
TM1637Display tm_display(CLK, DIO);

bool check = true;          // true = bật hệ thống / LED sáng
int counterValue = 0;
int phase = 0;
BlynkTimer timer;

// Biến chống dội nút
int lastButtonReading = HIGH;
int stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

//////////////////////////////////////////////////
// Cập nhật trạng thái LED + display
void updateSystemState() {
  digitalWrite(blue_led, check ? HIGH : LOW);
  tm_display.setBrightness(7, check);

  counterValue = 0;

  if (check) {
    tm_display.showNumberDec(0, false);
  } else {
    tm_display.clear();
  }

  if (Blynk.connected()) {
    Blynk.virtualWrite(V2, check);   // trạng thái nút/app
    Blynk.virtualWrite(V3, 0);       // reset counter trên app
  }
}

//////////////////////////////////////////////////
// HÀM ĐIỀU PHỐI HỆ THỐNG (MỖI 500ms)
void systemScheduler() {
  phase++;

  // Mỗi 1000ms tăng counter 1 lần nếu hệ thống đang bật
  if (phase % 2 == 0) {
    if (check) {
      counterValue++;
      tm_display.showNumberDec(counterValue, false);

      if (Blynk.connected()) {
        Blynk.virtualWrite(V3, counterValue);
      }
    }
  }

  // Mỗi 2 giây đọc cảm biến 1 lần
  if (phase % 4 == 1) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h) && Blynk.connected()) {
      Blynk.virtualWrite(V1, t); // nhiệt độ
      Blynk.virtualWrite(V0, h); // độ ẩm
    }
  }

  if (phase >= 4) phase = 0;
}

//////////////////////////////////////////////////
// Đồng bộ khi Blynk kết nối lại
BLYNK_CONNECTED() {
  Blynk.virtualWrite(V2, check);
  Blynk.virtualWrite(V3, counterValue);
}

//////////////////////////////////////////////////
// Điều khiển từ app Blynk
BLYNK_WRITE(V2) {
  check = param.asInt();
  updateSystemState();
}

//////////////////////////////////////////////////
// Kiểm tra nút nhấn vật lý
void checkPhysicalButton() {
  int reading = digitalRead(button);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // Nhấn nút: do INPUT_PULLUP nên nhấn = LOW
      if (stableButtonState == LOW) {
        check = !check;
        updateSystemState();
      }
    }
  }

  lastButtonReading = reading;
}

//////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  pinMode(blue_led, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  dht.begin();

  tm_display.setBrightness(7);
  tm_display.clear();

  updateSystemState();

  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);

  timer.setInterval(500L, systemScheduler);
  timer.setInterval(20L, checkPhysicalButton);
}

//////////////////////////////////////////////////

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  timer.run();
}