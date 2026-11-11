// Bài 3 — cả bộ kit: SHT31 (nhiệt/ẩm) + nút + LED + OLED qua MQTT
//  Chiều ĐI : publish aet2022/<SV_ID>/telemetry  {temp,hum,uptime,rssi,btn}
//  Chiều VỀ : subscribe aet2022/<SV_ID>/cmd/led  ('1' bật / '0' tắt)
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* WIFI_SSID   = "MizHoang";            // DOI
const char* WIFI_PASS   = "527527527";            // DOI
const char* MQTT_BROKER = "broker.hivemq.com";   // DOI cho đúng broker lớp dùng
const int   MQTT_PORT   = 1883;
const char* SV_ID       = "nhom-03";             // DOI

// Chân — chỉnh theo sơ đồ nối của kit
const int PIN_LED = 14;      // LED qua điện trở ~220Ω về GND
const int PIN_BTN = 27;      // nút nhấn, INPUT_PULLUP (nhấn = LOW)
const int PIN_SDA = 21, PIN_SCL = 22;

WiFiClient wifi;
PubSubClient mqtt(wifi);
Adafruit_SHT31 sht31;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

String TOPIC_TELE, TOPIC_LED, CLIENT_ID;
unsigned long lan = 0;
bool ledOn = false, btnPrev = false;
float temp = NAN, hum = NAN;

void veOled() {
  oled.clearDisplay();
  oled.setTextSize(1); oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);  oled.print(SV_ID);
  oled.setCursor(0, 16); oled.printf("Nhiet: %.1f C", temp);
  oled.setCursor(0, 28); oled.printf("Do am: %.1f %%", hum);
  oled.setCursor(0, 40); oled.printf("LED  : %s", ledOn ? "BAT" : "TAT");
  oled.setCursor(0, 52); oled.printf("Nut  : %s", btnPrev ? "NHAN" : "THA");
  oled.display();
}

// chiều VỀ: lệnh từ Node-RED
void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  Serial.printf("[%s] %s\n", topic, msg.c_str());
  if (String(topic) == TOPIC_LED) {
    ledOn = (msg == "1");
    digitalWrite(PIN_LED, ledOn ? HIGH : LOW);
    veOled();
  }
}

void noiBroker() {
  while (!mqtt.connected()) {
    Serial.print("Noi broker... ");
    if (mqtt.connect(CLIENT_ID.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_LED.c_str());
    } else {
      Serial.printf("loi rc=%d\n", mqtt.state());
      delay(2000);
    }
  }
}

void guiTelemetry() {
  JsonDocument doc;
  doc["temp"]   = temp;
  doc["hum"]    = hum;
  doc["uptime"] = millis() / 1000;
  doc["rssi"]   = WiFi.RSSI();
  doc["btn"]    = btnPrev ? 1 : 0;
  char b[160];
  serializeJson(doc, b, sizeof(b));
  mqtt.publish(TOPIC_TELE.c_str(), b);
  Serial.println(b);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!sht31.begin(0x44)) Serial.println("Khong thay SHT31!");
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("Khong thay OLED!");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.print("\nIP: "); Serial.println(WiFi.localIP());

  TOPIC_TELE = String("aet2022/") + SV_ID + "/telemetry";
  TOPIC_LED  = String("aet2022/") + SV_ID + "/cmd/led";
  String mac = WiFi.macAddress(); mac.replace(":", "");
  CLIENT_ID = String("esp32-") + SV_ID + "-" + mac;

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (!mqtt.connected()) noiBroker();
  mqtt.loop();

  // nút nhấn: gửi ngay khi đổi trạng thái
  bool btn = (digitalRead(PIN_BTN) == LOW);
  if (btn != btnPrev) {
    btnPrev = btn;
    guiTelemetry();
    veOled();
    delay(30);   // chống dội
  }

  if (millis() - lan >= 2000) {
    lan = millis();
    temp = sht31.readTemperature();
    hum  = sht31.readHumidity();
    guiTelemetry();
    veOled();
  }
}
