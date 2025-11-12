#include "web.h"

WebServer server(80);
int umbralPasos = 100;
int pasos = 0;
float temperatura = 24.5;

const char* ssid = "Wokwi-GUEST";
const char* password = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/datos", handleDatos);
  server.on("/setUmbral", handleSetUmbral);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

  // 模拟数据更新
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    lastUpdate = millis();
    pasos += random(5, 15);
    temperatura += (random(-5, 5)) / 10.0;
    handleDatos();
  }

  server.handleClient();
  //delay(1000);
}
