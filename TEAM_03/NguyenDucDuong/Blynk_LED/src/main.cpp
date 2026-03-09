#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL27bE4fg_1"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_AUTH_TOKEN "EbaiP8qdWS3OOF2OqslNU8sr1I-Daaey"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Nhật Funky";  //Tên mạng WiFi
char pass[] = "@@@@@@@@";             //Mật khẩu mạng WiFi


#define btnBLED  23 //Chân kết nối nút bấm
#define pinBLED  21 //Chân kết nối đèn xxanh

#define CLK 18  //Chân kết nối CLK của TM1637
#define DIO 19  //Chân kết nối DIO của TM1637

#define DHTPIN 16      //Chân kết nối DATA của DHT22
#define DHTTYPE DHT22  //Loại cảm biến DHT22

//Biến toàn cục
ulong currentMiliseconds = 0; //Thời gian hiện tại - miliseconds 
bool blueButtonON = true;     //Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637
float temperature = 0;        //Nhiệt độ (°C)
float humidity = 0;           //Độ ẩm (%)

//Khởi tạo màn hình TM1637 và cảm biến DHT22
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  dht.begin(); //Khởi động cảm biến DHT22
  
  // Start the WiFi connection
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  
  digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); //Đồng bộ trạng thái trạng thái của đèn với Blynk
  Blynk.virtualWrite(V4, "Võ Việt Dũng");
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();  //Chạy Blynk để cập nhật trạng thái từ Blynk Cloud

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  readDHT();
}

// put function definitions here:
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}
void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
    display.clear();
  }    
}

void uptimeBlynk(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  //Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk.
}

// Đọc DHT22 mỗi 2 giây, luân phiên hiển thị nhiệt độ và độ ẩm trên TM1637
void readDHT(){
  static ulong lastTime = 0;
  static bool showTemp = true; //true = hiển thị nhiệt độ, false = hiển thị độ ẩm
  if (!IsReady(lastTime, 2000)) return;

  float t = dht.readTemperature(); //Đọc nhiệt độ (°C)
  float h = dht.readHumidity();    //Đọc độ ẩm (%)

  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;

  Blynk.virtualWrite(V2, temperature); //Gửi nhiệt độ lên chân ảo V2
  Blynk.virtualWrite(V3, humidity);    //Gửi độ ẩm lên chân ảo V3

  Serial.print("Nhiet do: "); Serial.print(temperature); Serial.print(" C  ");
  Serial.print("Do am: ");    Serial.print(humidity);    Serial.println(" %");

  if (blueButtonON){
    if (showTemp){
      // Hiển thị nhiệt độ có 1 chữ số thập phân, ví dụ 25.6 -> hiện "25.6"
      display.showNumberDecEx((int)(temperature * 10), 0x40, false, 4, 0);
    } else {
      // Hiển thị độ ẩm nguyên, ví dụ 60% -> hiện "  60"
      display.showNumberDec((int)(humidity), false, 4, 0);
    }
    showTemp = !showTemp; //Đổi qua lại giữa nhiệt độ và độ ẩm mỗi 2 giây
  }
}

//được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V1) { //virtual_pin định nghĩa trong ứng dụng Blynk
  // Xử lý dữ liệu nhận được từ ứng dụng Blynk
  blueButtonON = param.asInt();  // Lấy giá trị từ ứng dụng Blynk
  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    
  }
  else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}