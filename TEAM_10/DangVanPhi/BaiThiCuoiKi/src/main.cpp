#include <WiFi.h>
#include "ThingSpeak.h"

// Thông số WiFi giả lập Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Thông tin ThingSpeak từ ảnh bạn chụp
unsigned long myChannelNumber = 3336701; 
const char * myWriteAPIKey = "QDLACM67KC3LCDF1"; 

// Chân cắm cảm biến
const int trigPin = 5;
const int echoPin = 18;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("============================");
  Serial.println("Wokwi terminal ready");
  Serial.println("Baudrate: 115200");
  Serial.println("Bat dau ket noi WiFi...");
  Serial.println("============================");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.print("Dang ket noi WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi da ket noi!");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  ThingSpeak.begin(client);
}

void loop() {
  // Đo khoảng cách bằng siêu âm
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  // Tính % nước (Giả sử bể cao 200cm, đầy ở mức 10cm)
  int level = map(distance, 200, 10, 0, 100);
  level = constrain(level, 0, 100);

  Serial.print("Khoang cach: "); Serial.print(distance); Serial.println(" cm");
  Serial.print("Muc nuoc: "); Serial.print(level); Serial.println(" %");

  // Gửi lên ThingSpeak
  ThingSpeak.setField(1, distance);
  ThingSpeak.setField(2, (float)level);
  
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(x == 200){
    Serial.println("Gui du lieu len ThingSpeak THANH CONG!");
  } else {
    Serial.println("Loi gui du lieu. Ma loi HTTP: " + String(x));
  }

  // Cho 15 giay theo quy dinh cua ThingSpeak
  delay(15000); 
}