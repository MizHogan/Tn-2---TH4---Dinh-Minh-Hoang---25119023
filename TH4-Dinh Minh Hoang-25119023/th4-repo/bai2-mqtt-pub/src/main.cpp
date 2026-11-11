// Bài 2 — MQTT: publish telemetry mỗi 2 giây vào aet2022/<SV_ID>/telemetry
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID   = "MizHoang";            // DOI
const char* WIFI_PASS   = "527527527";            // DOI
const char* MQTT_BROKER = "broker.hivemq.com";   // DOI cho đúng broker lớp dùng
const int   MQTT_PORT   = 1883;
const char* SV_ID       = "nhom-01";             // DOI

WiFiClient wifi;
PubSubClient mqtt(wifi);
String TOPIC_TELE;
String CLIENT_ID;
unsigned long lan = 0;

float docNhiet() { return 27.0 + 2.0 * sin(millis() / 10000.0); }
float docAm()    { return 55.0 + 5.0 * cos(millis() / 15000.0); }

void noiBroker() {
  while (!mqtt.connected()) {
    Serial.print("Noi broker... ");
    if (mqtt.connect(CLIENT_ID.c_str())) {
      Serial.println("OK");
    } else {
      Serial.printf("loi rc=%d, thu lai sau 2s\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.print("\nIP: "); Serial.println(WiFi.localIP());

  TOPIC_TELE = String("aet2022/") + SV_ID + "/telemetry";
  // client id duy nhất mỗi kit: ghép MAC vào id
  String mac = WiFi.macAddress(); mac.replace(":", "");
  CLIENT_ID = String("esp32-") + SV_ID + "-" + mac;

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  Serial.println("Topic: " + TOPIC_TELE);
}

void loop() {
  if (!mqtt.connected()) noiBroker();
  mqtt.loop();                                   // GOI thuong xuyen

  if (millis() - lan >= 2000) {
    lan = millis();
    JsonDocument doc;
    doc["temp"]   = docNhiet();
    doc["hum"]    = docAm();
    doc["uptime"] = millis() / 1000;
    doc["rssi"]   = WiFi.RSSI();                 // trường của riêng mình
    char b[128];
    serializeJson(doc, b, sizeof(b));
    mqtt.publish(TOPIC_TELE.c_str(), b);
    Serial.println(b);
  }
}
