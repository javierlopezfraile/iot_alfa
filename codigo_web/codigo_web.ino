#include "web.h"

WebServer server(80);
int umbralPasos = 100;
int pasos = 0;
float temperatura = 24.5;

const char* ssid = "RedmiLeo";
const char* password = "12345678";

String generarFecha() {
  // 模拟时间戳
  return "2025-11-08";
}

String generarHora() {
  return "18:00:00";
}

String generarDatosJSON() {
  String json = "{";
  json += "\"umbralPasos\":" + String(umbralPasos) + ",";
  json += "\"pasos\":{\"fecha\":\"" + generarFecha() + "\",\"valor\":" + String(pasos) + "},";
  json += "\"temp\":{\"fecha\":\"" + generarFecha() + "\",\"hora\":\"" + generarHora() + "\",\"valor\":" + String(temperatura, 2) + "}";
  json += "}";
  return json;
}

void handleRoot() {
  extern const char index_html[] PROGMEM;
  server.send_P(200, "text/html", index_html);
}

void handleDatos() {
  server.send(200, "application/json", generarDatosJSON());
  Serial.println(generarDatosJSON());
}

void handleSetUmbral() {
  if (server.hasArg("valor")) {
    umbralPasos = server.arg("valor").toInt();
    //Serial.printf("Nuevo umbral recibido: %d\n", umbralPasos);
  }
  server.send(200, "text/plain", "OK");
}


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
    //handleDatos();
  }

  server.handleClient();
  delay(5000);
}
