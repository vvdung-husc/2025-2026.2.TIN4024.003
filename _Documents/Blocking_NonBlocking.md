# Khái niệm Blocking và Non-blocking trong lập trình Arduino

## 1. Giới thiệu  
Trong lập trình vi điều khiển (như ESP32), **blocking** (chặn) và **non-blocking** (không chặn) là hai cách tiếp cận quan trọng để xử lý thời gian thực.

---

## 2. Blocking (Chương trình bị chặn)  

- **Khái niệm**: Một tác vụ được gọi là **blocking** khi nó dừng toàn bộ chương trình cho đến khi tác vụ đó hoàn thành. Trong thời gian chờ, vi xử lý không thể thực hiện bất kỳ công việc nào khác.
- **Ví dụ**: Khi sử dụng hàm `delay(1000)`, chương trình sẽ dừng lại trong 1 giây và không làm gì khác trong khoảng thời gian này.
- **Ưu điểm**: Đơn giản, dễ hiểu và dễ triển khai.
- **Nhược điểm**: Lãng phí thời gian và tài nguyên, không phù hợp với các ứng dụng đa nhiệm hoặc cần phản hồi nhanh.

**Ví dụ code blocking**:
```cpp
void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Blocking example");
  delay(1000); // Chương trình dừng lại 1 giây
}
```

## 3. Non-Blocking (Chương trình không bị chặn) 

- **Khái niệm**: Một tác vụ được gọi là **non-blocking** khi nó không dừng chương trình lại. Thay vào đó, chương trình tiếp tục thực hiện các tác vụ khác trong khi chờ đợi.
- **Ví dụ**: Sử dụng `millis()` để kiểm tra thời gian thay vì dùng `delay()`. Chương trình sẽ tiếp tục chạy và chỉ thực hiện tác vụ khi điều kiện thời gian được đáp ứng.
- **Ưu điểm**: Tận dụng tối đa thời gian và tài nguyên, phù hợp với các ứng dụng đa nhiệm hoặc cần phản hồi nhanh.
- **Nhược điểm**: Code phức tạp hơn so với blocking.

**Ví dụ code blocking**:
```cpp
unsigned long previousMillis = 0;
const long interval = 1000; // Khoảng thời gian 1 giây

void setup() {
  Serial.begin(115200);
}

void loop() {
  unsigned long currentMillis = millis(); // Lấy thời gian hiện tại

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Cập nhật thời gian
    Serial.println("Non-blocking example");
  }

  // Các tác vụ khác có thể được thực hiện ở đây
```

## 4. So sánh Blocking vs Non-Blocking  

| Tiêu chí             | Blocking (Chặn)                   | Non-Blocking (Không chặn)       |
|----------------------|----------------------------------|---------------------------------|
| **Thực thi**        | Chờ hoàn thành tác vụ            | Chạy tác vụ khác trong khi chờ  |
| **Hiệu suất**       | Thấp (chặn toàn bộ chương trình) | Cao (đa nhiệm tốt)             |
| **Ứng dụng phù hợp**| Nhiệm vụ đơn giản, không yêu cầu phản hồi nhanh | Hệ thống đa nhiệm, điều khiển thời gian thực |
| **Ví dụ điển hình** | `delay()`, `while()` chờ điều kiện | `millis()`, `Ticker`, `ESP32 Timer` |

## 5. Kết luận  

- **Blocking (`delay()`, `while()`)** chỉ phù hợp với các chương trình đơn giản.  
- **Non-Blocking (`millis()`, `Ticker`, `ESP32 Timer`)** giúp tối ưu hiệu suất, cho phép ESP32 xử lý nhiều tác vụ đồng thời.  
- Khi lập trình ESP32, **nên ưu tiên non-blocking** để hệ thống phản hồi nhanh và ổn định.  
- Tránh sử dụng `delay()` trong các ứng dụng phức tạp vì nó có thể gây ra hiện tượng "treo" hệ thống

> 🔥 **Hãy sử dụng `millis()` thay vì `delay()` để tối ưu hóa chương trình của bạn!**  

