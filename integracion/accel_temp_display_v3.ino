#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MPU6050_light.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ESP32Time.h>
#include <WebServer.h>

WebServer server(80);

#define BUTTON_PIN 17
#define DOUBLECLICK_WINDOW 10000
#define COUNTDOWN_TIME 5000
#define DEBOUNCE_TIME 200
#define LONG_PRESS_TIME 2000       

#define TEMP_MIN 34.0
#define TEMP_MAX 41.0

#define ANCHO 128
#define ALTO 64
#define OLED_RESET -1

Adafruit_SSD1306 pantalla(ANCHO, ALTO, &Wire, OLED_RESET);

class SensorTemperaturaMLX90614 {
  private:
    Adafruit_MLX90614 sensor;
    int numMediciones;

  public:
    SensorTemperaturaMLX90614(int n = 10) : numMediciones(n) {}

    void iniciar() {
      if (!sensor.begin()) {
        Serial.println("ERROR iniciando MLX90614!");
      }
    }

    float leerUna() {
      return sensor.readObjectTempC();
    }

    float medirPromedioBruto() {
      float suma = 0;
      for (int i = 0; i < numMediciones; i++) {
        suma += leerUna();
        delay(200);
      }
      return suma / numMediciones;
    }
};

class StepCounter {
  private:
    MPU6050 mpu;
    int stepCount;
    unsigned long lastStepTime;
    bool stepDetected;

    const float accelThreshold;
    const unsigned long processInterval; 
    unsigned long lastProcess;           

    float getAccelMagnitude() {
      float ax = mpu.getAccX();
      float ay = mpu.getAccY();
      float az = mpu.getAccZ();
      return sqrt(ax*ax + ay*ay + az*az);
    }

    void printStepData(float ax, float ay, float az, float mag) {
      Serial.print("Step detected! Total steps: ");
      Serial.print(ax); Serial.print(",");
      Serial.print(ay); Serial.print(",");
      Serial.print(az); Serial.print(",");
      Serial.print(mag); Serial.print(",");
      Serial.println(stepCount);
    }

    void printNoStepData(float ax, float ay, float az, float mag) {
      Serial.print("NO STEPS: ");
      Serial.print(ax); Serial.print(",");
      Serial.print(ay); Serial.print(",");
      Serial.print(az); Serial.print(",");
      Serial.println(mag);
    }

  public:
    StepCounter(TwoWire &w = Wire, float threshold = 0.25, unsigned long processMs = 500)
      : mpu(w),
        accelThreshold(threshold),
        processInterval(processMs),
        stepCount(0),
        lastStepTime(0),
        lastProcess(0),
        stepDetected(false)
    {}

    void iniciar() {
      Wire.begin();
      mpu.begin();
      mpu.calcOffsets();
      Serial.println("MPU6050 ready!");
    }

    void actualizar() {
      unsigned long now = millis();
      if (now - lastProcess < processInterval) return; // no procesar continuamente
      lastProcess = now;

      mpu.update();

      float ax = mpu.getAccX();
      float ay = mpu.getAccY();
      float az = mpu.getAccZ();
      float mag = sqrt(ax*ax + ay*ay + az*az);

      if ((mag > (1.0 + accelThreshold)) && (mag < 1.0 + 2*accelThreshold)) {
          stepCount++;
          stepDetected = true;
          lastStepTime = now;
          printStepData(ax, ay, az, mag);
      } else {
          stepDetected = false;
          // printNoStepData(ax, ay, az, mag);
      }
    }

    unsigned long getStepCount() {
      return stepCount;
    }
};

class PantallaOLED {
  private:
    int pasos, meta;
    char buffer_fecha[11];  // DD/MM/AAAA + '\0'
    char buffer_hora[9];    // HH:MM:SS + '\0'
    bool wifi;
    float umbral;
    int ip;

  public:
    PantallaOLED() {}

