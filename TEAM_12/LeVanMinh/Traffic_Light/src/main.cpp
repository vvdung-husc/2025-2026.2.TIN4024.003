#include <Arduino.h>
#include <TM1637Display.h>

// --- Định nghĩa các chân (Pin Definitions) ---
#define CLK 18
#define DIO 19
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_BLUE 21     // Đèn báo người đi bộ
#define BTN_PIN 4       // Nút nhấn
#define SENSOR_PIN 13   // Chân Analog (Potentiometer/Cảm biến)

// --- Cấu hình hiển thị ---
TM1637Display display(CLK, DIO);

// --- Các biến toàn cục ---
bool pedestrianRequest = false; // Trạng thái yêu cầu qua đường

void setup() {
  Serial.begin(115200);

  // Cấu hình chân LED là OUTPUT
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // Cấu hình nút nhấn (INPUT_PULLUP: không nhấn = HIGH, nhấn = LOW)
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Cấu hình màn hình
  display.setBrightness(0x0f); // Độ sáng tối đa
  display.clear();
  
  Serial.println("He thong den giao thong khoi dong...");
}

// Hàm kiểm tra nút nhấn (trả về true nếu nút được nhấn)
bool checkButton() {
  if (digitalRead(BTN_PIN) == LOW) { // Nút nhấn kích hoạt mức thấp
    digitalWrite(LED_BLUE, HIGH);    // Bật đèn xanh dương báo hiệu đã nhận lệnh
    pedestrianRequest = true;
    Serial.println("-> Nguoi di bo bam nut!");
    return true;
  }
  return false;
}

// Hàm đếm ngược và hiển thị lên 7-segment
// return true nếu bị ngắt bởi nút nhấn (chỉ áp dụng cho đèn xanh)
bool countdown(int seconds, bool checkBtn) {
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i, true); // Hiển thị số giây
    
    // Chia nhỏ delay 1 giây thành 10 lần 100ms để nút nhấn nhạy hơn
    for (int j = 0; j < 10; j++) {
      delay(100); 
      if (checkBtn && checkButton()) {
        return true; // Thoát sớm nếu có nút nhấn khi đang đèn xanh
      }
    }
  }
  return false;
}

void loop() {
  // 1. Đọc giá trị cảm biến để chỉnh thời gian đèn xanh
  // Giá trị analog từ 0-4095, map sang thời gian từ 5 đến 15 giây
  int sensorValue = analogRead(SENSOR_PIN);
  int greenTime = map(sensorValue, 0, 4095, 5, 15); 
  
  Serial.print("Thoi gian den xanh: ");
  Serial.print(greenTime);
  Serial.println(" giay (Dieu chinh boi bien tro)");

  // --- TRẠNG THÁI: ĐÈN XANH ---
  Serial.println("RED: OFF | YELLOW: OFF | GREEN: ON");
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  
  // Chạy đếm ngược đèn xanh, kiểm tra nút nhấn
  bool interrupted = countdown(greenTime, true);

  // Nếu có người đi bộ bấm nút, đèn xanh tắt ngay, chuyển sang vàng nhanh hơn
  if (interrupted) {
    pedestrianRequest = false; // Reset cờ sau khi xử lý
    delay(500); // Chờ 1 chút cho mượt
  }

  // --- TRẠNG THÁI: ĐÈN VÀNG ---
  Serial.println("RED: OFF | YELLOW: ON | GREEN: OFF");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  countdown(3, false); // Đèn vàng luôn là 3 giây, không check nút

  // --- TRẠNG THÁI: ĐÈN ĐỎ ---
  Serial.println("RED: ON | YELLOW: OFF | GREEN: OFF");
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  
  // Tắt đèn báo người đi bộ (nếu đang bật) vì giờ xe dừng, người đi bộ đi
  digitalWrite(LED_BLUE, LOW); 
  
  countdown(5, false); // Đèn đỏ 5 giây
}