#include <Arduino.h>

// // uint8_t LED_RED = 17;

// // bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
// //   if (millis() - ulTimer < millisecond) return false;
// //   ulTimer = millis();
// //   return true;
// // }

// // void setup() {

// //   printf("Welcome IoT\n");
// //   pinMode(LED_RED, OUTPUT); 
// // }

// // void loop() {
 
// //   printf("[LED_RED] => HIGH\n");
// //   digitalWrite(LED_RED, HIGH); 
// //   delay(500); 
// //   printf("[LED_RED] => LOW\n");
// //   digitalWrite(LED_RED, LOW); 
// //   delay(500); 
// // }

// // Định nghĩa các chân cắm (Pins)
// const int RED_LED = 25;
// const int YELLOW_LED = 33;
// const int GREEN_LED = 32;

// void setup() {
//   // Khởi tạo Serial để theo dõi trên Terminal
//   Serial.begin(115200);

//   // Thiết lập các chân là đầu ra (OUTPUT)
//   pinMode(RED_LED, OUTPUT);
//   pinMode(YELLOW_LED, OUTPUT);
//   pinMode(GREEN_LED, OUTPUT);
// }

// void loop() {
//   // 1. ĐÈN ĐỎ BẬT
//   digitalWrite(RED_LED, HIGH);
//   Serial.println("LED [RED   ] ON => 5 Seconds");
//   delay(5000); // Đợi 5 giây
//   digitalWrite(RED_LED, LOW);

//   // 2. ĐÈN VÀNG BẬT
//   digitalWrite(YELLOW_LED, HIGH);
//   Serial.println("LED [YELLOW] ON => 3 Seconds");
//   delay(3000); // Đợi 3 giây
//   digitalWrite(YELLOW_LED, LOW);

//   // 3. ĐÈN XANH BẬT
//   digitalWrite(GREEN_LED, HIGH);
//   Serial.println("LED [GREEN ] ON => 7 Seconds");
//   delay(7000); // Đợi 7 giây
//   digitalWrite(GREEN_LED, LOW);
// }





// ====== CHÂN KẾT NỐI ======
#define LED_RED     18
#define LED_YELLOW  5
#define LED_GREEN   17
#define LDR_PIN     34

// ====== NGƯỠNG PHÂN BIỆT SÁNG / TỐI ======
int lightThreshold = 2000;  // THẤP = sáng, CAO = tối

// ====== KHAI BÁO HÀM ======
void blinkLED(int ledPin, const char* name);
void nightMode();

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.begin(115200);
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  Serial.print("Light Value: ");
  Serial.println(lightValue);

  // ☀️ TRỜI SÁNG (giá trị thấp)
  if (lightValue < lightThreshold) {
    Serial.println("=== DAY MODE (Traffic Running) ===");

    blinkLED(LED_GREEN, "GREEN");
    blinkLED(LED_YELLOW, "YELLOW");
    blinkLED(LED_RED, "RED");
  }
  // 🌙 TRỜI TỐI (giá trị cao)
  else {
    nightMode();
  }
}

// ================= NHẤP NHÁY 1 ĐÈN TRONG 5 GIÂY =================
void blinkLED(int ledPin, const char* name) {
  Serial.print("Blinking LED: ");
  Serial.println(name);

  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {  // 5 giây
    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin, LOW);
    delay(300);
  }
}

// ================= BAN ĐÊM =================
void nightMode() {
  Serial.println("=== NIGHT MODE (Yellow Warning) ===");

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  digitalWrite(LED_YELLOW, HIGH);
  delay(500);
  digitalWrite(LED_YELLOW, LOW);
  delay(500);
}
