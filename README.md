# SMART CLOCK
Đề tài hướng đến việc xây dựng một hệ thống đồng hồ thông minh sử dụng ESP32, có khả năng hiển thị thời gian thực và kết nối internet để đồng bộ dữ liệu. Hệ thống giúp người dùng theo dõi giờ, phút, giây chính xác theo múi giờ Việt Nam.

# Hệ thống cung cấp các chức năng sau:
## Hiển thị Thời gian Thực (Real-Time Clock - RTC):
Hiển thị Ngày, tháng, năm, giờ, phút, giây.

Sử dụng NTP Server để đồng bộ thời gian khi có kết nối Wi-Fi.

Sử dụng DS3231 để duy trì và đồng bộ thời gian khi mất kết nối Internet.
## Đo lường Môi trường:
Đọc và hiển thị nhiệt độ, độ ẩm thực tế trong phòng thông qua cảm biến DHT22.
## Dự báo Thời tiết:
Cập nhật dự báo thời tiết online từ API (ví dụ: OpenWeatherMap).
## Báo thức:
Chức năng báo thức với thời gian do người dùng cài đặt.

Phát âm thanh cảnh báo qua Buzzer (hoặc loa) khi đến giờ đặt trước.
## Điều khiển bằng Nút nhấn (Button):
Nút 1: Đảo chế độ chỉnh giờ/phút cho báo thức.

Nút 2: Tăng giờ hoặc tăng phút.

Nút 3: Bật/tắt báo thức.

# 📋 Linh Kiện và Module Chính

| **Tên Module**                         | **Mô tả Chức năng**                                                        |
|----------------------------------------|----------------------------------------------------------------------------|
| **ESP32-WROOM-32 DevKit V1**          | Bộ xử lý trung tâm, kết nối WiFi/Bluetooth                                |
| **Màn hình LCD TFT (2.4'' SPI 240×320)** | Hiển thị thời gian, nhiệt độ, độ ẩm, và thông tin thời tiết               |
| **Cảm biến DHT22**                     | Đo nhiệt độ và độ ẩm trong phòng                                          |
| **Mạch thời gian thực DS3231**        | Cung cấp dữ liệu thời gian chính xác, có pin dự phòng                     |
| **Buzzer (TMB12A05)**                  | Phát âm thanh báo thức hoặc cảnh báo                                      |
| **Nút nhấn (Button)**                  | 3 nút nhấn để điều chỉnh thời gian và báo thức                            |
| **Module khuếch đại âm thanh (MAX98357 I2S)** | Dùng để khuếch đại tín hiệu âm thanh ra loa (nếu dùng loa ngoài 1W 8Ω 50mm) |

# Sơ đồ nối chân linh kiện
<img width="999" height="535" alt="image" src="https://github.com/user-attachments/assets/7b139598-6c04-4e52-9e42-ee5fd697dfdc" />

