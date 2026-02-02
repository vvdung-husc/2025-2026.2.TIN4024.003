# Dự án dành cho nhóm - ESP32, DHT, OLED Display
- ## Yêu cầu thực hiện theo nhóm (2 đến 5 thành viên mỗi nhóm)
	- Tạo dự án có tên **ESP32_DHT_OLED** tại thư mục của nhóm mình
	
		> ví dụ: ..\TEAM_X\ESP32_DHT_OLED
	- Thời gian đến hết ngày **09/02/2026**
	
		> Sau thời gian này các commit thay đổi nội dung của dự án sẽ không được tính.

## Một số yêu cầu 
- Hiển thị thông tin nhiệt độ và độ ẩm trên bảng OLED
- Hiển thị 3 đèn LED nhấp nháy theo ngưỡng nhiệt độ
### 🔥 Ngưỡng nhiệt độ hiển thị trên OLED và tín hiệu LED
| Nhiệt độ (°C) | Hiển thị | Đèn nhấp nháy |
|--------------|---------------------|-----|
| **< 13°C**  | TOO COLD | :snowflake: GREEN |
| **13 - 20°C** | COLD | :umbrella: GREEN |
| **20 - 25°C** | COOL | :cloud: YELLOW |
| **25 - 30°C** | WARM | :sunny: YELLOW |
| **30 - 35°C**  | HOT | :sun_with_face: RED |
| **> 35°C**  | TOO HOT | :fire: RED |
 
 ### Thông tin các thành viên trong mã nguồn ***main.cpp***
 >Ghi thông tin nhóm vào ngay đầu mã nguồn  
 ```cpp
 /*
THÔNG TIN NHÓM X
1. Nguyễn Văn Bình
2. Lê Nguyễn Hương Nguyên
3. ...
*/

#include  <Arduino.h>
 ```


### Nhóm tự thảo luận và tìm hiểu các thiết bị trong dự án 
![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/ESP32_DHT_OLED.png)

1\. **DHT22 - Cảm biến nhiệt độ, độ ẩm**
> Cảm biến nhiệt độ, độ ẩm

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/DHT.png)

[Tham khảo - https://docs.wokwi.com/parts/wokwi-dht22](https://docs.wokwi.com/parts/wokwi-dht22)

***Thư viện sử dụng***

> **DHT sensor library**  by  Adafruit
> 
> Arduino library for DHT11, DHT22, etc Temp & Humidity 

-------

2\. **SSD1306 OLED Display - Hiển thị OLED**

> Bảng hiển thị thông tin nhiệt độ, độ ẩm,...
 
![enter image description here](https://raw.githubusercontent.com/wokwi/wokwi-boards/main/boards/ssd1306/board.svg)

[Tham khảo - https://docs.wokwi.com/parts/board-ssd1306](https://docs.wokwi.com/parts/board-ssd1306)

***Thư viện sử dụng***
> **Adafruit_SSD1306**  by  Adafruit
> 
> SSD1306 oled driver library for monochrome 128x64 and 128x32 displays

---


