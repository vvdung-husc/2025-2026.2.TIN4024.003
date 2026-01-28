//C1:
// #include <Arduino.h>

// uint8_t LED_RED = 25;
// uint8_t LED_YELLOW = 33;
// uint8_t LED_GREEN = 32;

// int red_duration = 5000;    // 5s
// int yellow_duration = 3000; // 3s
// int green_duration = 7000;  // 7s

// void blinkLed(uint8_t pin, int duration) {
//   int timeLeft = duration;

//   while (timeLeft > 0) {
//     digitalWrite(pin, HIGH);
//     delay(500);   // bật 0.5s
//     digitalWrite(pin, LOW);
//     delay(500);   // tắt 0.5s
//     timeLeft -= 1000; // đã trôi qua 1s
//   }
// }

// void setup() {
//   pinMode(LED_RED, OUTPUT);
//   pinMode(LED_YELLOW, OUTPUT);
//   pinMode(LED_GREEN, OUTPUT);
//   Serial.begin(115200);
//   Serial.println("Traffic Blink Start!");
// }

// void loop() {
//   blinkLed(LED_RED, red_duration);    
//   blinkLed(LED_YELLOW, yellow_duration); 
//   blinkLed(LED_GREEN, green_duration); 
// }

//C2
#include <Arduino.h>

uint8_t LED_RED = 25;
uint8_t LED_YELLOW = 33;
uint8_t LED_GREEN = 32;

// thời gian mỗi đèn (ms)
unsigned long red_duration = 5000;
unsigned long yellow_duration = 3000;
unsigned long green_duration = 7000;

// thời gian nhấp nháy
unsigned long blinkInterval = 200;

// trạng thái đèn hiện tại
enum LightState { RED, YELLOW, GREEN };
LightState currentState = RED;

// thời gian bắt đầu của mỗi pha đèn
unsigned long stateStartTime = 0;

// thời gian nhấp nháy
unsigned long lastBlinkTime = 0;
bool ledStatus = false;

void turnOffAll() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Traffic Blink Start!");
}

void loop() {
  unsigned long now = millis();

  // Xác định LED và thời gian của trạng thái hiện tại
  unsigned long currentDuration;
  uint8_t currentLed;

  if (currentState == RED) {
    currentDuration = red_duration;
    currentLed = LED_RED;
  } 
  else if (currentState == YELLOW) {
    currentDuration = yellow_duration;
    currentLed = LED_YELLOW;
  } 
  else { // GREEN
    currentDuration = green_duration;
    currentLed = LED_GREEN;
  }

  // Nhấp nháy LED
  if (now - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = now;
    ledStatus = !ledStatus; // đảo trạng thái
    turnOffAll();
    digitalWrite(currentLed, ledStatus ? HIGH : LOW);
  }

  //Nếu hết thời gian → chuyển sang đèn tiếp theo
  if (now - stateStartTime >= currentDuration) {
    stateStartTime = now;
    ledStatus = false; // reset nhấp nháy
    turnOffAll();

    if (currentState == RED) currentState = YELLOW;
    else if (currentState == YELLOW) currentState = GREEN;
    else currentState = RED;
  }
}


