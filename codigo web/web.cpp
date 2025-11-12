#include "web.h"

extern WebServer server;
extern int umbralPasos;
extern int pasos;
extern float temperatura;

String generarFecha() {
  // 模拟时间戳
  return "2025-11-08 12:00:00";
}

String generarDatosJSON() {
  String json = "{";
  json += "\"umbralPasos\":" + String(umbralPasos) + ",";
  json += "\"pasos\":{\"fecha\":\"2025-11-08\",\"valor\":" + String(pasos) + "},";
  json += "\"temp\":{\"fecha\":\"" + generarFecha() + "\",\"valor\":" + String(temperatura, 2) + "}";
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
    Serial.printf("Nuevo umbral recibido: %d\n", umbralPasos);
  }
  server.send(200, "text/plain", "OK");
}



