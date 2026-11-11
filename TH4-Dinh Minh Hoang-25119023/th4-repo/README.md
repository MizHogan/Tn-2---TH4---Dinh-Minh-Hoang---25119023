# TH4 — REST/MQTT và dashboard Node-RED (AET2022)

- Họ tên / MSSV: **25119023**
- SV_ID dùng trong topic: `nhom-03` 
- Broker: `broker.hivemq.com:1883` (hoặc Mosquitto của lớp)
- IP ESP32 (Bài 1): `...`

## Cấu trúc repo

| Thư mục | Nội dung |
|---|---|
| `bai1-rest/` | ESP32 WebServer, `GET /api/sensor` trả JSON `{id,temp,hum,uptime,rssi}` |
| `bai2-mqtt-pub/` | Publish telemetry mỗi 2 s vào `aet2022/<sv_id>/telemetry` |
| `bai3-mqtt-sub-led/` | Cả bộ kit: SHT31 + nút + LED (GPIO14) + OLED; subscribe `aet2022/<sv_id>/cmd/led` |
| `nodered-flow/flow.json` | Flow Node-RED export (gauge, chart, đèn báo nút, nút BẬT/TẮT) |
| `media/` | Ảnh/video kết quả |

Build: mở từng thư mục bằng PlatformIO → sửa `WIFI_SSID`, `WIFI_PASS`, `SV_ID`, `MQTT_BROKER` ở đầu `main.cpp` → Upload.

## Bài 1 — REST
Trường tự thêm: `25119023` và `rssi` (`WiFi.RSSI()`).
![bai1](media/bai1-json.png)

## Bài 2 — MQTT publish
Topic: `aet2022/<25119023>/telemetry`; client id ghép MAC nên không trùng giữa các kit.
![bai2](media/bai2-explorer.png)

## Bài 3 — Node-RED
Chiều đi: `mqtt in` → `function` tách trường → gauge/chart/LED.
Chiều về: `ui_button` (payload `1`/`0`) → `mqtt out` → `.../cmd/led`.
Phần tự tùy biến: **...** (ghi rõ mình thêm gì)
![bai3](media/bai3-dashboard.png)
Video: `media/bai3-led.mp4`

## Bài 4 — Soi bằng MQTT Explorer
Subscribe `aet2022/<25119023>/#`: thấy `telemetry` và `cmd/led` (`1`/`0` khi bấm nút), không lẫn namespace nhóm khác.
![bai4](media/bai4-explorer.png)

## So sánh REST và MQTT (ngắn)
REST: client hỏi, server đáp (kéo) — hợp lấy dữ liệu theo yêu cầu. MQTT: thiết bị tự đẩy qua broker — hợp telemetry liên tục, nhiều thiết bị, dashboard realtime.
