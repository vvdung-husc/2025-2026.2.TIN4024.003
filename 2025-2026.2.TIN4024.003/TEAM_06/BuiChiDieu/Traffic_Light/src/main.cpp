#include <Arduino.h>
#include <TM1637Display.h>

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

const uint8_t RedLedPin = 27;
const uint8_t YellowLedPin = 26;
const uint8_t GreenLedPin = 25;
const uint8_t BlueLedPin = 21;
const uint8_t ButtonPin = 23;
const uint8_t CLK_DigitDisplayPin = 18;
const uint8_t DIO_DigitDisplayPin = 19;
const uint8_t AO_PhotoresistorSensor = 22;

const unsigned long RED_TIME = 10000 - 1;	// 10 giây (tu 0 -> 9)
const unsigned long YELLOW_TIME = 3000 - 1; // 3 giây (tu 0 -> 2)
const unsigned long GREEN_TIME = 5000 - 1;	// 5 giây  (tu 0 -> 4)
unsigned long blinkPreviousMillis = 0;
const long BLINK_INTERVAL = 500; // Chớp tắt mỗi 500ms
unsigned long interval = RED_TIME;

unsigned long previousMillis = 0;

int trafficState = 0;

bool blinkRed = false;
bool blinkYellow = false;
bool blinkGreen = false;

void setup()
{
	Serial.begin(115200);

	pinMode(RedLedPin, OUTPUT);
	pinMode(YellowLedPin, OUTPUT);
	pinMode(GreenLedPin, OUTPUT);

	pinMode(ButtonPin, INPUT_PULLUP);
	pinMode(AO_PhotoresistorSensor, INPUT);

	display.setBrightness(0x0f);
	digitalWrite(RedLedPin, HIGH);
}

void loop()
{
	unsigned long currentMillis = millis();

	if (trafficState == 0 && currentMillis - previousMillis >= RED_TIME)
	{
		previousMillis = millis();

		trafficState = (trafficState + 1) % 3;

		digitalWrite(RedLedPin, LOW);
		digitalWrite(YellowLedPin, HIGH);
		digitalWrite(GreenLedPin, LOW);

		interval = YELLOW_TIME;
	}
	else if (trafficState == 1 && currentMillis - previousMillis >= YELLOW_TIME)
	{
		previousMillis = millis();

		trafficState = (trafficState + 1) % 3;

		digitalWrite(RedLedPin, LOW);
		digitalWrite(YellowLedPin, LOW);
		digitalWrite(GreenLedPin, HIGH);

		interval = GREEN_TIME;
	}
	else if (trafficState == 2 && currentMillis - previousMillis >= GREEN_TIME)
	{
		previousMillis = millis();

		trafficState = (trafficState + 1) % 3;

		digitalWrite(RedLedPin, HIGH);
		digitalWrite(YellowLedPin, LOW);
		digitalWrite(GreenLedPin, LOW);

		interval = RED_TIME;
	}

	int remainingSeconds = (interval - (currentMillis - previousMillis));



	if (currentMillis - blinkPreviousMillis >= BLINK_INTERVAL)
	{
		if (trafficState == 0)
		{
			blinkRed = !blinkRed;
			digitalWrite(RedLedPin, static_cast<uint8_t>(blinkRed));
			
		}
		else if (trafficState == 1)
		{
			blinkYellow = !blinkYellow;
			digitalWrite(YellowLedPin, static_cast<uint8_t>(blinkYellow));
		}
		else if (trafficState == 2)
		{
			blinkGreen = !blinkGreen;
			digitalWrite(GreenLedPin, static_cast<uint8_t>(blinkGreen));
		}
		blinkPreviousMillis = millis();
	}

	display.showNumberDec(remainingSeconds / 1000);
}
