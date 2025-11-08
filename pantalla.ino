#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANCHO 128
#define ALTO 64
#define OLED_RESET -1

Adafruit_SSD1306 pantalla(ANCHO, ALTO, &Wire, OLED_RESET);

class PantallaOLED {
  private:
    int pasos, meta;
    char buffer_fecha[11];  // DD/MM/AAAA + '\0'
    char buffer_hora[9];    // HH:MM:SS + '\0'
    bool wifi;
    float temperatura, umbral;

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
      pantalla.print("HORA:  "); pantalla.println(buffer_hora);
      pantalla.println("--------------------");
      pantalla.print("WIFI:  "); pantalla.println(wifi ? "ON" : "OFF");
      pantalla.display();
    }

    void displayEnhorabuena() {
      encender();
      pantalla.setCursor(0, 0);

      pantalla.println("*** ENHORABUENA ***");
      pantalla.println();
      pantalla.println("META DE PASOS LOGRADA");
      pantalla.println();
      
      pantalla.print(pasos);
      pantalla.println(" PASOS");
      
      pantalla.println();
      pantalla.println("--------------------");
      pantalla.println("¡OBJETIVO OK!");

      pantalla.display();
    }

    void setDatos(int p, int m, const char* fecha, const char* hora, bool w) {
      pasos = p;
      meta = m;
      wifi = w;

      strncpy(buffer_fecha, fecha, sizeof(buffer_fecha) - 1);
      buffer_fecha[sizeof(buffer_fecha) - 1] = '\0';

      strncpy(buffer_hora, hora, sizeof(buffer_hora) - 1);
      buffer_hora[sizeof(buffer_hora) - 1] = '\0';
    }
};

PantallaOLED miPantalla;

void setup() {
  Serial.begin(115200);
  miPantalla.iniciar();
}

void loop() {
  miPantalla.setDatos(4720, 10000, "20/10/2025", "19:48:23", true);
  miPantalla.displayPredeterminado();
  delay(3000);
  miPantalla.displayEnhorabuena();
  delay(3000);
  miPantalla.apagar();
  delay(3000);
}