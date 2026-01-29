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





#include <Arduino.h>

#define LED_RED     18
#define LED_YELLOW  5
#define LED_GREEN   17
#define LDR_PIN     34

int lightThreshold = 2000;   // ngưỡng sáng/tối

void blinkLED(int ledPin, const char* name);
void nightMode();

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int rawValue = analogRead(LDR_PIN);

  // 🔥 ĐẢO GIÁ TRỊ (để thấp = sáng, cao = tối)
  int lightValue = 4095 - rawValue;

  Serial.print("Light Value (Adjusted): ");
  Serial.println(lightValue);

  // ☀️ GIÁ TRỊ THẤP = SÁNG
  if (lightValue < lightThreshold) {
    Serial.println("=== DAY MODE ===");
    blinkLED(LED_GREEN, "GREEN");
    blinkLED(LED_YELLOW, "YELLOW");
    blinkLED(LED_RED, "RED");
  }
  // 🌙 GIÁ TRỊ CAO = TỐI
  else {
    nightMode();
  }
}

// ====== NHẤP NHÁY 5 GIÂY ======
void blinkLED(int ledPin, const char* name) {
  Serial.print("Blinking LED: ");
  Serial.println(name);

  unsigned long start = millis();
  while (millis() - start < 5000) {
    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin, LOW);
    delay(300);
  }
}

// ====== CHẾ ĐỘ TỐI ======
void nightMode() {
  Serial.println("=== NIGHT MODE (Yellow only) ===");

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  digitalWrite(LED_YELLOW, HIGH);
  delay(500);
  digitalWrite(LED_YELLOW, LOW);
  delay(500);
}

