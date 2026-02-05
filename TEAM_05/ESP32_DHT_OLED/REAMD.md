| STT | Phần thay đổi | Hiện tại đã làm những gì | File | Vị trí thay đổi | Commit message |
|-----|----------------|---------------------------|------|-----------------|----------------|
| 2 | Cập nhật define và pin cho LED GREEN | Chưa làm gì, vẫn dùng LED_BLUE 15 cho green | `main.cpp` | Dòng ~30-40 (define), ~50-60 (setup()) | `Add define and pin setup for green LED` |
| 6 | Thêm hiển thị emoji/icon trên OLED (tùy chọn) | Chưa làm gì | `main.cpp` | Dòng ~90-95 (loop()) | `Add emoji/icons to OLED display for temperature status` |
| 7 | Cập nhật Serial Monitor để debug | Chưa làm gì | `main.cpp` | Dòng ~120-125 (loop()) | `Add Serial output for debugging temperature status and LED` |
| 8 | Test và tinh chỉnh (nếu cần) | Đã build thành công (.pio/ có file), nhưng chưa test simulation chi tiết | `main.cpp` / `diagram.json` | Toàn bộ | `Test and fix any issues in simulation` |