    // Función para inicializar la pantalla
    void iniciar() {
      if (!pantalla.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("No se encontró la pantalla OLED"));
        for (;;);
      }
      pantalla.clearDisplay();
      pantalla.setTextSize(1);
      pantalla.setTextColor(SSD1306_WHITE);
    }

    // Función para apagar la pantalla
    void apagar() {
      pantalla.clearDisplay();
      pantalla.display();
      pantalla.ssd1306_command(SSD1306_DISPLAYOFF); // Apaga el OLED
    }

    // Función para encender la pantalla de nuevo
    void encender() {
      pantalla.clearDisplay();
      pantalla.display();
      pantalla.ssd1306_command(SSD1306_DISPLAYON);  // Enciende el OLED
    }

    void displayPredeterminado() {
      encender();
      pantalla.setCursor(0, 0);

      pantalla.print("PASOS: "); pantalla.print(pasos); pantalla.print("/"); pantalla.println(meta);
      pantalla.println();
      pantalla.print("FECHA: "); pantalla.println(buffer_fecha);
      pantalla.print("HORA: "); pantalla.println(buffer_hora);
      pantalla.println();
      pantalla.println("---------------------");
      pantalla.print("WIFI:  "); pantalla.println(wifi ? "ON" : "OFF");

      pantalla.display();
    }

    void displayEnhorabuena() {
      encender();
      pantalla.setCursor(0, 0);

      pantalla.print(" *** ENHORABUENA ***");
      pantalla.println();
      pantalla.println("META DE PASOS LOGRADA");
      pantalla.println();
      pantalla.print("    ");
      pantalla.print(pasos); pantalla.println(" PASOS");
      pantalla.println();
      pantalla.println("---------------------");
      pantalla.println(" OBJETIVO ALCANZADO!");

      pantalla.display();
    }

    void displayTomandoMedida() {
      encender();
      pantalla.setCursor(0, 0);

      pantalla.println("MIDIENDO TEMPERATURA");
      pantalla.println();
      pantalla.println("   MANTENTE QUIETO");
      pantalla.println();
      pantalla.println("---------------------");
      pantalla.println();
      pantalla.print("ESPERA 5 SEGUNDOS...");

      pantalla.display();
    }

    void displayErrorMedida() {
      encender();
      pantalla.setCursor(0, 0);

      pantalla.println(" ERROR DE MEDICION!");
      pantalla.println();
      pantalla.println("MALA MEDIDA DETECTADA");
      pantalla.println();
      pantalla.println("---------------------");
      pantalla.println();
      pantalla.print(" INTENTELO DE NUEVO");

      pantalla.display();
    }
  
    void displayEmpezarMedida(int tiempoEmpezarMedida) {
      encender();
      pantalla.setCursor(0, 0);

      pantalla.println("¡RELOJ EN LA FRENTE!");
      pantalla.println();
      pantalla.println("LA MEDIDA COMIENZA EN"); pantalla.print(tiempoEmpezarMedida); pantalla.println(" s");
      pantalla.println();
      pantalla.println("---------------------");
      pantalla.println();

      pantalla.display();
    }

    void displayMedidaCorrecta(float temperatura) { // está el estado eliminado
      encender();
      pantalla.setCursor(0, 0);

      pantalla.print("TEMPERATURA: "); pantalla.print(temperatura, 1); pantalla.println(" C");
      pantalla.println();
      pantalla.print("   UMBRAL: "); pantalla.print(umbral, 1); pantalla.println(" C");
      pantalla.println();
      pantalla.println("---------------------");
      pantalla.println();
      pantalla.print("   ESTADO: ");

      pantalla.display();
    }

    void setDatos(int p, int m, const char* fecha, const char* hora, bool w, float u) {
      pasos = p;
      meta = m;
      wifi = w;
      umbral = u;

      strncpy(buffer_fecha, fecha, sizeof(buffer_fecha) - 1);
      buffer_fecha[sizeof(buffer_fecha) - 1] = '\0';

      strncpy(buffer_hora, hora, sizeof(buffer_hora) - 1);
      buffer_hora[sizeof(buffer_hora) - 1] = '\0';
    }
};

SensorTemperaturaMLX90614 sensorTemp(10); 
StepCounter contadorPasos(Wire, 0.25, 500); 
PantallaOLED miPantalla;

bool rtcSincronizado = false;

bool esperandoSegundoClick = false;
unsigned long tiempoPrimerClick = 0;

bool enCuentaAtras = false;
unsigned long inicioCuentaAtras = 0;

bool enCuentaAtrasTemperatura = false;
unsigned long inicioCuentaAtrasTemperatura = 0;

