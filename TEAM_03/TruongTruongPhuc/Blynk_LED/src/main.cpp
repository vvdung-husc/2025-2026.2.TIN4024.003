#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6ASlZf_Q8"
#define BLYNK_TEMPLATE_NAME "esp32"
#define BLYNK_AUTH_TOKEN "JHW_9VvBIJo9tzDs8vqdlDZoZYUFWaSb"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

#define PIN_LED_RED  23 //Chân kết đèn LED RED

// Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}
void uptimeBlynk();

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);  //Đảm bảo đèn LED RED tắt khi khởi động
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi và Blynk
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();  //Chạy Blynk để cập nhật trạng thái từ Blynk Cloud

  uptimeBlynk(); //Gọi hàm cập nhật thời gian hoạt động lên Blynk
}

void uptimeBlynk(){
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  unsigned long value = lastTime / 1000;
  printf("Uptime: %lu seconds\n", value); //In thời gian hoạt động ra Serial Monitor
  Blynk.virtualWrite(V0, value);  //Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk.
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