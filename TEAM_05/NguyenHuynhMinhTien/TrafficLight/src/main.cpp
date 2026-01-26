#include <Arduino.h>  // Thư viện chính của Arduino cho ESP32, cung cấp các hàm như pinMode, digitalWrite, millis(), Serial, v.v.
#include <TM1637Display.h>  // Thư viện để điều khiển màn hình 7 đoạn TM1637, cho phép hiển thị số và ký tự

// Định nghĩa các chân GPIO (General Purpose Input/Output) của ESP32 được sử dụng
#define PIN_LED_RED 25      // Chân kết nối với LED đỏ
#define PIN_LED_YELLOW 33   // Chân kết nối với LED vàng
#define PIN_LED_GREEN 32    // Chân kết nối với LED xanh
#define CLK 15              // Chân clock cho TM1637 display
#define DIO 2               // Chân data cho TM1637 display
#define PIN_BUTTON_DISPLAY 23  // Chân kết nối với nút nhấn để bật/tắt display
#define PIN_LED_BLUE 21     // Chân kết nối với LED xanh dương (cho trạng thái display)

// Khởi tạo đối tượng display cho TM1637
TM1637Display display(CLK, DIO);  // Đối tượng để điều khiển màn hình 7 đoạn
bool displayOn = false;  // Biến boolean để theo dõi trạng thái bật/tắt của display (true = bật, false = tắt)

// Finite State Machine (FSM) cho đèn giao thông - Máy trạng thái hữu hạn
enum State { GREEN, YELLOW, RED };  // Định nghĩa các trạng thái: xanh, vàng, đỏ
State currentState = GREEN;  // Trạng thái hiện tại, bắt đầu với xanh
const uint32_t times[] = {7000, 3000, 5000};  // Thời gian cho mỗi trạng thái: xanh 7s, vàng 3s, đỏ 5s (tính bằng ms)
unsigned long stateStart = 0;  // Thời điểm bắt đầu trạng thái hiện tại (tính bằng millis())
int remainingSec = 0;  // Số giây còn lại của trạng thái hiện tại

// Hàm kiểm tra thời gian (non-blocking timer) - Kiểm tra xem đã đủ thời gian chưa mà không chặn chương trình
bool IsReady(unsigned long &timer, uint32_t ms) {
  if (millis() - timer < ms) return false;  // Nếu chưa đủ thời gian, trả về false
  timer = millis();  // Cập nhật timer với thời gian hiện tại
  return true;  // Đã sẵn sàng
}

// Hàm trả về chuỗi mô tả trạng thái hiện tại để debug
const char* StateString() {
  switch (currentState) {
    case GREEN: return "GREEN";    // Trả về "GREEN" nếu trạng thái xanh
    case YELLOW: return "YELLOW";  // Trả về "YELLOW" nếu trạng thái vàng
    case RED: return "RED";        // Trả về "RED" nếu trạng thái đỏ
    default: return "UNKNOWN";     // Trường hợp không xác định
  }
}

// Hàm cập nhật trạng thái các LED dựa trên trạng thái hiện tại
void UpdateLEDs() {
  digitalWrite(PIN_LED_GREEN, currentState == GREEN ? HIGH : LOW);    // Bật LED xanh nếu trạng thái GREEN, ngược lại tắt
  digitalWrite(PIN_LED_YELLOW, currentState == YELLOW ? HIGH : LOW);  // Bật LED vàng nếu trạng thái YELLOW
  digitalWrite(PIN_LED_RED, currentState == RED ? HIGH : LOW);        // Bật LED đỏ nếu trạng thái RED
}

// Hàm setup() - Chạy một lần khi ESP32 khởi động, dùng để cấu hình ban đầu
void setup() {
  Serial.begin(115200);  // Khởi tạo giao tiếp Serial với tốc độ 115200 baud để debug qua USB
  Serial.println("*** OPTIMIZED LED TRAFFIC FSM ***");  // In thông báo khởi động
  
  // Cấu hình các chân LED là OUTPUT (đầu ra)
  pinMode(PIN_LED_RED, OUTPUT); pinMode(PIN_LED_YELLOW, OUTPUT); pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUTTON_DISPLAY, INPUT_PULLUP);  // Cấu hình chân nút là INPUT với PULLUP (điện trở kéo lên)
  pinMode(PIN_LED_BLUE, OUTPUT);  // Chân LED xanh dương là OUTPUT
  
  display.setBrightness(0x0a);  // Đặt độ sáng cho TM1637 display (0x0a = 10/15)
  display.clear();  // Xóa màn hình display
  stateStart = millis();  // Ghi lại thời điểm bắt đầu trạng thái đầu tiên
}

// Hàm loop() - Chạy liên tục sau setup(), chứa logic chính của chương trình
void loop() {
  static unsigned long timer500 = 0, timer10 = 0;  // Biến static để lưu thời gian cho các timer (không bị reset mỗi loop)
  
  // Phần FSM (Finite State Machine) cho đèn giao thông - Kiểm tra mỗi 500ms
  if (IsReady(timer500, 500)) {  // Nếu đã đủ 500ms
    unsigned long elapsed = millis() - stateStart;  // Tính thời gian đã trôi qua từ khi bắt đầu trạng thái hiện tại
    remainingSec = (times[currentState] - elapsed) / 1000;  // Tính số giây còn lại (chia cho 1000 để chuyển ms sang s)
    
    if (elapsed >= times[currentState]) {  // Nếu đã hết thời gian của trạng thái hiện tại
      currentState = static_cast<State>((currentState + 1) % 3);  // Chuyển sang trạng thái tiếp theo (GREEN -> YELLOW -> RED -> GREEN)
      stateStart = millis();  // Reset thời điểm bắt đầu cho trạng thái mới
      Serial.printf("Switch to %s (%d giây)\n", StateString(), times[currentState]/1000);  // In debug thông tin chuyển trạng thái
    } else if (displayOn) {  // Nếu display đang bật
      display.showNumberDec(remainingSec > 0 ? remainingSec : 0);  // Hiển thị số giây còn lại trên TM1637 (không âm)
      Serial.printf("[%s] %d giây\n", StateString(), remainingSec);  // In debug thời gian còn lại
    }
    
    // Tính năng nhấp nháy LED trong 2 giây cuối (tùy chọn)
    bool blink = (elapsed > times[currentState] - 2000);  // Kiểm tra nếu còn dưới 2s
    digitalWrite( (currentState == GREEN ? PIN_LED_GREEN : 
                   currentState == YELLOW ? PIN_LED_YELLOW : PIN_LED_RED), blink ? HIGH : LOW);  // Nhấp nháy LED tương ứng
  }
  
  // Phần xử lý nút nhấn - Kiểm tra mỗi 10ms để debounce
  if (IsReady(timer10, 10)) {  // Nếu đã đủ 10ms
    bool pressed = !digitalRead(PIN_BUTTON_DISPLAY);  // Đọc trạng thái nút (đảo logic vì PULLUP: LOW = nhấn, HIGH = không nhấn)
    if (pressed != displayOn) {  // Nếu trạng thái nút thay đổi
      displayOn = pressed;  // Cập nhật trạng thái display
      digitalWrite(PIN_LED_BLUE, displayOn ? HIGH : LOW);  // Bật/tắt LED xanh dương theo trạng thái display
      Serial.printf("*** DISPLAY %s ***\n", displayOn ? "ON" : "OFF");  // In debug trạng thái display
      if (!displayOn) display.clear();  // Nếu tắt display, xóa màn hình
    }
  }
}
