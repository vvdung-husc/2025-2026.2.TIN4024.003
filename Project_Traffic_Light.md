# Dự án - Đèn giao thông

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32_trafic.png)

---

- ## Các thiết bị trong dự án
 
	1\.  **LED - Đèn LED**
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/LED.png)
		
	[Tham khảo - https://docs.wokwi.com/parts/wokwi-led](https://docs.wokwi.com/parts/wokwi-led)
		
	2\.  **Resistor - Điện trở**
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/Resistor.png)
	
	[Tham khảo - https://docs.wokwi.com/parts/wokwi-resistor](https://docs.wokwi.com/parts/wokwi-resistor)

	3\.  **PushButton - Nút nhấn**

	> Bật tắt thiển thị thời gian đếm ngược của đèn
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/PushButton.png)
	
	[Tham khảo - https://docs.wokwi.com/parts/wokwi-pushbutton](https://docs.wokwi.com/parts/wokwi-pushbutton)
	
	4\.  **TM1637 - Bảng hiển thị đèn LED**

	> Bảng hiển thị thời gian
		
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/TM1637.png)

	[Tham khảo - https://docs.wokwi.com/parts/wokwi-tm1637-7segment](https://docs.wokwi.com/parts/wokwi-tm1637-7segment)

	***Thư viện sử dụng***
	> **TM1637** by Avishay - Arduino library for TM1637

	```cpp
	#include <TM1637Display.h>

	#define CLK 2
	#define DIO 3
			  
	TM1637Display display(CLK, DIO);	
	```

	5\.  **LDR - Cảm biến quang điện trở**

	> Dùng để bật/tắt đèn đường, đèn trong nhà dựa trên cường độ ánh sáng môi trường.
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/LDR.png)

	[Tham khảo - https://docs.wokwi.com/parts/wokwi-photoresistor-sensor](https://docs.wokwi.com/parts/wokwi-photoresistor-sensor)

	6\.  **ESP32 Board - DevkitC-v4**

	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32_board.png)
		
---

## Thêm thư viện vào dự án

### Ví dụ thêm thư viện TM1637 

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/Library_import_1.png)

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/Library_import_2.png)


---
