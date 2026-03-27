Hướng dẫn nhanh — Xoá secret khỏi lịch sử Git (tiếng Việt)

1) Bắt buộc: Revoke/rotate key bị lộ
- Đăng nhập vào OpenWeather, vào trang API keys, xóa hoặc thu hồi key đã lộ.
- Nếu không làm bước này trước khi push, key vẫn có thể được sử dụng ngay cả khi bạn xóa trong lịch sử.

2) Chuẩn bị môi trường (máy của bạn):
- Cài Git
- Cài Python
- Cài git-filter-repo: `pip install git-filter-repo`

3) Tùy chọn xử lý (chạy trên máy cá nhân):
- A. Xoá file chứa key hoàn toàn khỏi lịch sử:
  - Chạy script (ví dụ):

```powershell
# chỉnh RepoUrl và MirrorDir theo đường dẫn của bạn
.
cd .\scripts
.\remove-secret.ps1 -RepoUrl "https://github.com/vvdung-husc/2025-2026.2.TIN4024.003.git" -MirrorDir "C:\\tmp\\repo.git" -RemoveFile "TEAM_05/NguyenHuynhMinhTien/ESP32_API HTPP GET/src/main.cpp"
```

- B. Thay thế giá trị key bằng `REDACTED` (nếu bạn biết chính xác key):

```powershell
cd .\scripts
.\remove-secret.ps1 -RepoUrl "https://github.com/vvdung-husc/2025-2026.2.TIN4024.003.git" -MirrorDir "C:\\tmp\\repo.git" -ReplaceKey "THE_LEAKED_KEY"
```

4) Sau khi chạy script và force-push thành công:
- Thông báo cho mọi cộng tác viên: họ phải reclone repository (lịch sử đã thay đổi).
- Kiểm tra remote (GitHub) để đảm bảo secret không còn hiển thị.

5) Nếu bạn không thể rewrite lịch sử:
- Yêu cầu admin repo dùng trang unblock (không an toàn lâu dài):
  https://github.com/vvdung-husc/2025-2026.2.TIN4024.003/security/secret-scanning/unblock-secret/3AggtID59Hcl0GFyPCSHxjP5wxD

6) Hỗ trợ từ tôi:
- Tôi có thể tạo các file và lệnh (đã tạo `scripts/remove-secret.ps1` và `scripts/replacements-example.txt`).
- Tôi không thể tự chạy lệnh trên máy của bạn; nếu muốn, tôi sẽ hướng dẫn chi tiết từng bước bạn cần chạy.
