#include <Arduino.h>
#include <TM1637Display.h>

// ===== Pin definitions =====
#define RED_LED     27
#define YELLOW_LED  23
#define GREEN_LED   5

#define TM_CLK      22
#define TM_DIO      21

#define BUTTON_PIN  4

bool displayEnabled = true;   // true = show countdown, false = clear
bool lastButtonState = HIGH;  // for edge detection
// ===== TM1637 display object =====
TM1637Display display(TM_CLK, TM_DIO);

// ===== Function declaration =====
void runLed(uint8_t pin, int durationSeconds);

void handleButton() {
  bool currentState = digitalRead(BUTTON_PIN);

  // Detect button press (HIGH → LOW)
  if (lastButtonState == HIGH && currentState == LOW) {
    displayEnabled = !displayEnabled;

    if (!displayEnabled) {
      Serial.println("Off");
      display.clear();
    }

    Serial.println("On");

    delay(200); // debounce
  }

  lastButtonState = currentState;
}

void setup() {
  // please hold light for more than 1 second
  Serial.begin(115200);
  Serial.println("----------------");
  Serial.println("Started");

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);   // 0–7
  display.clear();
}

void loop() {
  runLed(RED_LED, 7);
  runLed(GREEN_LED, 7);
  runLed(YELLOW_LED, 3);
}

// ===== Function definition =====
void logLED(uint8_t pin) {
  Serial.print("LED: ");
  String res = "";
  switch (pin)
  {
  case RED_LED:
    Serial.println("RED");
    break;
  
  case YELLOW_LED:
    Serial.println("yellow");
    break;
  
  case GREEN_LED:
    Serial.println("green");
    break;
  default:
    break;
  }
}


void runLed(uint8_t pin, int durationSeconds) {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  digitalWrite(pin, HIGH);

  for (int i = durationSeconds; i > 0; i--) {
    handleButton();  // <-- button always responsive

    if (displayEnabled) {
      Serial.print("Countdown: ");
      Serial.println(i);
      display.showNumberDec(i, true);
    } else {
      display.clear();
    }

    delay(1000);
  }

  digitalWrite(pin, LOW);
  display.clear();
}