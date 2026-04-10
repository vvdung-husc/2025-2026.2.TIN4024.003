#include <Arduino.h>

static const int LED_RED    = 25;
static const int LED_YELLOW = 26;
static const int LED_GREEN  = 27;

static void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// Nhấp nháy 1 LED trong totalMs (mili giây), với chu kỳ blinkMs
static void blinkFor(int pin, uint32_t totalMs, uint32_t blinkMs) {
  allOff(); // đảm bảo chỉ có 1 màu được phép hoạt động

  uint32_t start = millis();
  bool state = false;

  while (millis() - start < totalMs) {
    // bật/tắt LED đang xét
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);

    // trong lúc chờ blinkMs, vẫn giữ 2 LED còn lại tắt
    // (allOff() sẽ tắt cả, nên phải bật lại pin đang blink)
    delay(blinkMs);
    if (state) {
      // nếu đang bật thì đảm bảo nó vẫn bật (phòng trường hợp bạn muốn mở rộng sau này)
      allOff();
      digitalWrite(pin, HIGH);
    } else {
      allOff();
    }
  }

  // kết thúc pha: tắt LED đó
  allOff();
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  allOff();
}

void loop() {
  // Đỏ nhấp nháy trong 3 giây
  blinkFor(LED_RED, 3000, 250);     // 250ms đổi trạng thái => nhấp nháy rõ

  // Vàng nhấp nháy trong 2 giây
  blinkFor(LED_YELLOW, 2000, 250);

  // Xanh nhấp nháy trong 4 giây
  blinkFor(LED_GREEN, 4000, 250);
}
