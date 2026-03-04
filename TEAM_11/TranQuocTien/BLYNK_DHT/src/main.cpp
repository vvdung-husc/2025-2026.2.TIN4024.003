#define BLYNK_TEMPLATE_ID "TMPL6aWxMoVCa"
#define BLYNK_TEMPLATE_NAME "Test"
#define BLYNK_AUTH_TOKEN "oBOhKUTield5M6K6xyzKJMpYKIjpkIYe"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Arduino.h>
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
int i = -1;

void setup_screen() {
  tm_display.setBrightness(7);
  tm_display.clear();
}

void setup_sense() {
  dht.begin();
}

float readTem() {
  return dht.readTemperature();
}

float readHum() {
  return dht.readHumidity();
}

void showScreen(int val) {
  tm_display.showNumberDec(val, false);
}

void turn_on_Off(bool turn) {
  if (turn == true)
    digitalWrite(blue_led, HIGH);
  else
    digitalWrite(blue_led, LOW);
  tm_display.setBrightness(7, turn);
}

BLYNK_WRITE(V2) {
  check = param.asInt(); 
  turn_on_Off(check);
  if (check) i = -1;
}
// --- MODEL SURGERY 1: Ép Web cập nhật theo Mạch ngay khi vừa Online ---
BLYNK_CONNECTED() {
  // Gửi trạng thái thực tế của đèn lên Web để khớp Switch
  Blynk.virtualWrite(V2, check); 
  
  // Gửi giá trị thời gian hiện tại lên để xóa bỏ việc đứng yên ở số 0
  if (check) {
    Blynk.virtualWrite(V3, i + 1);
  } else {
    Blynk.virtualWrite(V3, 0);
  }
  
  // Cập nhật ngay lập tức cảm biến
  Blynk.virtualWrite(V1, readTem());
  Blynk.virtualWrite(V0, readHum());
}

void run(){
  // Khởi tạo (Giữ nguyên của bạn)
  check = true; 
  i = -1;
  showScreen(i+1);
  int lastbutt= 1;
  unsigned long lastConnectAttempt = 0;

  while (true)
  {
    for(int j=0; j< 100; j++){
      if (WiFi.status() == WL_CONNECTED) {
        if (!Blynk.connected()) {
          // --- MODEL SURGERY 2: Thử kết nối sớm hơn ở lần đầu tiên ---
          // Thay vì đợi 10s, lần đầu sẽ thử ngay sau 1s
          unsigned long retryInterval = (lastConnectAttempt == 0) ? 1000 : 10000;
          
          if (millis() - lastConnectAttempt >= retryInterval) {
            lastConnectAttempt = millis();
            Blynk.connect(2000); // Tăng lên 2s để Handshake chắc chắn hơn trên Wokwi
          }
        } else {
          Blynk.run(); 
        }
      }

      int currentbutt = digitalRead(button);
      if(currentbutt == 0 && lastbutt == 1){
        check = !check;
        turn_on_Off(check);
        
        if (Blynk.connected()) {
          Blynk.virtualWrite(V2, check); 
          Blynk.virtualWrite(V3, (check) ? (i + 1) : 0);
        }
        i = -1;
        showScreen(i+1);
      }
      lastbutt = currentbutt;
      delay(10);
    }

    i++;
    showScreen(i+1);
    
    if (Blynk.connected()) {
      // Đẩy V3 liên tục để Web nhảy số theo giây
      Blynk.virtualWrite(V3, (check) ? (i + 1) : 0);
      Blynk.virtualWrite(V1, readTem());
      Blynk.virtualWrite(V0, readHum());
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(blue_led, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  digitalWrite(blue_led, HIGH);

  setup_screen();
  setup_sense();
  
  WiFi.begin(ssid, pass);
  // Ép đích danh server để tăng tốc độ kết nối
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  
  delay(1000);
}

void loop() {
  run();
}