unsigned long ultimoCambioBoton = 0;
bool botonPrev = HIGH;

bool internetConectado = false;
bool botonMantiene = false;
unsigned long tiempoInicioHold = 0;

// Configuración WiFi
const char* ssid = "iPhone de Javier"; // ACZ_22
const char* password = "12345678"; // asd123as

// Configuración NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

// Objeto RTC
ESP32Time rtc;

// Variables para fecha y hora
char fechaActual[11] = "01/01/2025"; // DD/MM/AAAA
char horaActual[9] = "00:00:00";     // HH:MM:SS

void cancelarTodo() {
  esperandoSegundoClick = false;
  enCuentaAtras = false;
  Serial.println("Operación cancelada por el usuario.");
  bool pantallaTrabajando = false;
  bool sensorTemperaturaTrabajando = false;
  bool sensorAcelerometroTrabajando = true;
  miPantalla.apagar();
}

// Función para configurar el tiempo usando ESP32Time
void configurarTiempo() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
    
    // Almacenar en las variables globales
    snprintf(fechaActual, sizeof(fechaActual), "%02d/%02d/%04d", 
             rtc.getDay(), rtc.getMonth() + 1, rtc.getYear());
    snprintf(horaActual, sizeof(horaActual), "%02d:%02d:%02d", 
             rtc.getHour(true), rtc.getMinute(), rtc.getSecond());
    
    Serial.println("Tiempo sincronizado: " + String(rtc.getTime("%d/%m/%Y %H:%M:%S")));
    Serial.print("Fecha almacenada: ");
    Serial.println(fechaActual);
    Serial.print("Hora almacenada: ");
    Serial.println(horaActual);
  } else {
    Serial.println("Error al obtener el tiempo");
  }
}

void handleRoot() {
  server.send(200, "text/html", "<h1>Prueba</h1><p>Servidor web funcionando</p>");
}

void inicializarFechaHora() {
    strcpy(fechaActual, "01/01/2000");
    strcpy(horaActual, "00:00:00");
}

void actualizarFechaHoraDesdeRTC() {
  if (!rtcSincronizado) return; 
  
  snprintf(fechaActual, sizeof(fechaActual), "%02d/%02d/%04d", 
           rtc.getDay(), rtc.getMonth() + 1, rtc.getYear());
  snprintf(horaActual, sizeof(horaActual), "%02d:%02d:%02d", 
           rtc.getHour(true), rtc.getMinute(), rtc.getSecond());
}

bool conectarWiFi() {
  Serial.print("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 5) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado a WiFi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    configurarTiempo();
    
    server.on("/", handleRoot);
    server.begin();
    return true;
  }
  
  Serial.println("\nError conectando");
  return false;
}

void desconectarWiFi() {
  server.stop();
  WiFi.disconnect(true);
  Serial.println("WiFi desconectado");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // sensorTemp.iniciar(); //sin temperatura
  contadorPasos.iniciar();
  miPantalla.iniciar();

  // Inicializar fecha y hora una vez al inicio
  inicializarFechaHora();

  miPantalla.apagar();

  Serial.println("Sistema listo. Pulsa el botón para iniciar.");
}

bool pantallaTrabajando = false;
bool sensorTemperaturaTrabajando = false;
bool sensorAcelerometroTrabajando = true;
int pasos = 0;

