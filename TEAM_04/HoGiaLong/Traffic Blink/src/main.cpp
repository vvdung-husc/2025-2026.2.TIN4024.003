#include <Arduino.h>
// 1. Định nghĩa các chân (Theo sơ đồ JSON bạn gửi)
const int LED_XANH = 32;
const int LED_VANG = 33;
const int LED_DO   = 25;

// 2. Biến quản lý thời gian và trạng thái
unsigned long thoiGianTruoc = 0;
const long khoangThoiGian = 1000; // 1 giây đổi đèn 1 lần
int trangThaiDen = 0; // 0: Đỏ, 1: Xanh, 2: Vàng

void setup() {
  // Cấu hình chân đèn là Output
  pinMode(LED_XANH, OUTPUT);
  pinMode(LED_VANG, OUTPUT);
  pinMode(LED_DO, OUTPUT);

  // Mở Serial để kiểm tra (nếu cần)
  Serial.begin(115200);
}

void loop() {
  // Lấy thời gian hiện tại
  unsigned long thoiGianHienTai = millis();

  // Kiểm tra: Đã đủ 1 giây chưa?
  if (thoiGianHienTai - thoiGianTruoc >= khoangThoiGian) {
    
    // Lưu lại mốc thời gian vừa thực hiện
    thoiGianTruoc = thoiGianHienTai;

    // Tắt hết tất cả đèn trước (để đảm bảo chỉ 1 đèn sáng)
    digitalWrite(LED_XANH, LOW);
    digitalWrite(LED_VANG, LOW);
    digitalWrite(LED_DO, LOW);

    // Kiểm tra trạng thái và bật đèn tương ứng
    if (trangThaiDen == 0) {
      digitalWrite(LED_DO, HIGH);  // Bật đèn Đỏ
      Serial.println("Đèn Đỏ sáng");
      trangThaiDen = 1;            // Chuyển trạng thái kế tiếp là Xanh
    } 
    else if (trangThaiDen == 1) {
      digitalWrite(LED_XANH, HIGH); // Bật đèn Xanh
      Serial.println("Đèn Xanh sáng");
      trangThaiDen = 2;             // Chuyển trạng thái kế tiếp là Vàng
    } 
    else if (trangThaiDen == 2) {
      digitalWrite(LED_VANG, HIGH); // Bật đèn Vàng
      Serial.println("Đèn Vàng sáng");
      trangThaiDen = 0;             // Quay về trạng thái Đỏ
    }
  }
}