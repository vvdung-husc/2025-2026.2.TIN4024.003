#include "main.h"

//Định nghĩa chân cho đèn LED
#define PIN_LED_RED     18
#define PIN_LED_YELLOW  5
#define PIN_LED_GREEN    17

//Định nghĩa cho LDR (Light Dependent Resistor)
#define PIN_LDR 34 // Analog ADC1 GPIOGPIO34 connected to LDR

int DAY_ADC_THRESHOLD = 2000; // Ngưỡng ánh sáng ban ngày

// These constants should match the photoresistor's "gamma" and "rl10" attributes
// const float GAMMA = 0.7; // typical gamma value for LDRs
// const float RL10 = 33;  // 50 resistance at 10 lux in kOhms
// const float LDR_VCC = 3.3; //5.0 Voltage supplied to the LDR voltage divider
// float luxValue = 0.0;

// const char* LEDString(uint8_t pin)
// {
//   switch (pin)
//   {
//     case PIN_LED_RED:     return "RED";
//     case PIN_LED_YELLOW:  return "YELLOW";
//     case PIN_LED_GREEN:   return "GREEN";
//     default:              return "UNKNOWN";
//   }  
// }

// int LDR_Read(int pin, int loopValue = 10){
//   static int buffer[50];      // tối đa 50 mẫu
//   static int index = 0;
//   static long sum = 0;
//   static bool filled = false;

//   int value = analogRead(pin);

//   // trừ giá trị cũ
//   sum -= buffer[index];

//   // ghi giá trị mới
//   buffer[index] = value;
//   sum += value;

//   index++;
//   if (index >= loopValue) {
//     index = 0;
//     filled = true;
//   }

//   // nếu chưa đủ mẫu thì chia theo số mẫu hiện có
//   int divisor = filled ? loopValue : index;
//   return sum / divisor;
// }

//LED_Blink ledYellow;
Trafic_Blink traficLight;
LDR ldrSensor;

void setup() {
  // put your setup code here, to run once:
  printf("Welcome IoT\n");
  //pinMode(PIN_LED_RED, OUTPUT);
  //pinMode(PIN_LED_YELLOW, OUTPUT);
  //pinMode(PIN_LED_GREEN, OUTPUT); 

  //pinMode(PIN_LDR, INPUT);

  //ledYellow.setup(PIN_LED_YELLOW);

  ldrSensor.setup(PIN_LDR, false); // VCC = 3.3V

  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traficLight.setupWaitTime(5, 3, 7); // seconds
}

// void RED_Blink(){
//   unsigned long ulTimer = 0;
//   static bool bLEDStatus = false;
//   if (IsReady(ulTimer, 1000)){
//     bLEDStatus = !bLEDStatus;
//     digitalWrite(PIN_LED_RED, bLEDStatus); 
//   }
// }

// void ProcessLDR(){
//   static unsigned long ulTimer = 0;
//   if (!IsReady(ulTimer, 500)) return;
//   int analogValue =  analogRead(PIN_LDR);
//   float voltage = (float)analogValue * LDR_VCC / 4095.0 ;
//   float resistance = 2000 * voltage / (1 - voltage / LDR_VCC);
//   float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  
//   if (lux != luxValue){
//     luxValue = lux;
//     if (luxValue >= 1.0)
//       printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.0f] lux\n", analogValue, voltage, resistance, lux, luxValue);
//     else
//       printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.1f] lux\n", analogValue, voltage, resistance, lux, luxValue);
//   }
// }

/*
void TrafficBlink(){
static unsigned long ulTimer = 0;
  static uint8_t idxLED = 0;//PIN_LED_GREEN
  static uint8_t LEDs[3] = {PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED};
  static uint32_t waitTime[3] = {7000, 3000, 5000};// Green, Yellow, Red
  static uint32_t count = waitTime[idxLED];
  static bool ledStatus = false;
  static int secondCount = 0;

  if (!IsReady(ulTimer, 500)) return;

  if (count == waitTime[idxLED])
  {
    secondCount = (count / 1000) - 1;

    ledStatus = true;
    for (size_t i = 0; i < 3; i++)
    {
      if (i == idxLED){
        digitalWrite(LEDs[i], HIGH);
        printf("LED [%-6s] ON => %d Seconds\n", LEDString(LEDs[i]), count/1000);
      }
      else digitalWrite(LEDs[i], LOW);
    }    
  }
  else {
    ledStatus = !ledStatus;
    digitalWrite(LEDs[idxLED], ledStatus ? HIGH : LOW);
  }

  if (ledStatus) {
    printf(" [%s] => seconds: %d \n",LEDString(LEDs[idxLED]), secondCount);  
    --secondCount;
  }

  count -= 500;
  if (count > 0) return;

  idxLED = (idxLED + 1) % 3;// Next LED => idxLED = 0,1,2,...
  count = waitTime[idxLED];

}
*/
void loop() {
  //ledYellow.blink(500);
  int analogValue = 0;
  float lux =ldrSensor.readLux(&analogValue);
  bool isDark = (analogValue > DAY_ADC_THRESHOLD);
  traficLight.blink(500, isDark);
  //RED_Blink();
  //ProcessLDR();
  //TrafficBlink();
}

// void loop() {
//   // put your main code here, to run repeatedly:
//   printf("[LED_RED] => HIGH\n");
//   digitalWrite(PIN_LED_RED, HIGH); // Turn LED ON
//   delay(500); // Wait for 500 milliseconds
//   printf("[LED_RED] => LOW\n");
//   digitalWrite(PIN_LED_RED, LOW); // Turn LED OFF
//   delay(500); // Wait for 500 milliseconds  
// }


