


# 2025-2026.2.TIN4024.003

## Phát triển ứng dụng IoT

## WiFi Lab: CNTT-MMT/13572468

  

-  #### WOKWI - World's most advanced ESP32 simulator

	Đăng ký tài khoản tại [wokwi.com](https://wokwi.com/)
	>Dùng để thiết kế mạch IoT cho việc mô phỏng

-  #### Your code editor

	Cài đặt [Visual Studio Code](https://code.visualstudio.com/)
	>Lập trình cho mạch đã thiết kế, chạy mô phỏng và nạp vào vi mạch thực hành

-  #### Your Gateway to Embedded Software Development Excellence

	Cài đặt [PlatformIO Extension](https://platformio.org/) cho VSCode

	> Biên dịch mã nguồn c/c++ cho thiết bị vi mạch

-  #### Wokwi Embedded Simulator

	Cài đặt [Wokwi Simulator](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode) cho VSCode
	>Chạy mô phỏng mạch kết hợp
  
-  #### Reference videos - Wokwi, PlatformIO, VS Code

	- [Mô phỏng các dự án IoT trong VS Code | Wokwi, PlatformIO, VSCode](https://www.youtube.com/watch?v=9pTZL934k2s)
	- [Bắt đầu lập trình Arduino, ESP32 với PlatformIO trên VSCode](https://www.youtube.com/watch?v=20eakkralUs)
---

#### [Đăng ký tài khoản Github, nhóm thực hành](https://docs.google.com/spreadsheets/d/1AYRu1G8-8MHzO_bBbn2DuxU_Bzcy306O/edit?usp=sharing&ouid=100313771319244626917&rtpof=true&sd=true)

---

#### [Tổng quan về sơ đồ chân ESP32 và Ngoại vi](https://khuenguyencreator.com/tong-quan-ve-so-do-chan-esp32-va-ngoai-vi/)

#### [Lập trình ESP32 từ A tới Z](https://khuenguyencreator.com/lap-trinh-esp32-tu-a-toi-z/)

#### [Lập trình ESP32 GPIO Digital Input và Digital Output](https://khuenguyencreator.com/lap-trinh-esp32-gpio-digital-input-va-digital-output/)

---

#### Tham khảo

  
+ [Tạo một dự án lập trình ESP32 với PlatformIO](https://khuenguyencreator.com/huong-dan-cai-dat-platform-io-lap-trinh-esp32/#Huong_dan_su_dung_Platform_IO_lap_trinh_ESP32)

  
+ [Wokwi for VS Code](https://docs.wokwi.com/vscode/getting-started)

  
+ [Cài đặt thư viện cho PlatformIO](https://khuenguyencreator.com/huong-dan-cai-dat-platform-io-lap-trinh-esp32/#Cai_dat_thu_vien_cho_Platformio)

--------------------
***2026-03-12***

## ESP8266 - NodeMCU v2 và v3
![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/NodeMCU-V2-vs-V3.jpg)

## ESP8266 Controller
![](https://github.com/vvdung/storage/blob/main/IOT/esp8266_controller.png)

## [**Sơ đồ Board thực hành**](https://github.com/vvdung/storage/blob/main/IOT/CNTT%20Board%20schematic.pdf)

## [Tải và cài đặt trình điều khiển (CH341 Windows Drivers)](https://drive.google.com/file/d/1Qbuh5fdXORbVRjsayQjNQgQ2_EU--v2r/view?usp=sharing)

#### Tạo dự án - ESP32_Project 
+ Board ***NodeMCU 1.0*** trong 
+ Điều khiển đèn nhấp nháy trên Board ESP8266
+ Đọc thông tin nhiệt độ
+ Hiển thị lên màn hình OLED (SH1106)
  > Thư viện hổ trợ U8g2 by oliver


--------------------
***2026-03-09***

[Telegram IoT Testing Link](https://t.me/+GzXzLiCzYOxkZTc1)

### - [Esp32 |  TELEGRAM BOT (***mã nguồn minh họa***)](https://wokwi.com/projects/425361659331202049)

#### Tham khảo

+ [ESP32 Telegram Bot](https://www.iotzone.vn/esp32/cach-dung-esp32-telegram-dieu-khien-den-led-voi-arduino-ide/)

#### Ứng dụng Telegram

>[Telegram Applications](https://telegram.org/apps)

>[Telegram APIs](https://core.telegram.org/api)

## => Yêu cầu thực hiện được đoạn mã nguồn minh họa để hiển thị trong ứng dụng Telegram

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32_telegram_led_control.png)
--------------------
***2026-03-05***

### - [Esp32 | API HTTP GET (***mã nguồn minh họa***)](https://wokwi.com/projects/425209099504209921)

#### Tham khảo

+ [ESP32 HTTP Client phương thức Get](https://khuenguyencreator.com/lay-du-lieu-thoi-tiet-voi-esp32-http-client-phuong-thuc-get/)

#### Một số phần mềm

+ [CURL for Windows](https://curl.se/windows/)
+ [Postman - Kiểm thử API](https://www.postman.com/)

#### http://ip4.iothings.vn?geo=1
>Thông tin địa chỉ IPv4 đang sử dụng và vị trí địa lý (Geolocation - Latitude: Vĩ độ, Longitude: Kinh độ) 

#### http://www.google.com/maps/place/16.4591267,107.5901477
>Google Maps tại Latitude, Longitude

## => Hãy lập trình trong ESP32 sử dụng API HTTP GET để lấy được IPv4, Latitude, Longitude và đưa ra Link Google Maps (Hiển thị tại Terminal)

## => Đăng ký tài khoản https://openweathermap.org/, sử dụng API http://ip4.iothings.vn?geo=1 và API https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={API key} để thực hiện trên Blynk như hình sau
> Xử lý chuỗi và JSON trả về từ API

> Đính kèm ảnh của mobile và web lên github, trong ảnh phải có tên sinh viên thực hiện

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/ESP32_API.png)

+ [***Đính kèm ảnh của mobile và web lên github***](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/mobile_esp32_dht.png)

	> Ảnh phải có tên của cá nhân trong ứng dụng
--------------------
***2026-03-02***

### - [Blynk | Traffic & DHT Sensor](https://wokwi.com/projects/424198235739151361)
+ ***Thư viện sử dụng***
	> **# DHT sensor library** by Avishay - Arduino library for DHT11, DHT22, etc Temp & Humidity Sensors
	
![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/traffic_blynk_1.png)

+ [***Đính kèm ảnh của mobile và web lên github***](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/mobile_esp32_dht.png)

	> Ảnh phải có tên của cá nhân trong ứng dụng

--------------------

### - [Esp32 | Blynk & Wokwi](https://wokwi.com/projects/423790624312911873) 

#### Tham khảo

+ [Sử dụng Blynk IOT với ESP32](https://dienthongminhesmart.com/lap-trinh-esp32/blynk-iot-va-esp32/)
+ ***Thư viện sử dụng***
	> **TM1637** by Avishay - Arduino library for TM1637
 
	> **Blynk** by Volodymyr Shymanskyy

+ [Sử dụng Blynk IOT trên ESP32 (nếu không xem được từ youtube)](https://it.iothings.vn/downloads/mp4/Blynk_IOT_ESP32_WEB.mp4)

+ [https://it.iothings.vn/downloads/mp4](https://it.iothings.vn/downloads/mp4/)

>>>>>>> f41be4aa6fe85225993beff762612d64b89070de
-------------------------------

## [***1. Dự án - LED BLINK***](https://github.com/vvdung-husc/2025-2026.2.TIN4024.003/blob/main/Project_LED_Blink.md)

## [***2. Dự án - TRAFFIC LIGHT***](https://github.com/vvdung-husc/2025-2026.2.TIN4024.003/blob/main/Project_Traffic_Light.md)

## [***3. Dự án - DHT_OLED (Theo nhóm, hạn cuối 09/02/2026)***](https://github.com/vvdung-husc/2025-2026.2.TIN4024.003/blob/main/Team_DHT_OLED.md)

## [***4. Dự án - BLYNK_DHT***](https://github.com/vvdung-husc/2025-2026.2.TIN4024.003/blob/main/README.md#--blynk--traffic--dht-sensor)

## [***5. Dự án - BLYNK_API***](https://github.com/vvdung-husc/2025-2026.2.TIN4024.003/blob/main/README.md#-%C4%91%C4%83ng-k%C3%BD-t%C3%A0i-kho%E1%BA%A3n-httpsopenweathermaporg-s%E1%BB%AD-d%E1%BB%A5ng-api-httpip4iothingsvngeo1-v%C3%A0-api-httpsapiopenweathermaporgdata25weatherlatlatlonlonappidapi-key-%C4%91%E1%BB%83-th%E1%BB%B1c-hi%E1%BB%87n-tr%C3%AAn-blynk-nh%C6%B0-h%C3%ACnh-sau)

## [***6. Dự án - TELEGRAM_LED***](https://github.com/vvdung-husc/2025-2026.2.TIN4024.003/blob/main/README.md#-y%C3%AAu-c%E1%BA%A7u-th%E1%BB%B1c-hi%E1%BB%87n-%C4%91%C6%B0%E1%BB%A3c-%C4%91o%E1%BA%A1n-m%C3%A3-ngu%E1%BB%93n-minh-h%E1%BB%8Da-%C4%91%E1%BB%83-hi%E1%BB%83n-th%E1%BB%8B-trong-%E1%BB%A9ng-d%E1%BB%A5ng-telegram)


## [***7. Dự án - ESP8266_BLYNK_TELEGRAM  (Theo nhóm, hạn cuối 23/03/2026)***]()
- ## Yêu cầu thực hiện theo nhóm
  - Tạo dự án có tên **ESP8266_BLYNK_TELEGRAM** tại thư mục của nhóm mình

	> ví dụ: ..\TEAM_X\ESP8266_BLYNK_TELEGRAM
  - Thời gian đến hết ngày **23/03/2026**
	
	> Sau thời gian này các commit thay đổi nội dung của dự án sẽ không được tính.

## Một số yêu cầu 

1. ### ***Thông tin các thành viên trong mã nguồn***

	> Ghi thông tin nhóm và tên các thành viên ở đầu mã nguồn ***main.cpp***  
```cpp
	/*
	THÔNG TIN NHÓM X
	1. Nguyễn Văn Bình
	2. Lê Nguyễn Hương Nguyên
	3. ...
	*/
```

2. ### ***Hiển thị thông tin trên bảng OLED***
3. ### ***Blynk***
  - Hiển thị thời gian hoạt động của thiết bị (uptime)
  - Hiển thị Switch điều khiển tắt mở đèn LED
  - Hiển thị thông tin nhiệt độ, độ ẩm
  - Hiển thị thông tin khí ga từ cảm biến MQ2 (nếu không có cảm biến thì sinh ngẫu nhiên)
  - Hiển thị thông tin Team X ở cuối
 4. ### ***Telegram***
  - Tạo tên nhóm **IoT - Team X** và thêm các thành viên vào nhóm và tài khoản vvdung_husc (hoặc 0396634274)
  - Hiển thị thông tin nhiệt độ, độ ẩm khi có thay đổi từ thiết bị
  - Điều khiển tắt/mở và nhận trạng thái đèn LED bằng lệnh nhập từ khung chat
 5. ### Gửi đính kèm lên github 2 ảnh của Blynk (***web***) và Telegram (***với đầy đủ tên thành viên***)
-------------------------------

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32-devkitC-v4-pinout.png)

-------------------------------

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/diagram_one.png)

-------------------------------

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/diagram_two.png)

-------------------------------
