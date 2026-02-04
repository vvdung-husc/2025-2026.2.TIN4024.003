| STT | Phần thay đổi | File | Vị trí thay đổi | Commit message |
|-----|----------------|------|-----------------|----------------|
| 1 | Thêm LED GREEN vào diagram | `diagram.json` | Dòng ~20-50 (parts), ~100-200 (connections) | `Add green LED to diagram for temperature thresholds` |
| 2 | Cập nhật define và pin cho LED GREEN | `main.cpp` | Dòng ~30-40 (define), ~50-60 (setup()) | `Add define and pin setup for green LED` |
| 3 | Sửa logic xác định trạng thái nhiệt độ (6 ngưỡng) | `main.cpp` | Dòng ~80-100 (loop()) | `Update temperature threshold logic to match 6 levels from requirements` |
| 4 | Cập nhật hiển thị trạng thái trên OLED | `main.cpp` | Dòng ~85-110 (loop()) | `Update OLED display to show correct temperature status` |
| 5 | Sửa logic nhấp nháy LED theo trạng thái | `main.cpp` | Dòng ~75-85 (loop()) | `Update LED blinking logic to match temperature thresholds` |
| 6 | Thêm hiển thị emoji/icon trên OLED (tùy chọn) | `main.cpp` | Dòng ~90-95 (loop()) | `Add emoji/icons to OLED display for temperature status` |
| 7 | Cập nhật Serial Monitor để debug | `main.cpp` | Dòng ~120-125 (loop()) | `Add Serial output for debugging temperature status and LED` |
| 8 | Test và tinh chỉnh (nếu cần) | `main.cpp` / `diagram.json` | Toàn bộ | `Test and fix any issues in simulation` |