void loop() {
  unsigned long ahora = millis();
  bool lectura = digitalRead(BUTTON_PIN);
  bool botonPulsadoAhora = false;

  if (!pantallaTrabajando && !sensorTemperaturaTrabajando) { // controlar!
    contadorPasos.actualizar(); 
    pasos = contadorPasos.getStepCount(); 
  }

  if (lectura != botonPrev && (ahora - ultimoCambioBoton > DEBOUNCE_TIME)) {
    ultimoCambioBoton = ahora;
    botonPrev = lectura;

    if (lectura == LOW) {
      botonPulsadoAhora = true;
    }
  }

  if (lectura == LOW && !botonMantiene) {
    // Se acaba de empezar a mantener pulsado
    botonMantiene = true;
    tiempoInicioHold = ahora;
  }

  if (lectura == HIGH && botonMantiene) {
    // Dejó de mantener pulsado
    botonMantiene = false;
  }

  if (botonMantiene && (ahora - tiempoInicioHold > LONG_PRESS_TIME)) {
    botonMantiene = false;

    // para evitar conflicto entre la segunda pulsación y mantener pulsado
    // se anula la medida de temperatura
    esperandoSegundoClick = false;
    enCuentaAtras = false;
    enCuentaAtrasTemperatura = false;
    pantallaTrabajando = false;
    sensorTemperaturaTrabajando = false;
    sensorAcelerometroTrabajando = true;
    miPantalla.apagar();

    if (!internetConectado) {
      if (conectarWiFi()) {
        internetConectado = true;
        rtcSincronizado = true;
      }
    } else {
      desconectarWiFi();
      internetConectado = false;
    }
  }

  if (botonPulsadoAhora) {

    if (enCuentaAtras) {
      pantallaTrabajando = false;
      sensorTemperaturaTrabajando = false;
      sensorAcelerometroTrabajando = true;
      cancelarTodo();
      return;
    }

    if (!esperandoSegundoClick && !sensorTemperaturaTrabajando) { // controlar
      pantallaTrabajando = true;
      sensorTemperaturaTrabajando = false;
      sensorAcelerometroTrabajando = false;

      actualizarFechaHoraDesdeRTC();
      miPantalla.setDatos(pasos, 10000, fechaActual, horaActual, internetConectado, 38);
      miPantalla.displayPredeterminado();
      Serial.println("Primer click detectado. Tienes 10 segundos para pulsar otra vez.");
      esperandoSegundoClick = true;
      tiempoPrimerClick = ahora;
      return;
    }

    if (esperandoSegundoClick && (ahora - tiempoPrimerClick <= DOUBLECLICK_WINDOW)) {
      pantallaTrabajando = false;
      sensorTemperaturaTrabajando = true;
      sensorAcelerometroTrabajando = false;

      Serial.println("Segundo click detectado. Preparando medición...");
      esperandoSegundoClick = false;

      enCuentaAtras = true;
      inicioCuentaAtras = ahora;
      return;
    }
  }

  if (esperandoSegundoClick && (ahora - tiempoPrimerClick > DOUBLECLICK_WINDOW) && pantallaTrabajando) {
    pantallaTrabajando = false;
    sensorTemperaturaTrabajando = false;
    sensorAcelerometroTrabajando = true;

    miPantalla.apagar();
    Serial.println("Tiempo agotado. Operación cancelada.");
    esperandoSegundoClick = false;
  }

  if (enCuentaAtras) {
    unsigned long restante = COUNTDOWN_TIME - (ahora - inicioCuentaAtras);

    if (restante <= 0) {
      enCuentaAtras = false;

      miPantalla.apagar();
      Serial.println("Midiendo temperatura...");

      // float temp = sensorTemp.medirPromedioBruto(); //sin temperatura
      float temp = 35;

      pantallaTrabajando = true;
      sensorTemperaturaTrabajando = false;
      sensorAcelerometroTrabajando = false;

      enCuentaAtrasTemperatura = true;
      inicioCuentaAtrasTemperatura = ahora;

      if (temp < TEMP_MIN || temp > TEMP_MAX) {
        miPantalla.displayErrorMedida();

        Serial.print("Temperatura fuera de rango (");
        Serial.print(temp);
        Serial.println(" °C). Parece temperatura ambiental.");
      } else {
        miPantalla.displayMedidaCorrecta(temp);

        Serial.print("Temperatura corporal válida: ");
        Serial.print(temp);
        Serial.println(" °C");
      }

    } else {
      static unsigned long lastPrint = 0;
      if (ahora - lastPrint >= 1000) {
        lastPrint = ahora;
        Serial.print("Medición en ");
        Serial.print(restante / 1000);
        Serial.println(" segundos...");
        miPantalla.displayEmpezarMedida(restante / 1000);
      }
    }
  }

  if (enCuentaAtrasTemperatura) {
    unsigned long restanteTemperatura = COUNTDOWN_TIME - (ahora - inicioCuentaAtrasTemperatura);

    if (restanteTemperatura <= 0) {
      enCuentaAtrasTemperatura = false;

      miPantalla.apagar();

      pantallaTrabajando = false;
      sensorTemperaturaTrabajando = false;
      sensorAcelerometroTrabajando = true;   

    }
  }

}
