// Bài 1 — REST: GET /api/sensor trả JSON cho máy đọc
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "MizHoang";     // DOI
const char* WIFI_PASS = "527527";     // DOI
const char* MY_ID     = "25119023";        // DOI: mã sinh viên (trường tự thêm)

WebServer server(80);

// Số đo giả để chạy ngay khi chưa có cảm biến
float docNhiet() { return 27.0 + 2.0 * sin(millis() / 10000.0); }
float docAm()    { return 55.0 + 5.0 * cos(millis() / 15000.0); }

void handleSensor() {
  JsonDocument doc;
  doc["id"]     = MY_ID;               // trường của riêng mình
  doc["temp"]   = docNhiet();
  doc["hum"]    = docAm();
  doc["uptime"] = millis() / 1000;
  doc["rssi"]   = WiFi.RSSI();         // trường của riêng mình
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Noi Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.print("\nIP ESP32: ");
  Serial.println(WiFi.localIP());

  server.on("/api/sensor", HTTP_GET, handleSensor);
  server.begin();
}

void loop() {
  server.handleClient();
}
