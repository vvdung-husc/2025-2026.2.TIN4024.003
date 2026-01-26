#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân cắm (Khớp 100% với sơ đồ của bạn)
#define PIN_CLK 18
#define PIN_DIO 19
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_BLUE_PED 22
#define BTN_PEDESTRIAN 23
#define LDR_SENSOR 13

TM1637Display display(PIN_CLK, PIN_DIO);

bool pedRequest = false; 

void setup() {
  // Khởi tạo Serial với tốc độ 115200 baud
  Serial.begin(115200);
  delay(500); // Chờ một chút để Terminal kịp kết nối
  
  Serial.println("========================================");
  Serial.println("HE THONG DEN GIAO THONG DANG KHOI DONG");
  Serial.println("========================================");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE_PED, OUTPUT);
  pinMode(BTN_PEDESTRIAN, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  display.clear();

  Serial.println("[OK] Cac thiet bi da san sang.");
}

// Hàm đếm ngược có thông báo ra Terminal từng giây
void runCountdownWithLog(int seconds, String stateName, bool canInterrupt) {
  Serial.println(">>> Bat dau pha: " + stateName + " (" + String(seconds) + "s)");
  
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i, true);
    
    // In thông tin nhịp tim hệ thống ra Terminal mỗi giây
    Serial.print("[" + stateName + "] Con lai: ");
    Serial.print(i);
    Serial.println(" giay...");

    // Kiểm tra nút nhấn người đi bộ
    for (int j = 0; j < 10; j++) {
      if (digitalRead(BTN_PEDESTRIAN) == LOW) {
        if (!pedRequest) {
          pedRequest = true;
          Serial.println("   [!] PHAT HIEN: Co nguoi nhan nut uu tien qua duong!");
          digitalWrite(LED_BLUE_PED, HIGH); // Sáng đèn báo hiệu nhận lệnh
        }
      }
      delay(100);
    }

    // Nếu đang đèn xanh và có yêu cầu, rút ngắn thời gian
    if (canInterrupt && pedRequest && i > 3) {
      Serial.println("   [!] UU TIEN: Rut ngan thoi gian xanh de nhuong duong.");
      i = 3; 
    }
  }
}

void loop() {
  // 1. Đọc cảm biến ánh sáng LDR
  int ldrValue = analogRead(LDR_SENSOR);
  Serial.println("----------------------------------------");
  Serial.print("[SENSOR] Cuong do anh sang hien tai: ");
  Serial.println(ldrValue);
  if (ldrValue > 2500) Serial.println("[!] Thong bao: Troi dang toi.");
  else Serial.println("[!] Thong bao: Troi dang sang.");

  // 2. PHA ĐÈN XANH
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  runCountdownWithLog(10, "DEN XANH", true);

  // 3. PHA ĐÈN VÀNG
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  runCountdownWithLog(3, "DEN VANG", false);

  // 4. PHA ĐÈN ĐỎ
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  
  // Xử lý đèn xanh dương cho người đi bộ
  if (pedRequest) {
    Serial.println("[ACTION] Cho phep nguoi di bo sang duong.");
    digitalWrite(LED_BLUE_PED, HIGH);
  } else {
    digitalWrite(LED_BLUE_PED, LOW);
  }
  
  runCountdownWithLog(12, "DEN DO", false);

  // Reset trạng thái sau chu kỳ
  pedRequest = false;
  digitalWrite(LED_BLUE_PED, LOW);
  Serial.println("[DONE] Hoan thanh 1 chu ky. Bat dau lai...");
}