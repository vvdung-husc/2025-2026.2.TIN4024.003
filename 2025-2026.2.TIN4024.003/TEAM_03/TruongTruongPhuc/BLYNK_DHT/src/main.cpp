#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6jO8gjgZP"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "It8AOO-TEuEm0l8mW31LBXU7HrcLk7cI"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>            //Thư viện đọc cảm biến DHT

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

#define PIN_LED_RED  21 //Chân kết đèn LED BLUE (nối GPIO21 theo diagram)
#define PIN_DHT      16 //Chân DATA của DHT22 nối vào GPIO16
#define DHT_TYPE     DHT22

DHT dht(PIN_DHT, DHT_TYPE); //Khởi tạo đối tượng DHT
BlynkTimer timer;            //Bộ hẹn giờ Blynk

void connectWiFi() {
  Serial.println("Dang ket noi WiFi...");
  WiFi.begin(ssid, pass);
  
  // Cấu hình DNS thủ công để tránh lỗi DNS failed
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8));
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi da ket noi!");
    Serial.print("Dia chi IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nLoi: Khong the ket noi WiFi!");
  }
}

void sendToBlynk() {
  // Uptime
  unsigned long uptime = millis() / 1000;
  Serial.printf("Uptime: %lu seconds\n", uptime);
  Blynk.virtualWrite(V0, uptime);

  // Nhiệt độ
  float nhietDo = dht.readTemperature();
  float doAm    = dht.readHumidity();

  if (isnan(nhietDo) || isnan(doAm)) {
    Serial.println("Loi doc cam bien DHT22!");
    return;
  }
  Serial.printf("Nhiet do: %.1f C - Do am: %.1f %%\n", nhietDo, doAm);
  Blynk.virtualWrite(V2, nhietDo); //Gửi nhiệt độ lên V2
  Blynk.virtualWrite(V3, doAm);    //Gửi độ ẩm lên V3
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);
  dht.begin();
  
  // Kết nối WiFi
  connectWiFi();
  
  // Cấu hình DNS servers sau khi đã kết nối WiFi
  IPAddress dns1(8, 8, 8, 8);       // Google DNS
  IPAddress dns2(8, 8, 4, 4);       // Google DNS backup
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns1, dns2);

  // Kết nối Blynk
  Serial.println("Dang ket noi Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  
  int attempts = 0;
  while (!Blynk.connected() && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (Blynk.connected()) {
    Serial.println("\nBlynk da ket noi!");
  } else {
    Serial.println("\nLoi: Khong the ket noi Blynk!");
  }

  timer.setInterval(5000L, sendToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}

//được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V1) { //virtual_pin định nghĩa trong ứng dụng Blynk
  // Xử lý dữ liệu nhận được từ ứng dụng Blynk
  int value = param.asInt();  // Lấy giá trị từ ứng dụng Blynk
  if (value == 1){
    printf("Blynk -> RED Light ON");
    digitalWrite(PIN_LED_RED, HIGH);
    
  }
  else {
    printf("Blynk -> RED Light OFF");
    digitalWrite(PIN_LED_RED, LOW);   
  }
}