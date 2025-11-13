#include <Wire.h>
#include <Adafruit_MLX90614.h>

class SensorTemperaturaMLX90614 {
  private:
    Adafruit_MLX90614 sensor;
    int numMediciones;

    float leerUnaMedicion() {
      return sensor.readObjectTempC(); 
    }

  public:
    SensorTemperaturaMLX90614(int n = 10) { 
      numMediciones = n;
    }

    void iniciar() {
      if (!sensor.begin()) {
        Serial.println("ERROR");
    }

    float medirTemperatura() {
      float suma = 0;
      for (int i = 0; i < numMediciones; i++) {
        suma += leerUnaMedicion();
        delay(500);
      }
      return suma / numMediciones;
    }
};

SensorTemperaturaMLX90614 sensorTemp(10); 

void setup() {
  Serial.begin(115200);
  Wire.begin();
  sensorTemp.iniciar();
}

void loop() {
  Serial.println("Se empieza a medir en 5 segundos. "); 
  /* Comentario: desde que el usuario presiona el botón que empieza a pitar de forma
  intermitente el buzzer unos segundos, que es el tiempo que tiene para ponerse el sensor en la frente.
  Porque sino coge las primeras medidas del ambiente y distorsiona la medida.
  */
  delay(5000);
  Serial.println("Inicio");
  float temperatura = sensorTemp.medirTemperatura();
  Serial.print("Temperatura corporal: ");
  Serial.print(temperatura);
  Serial.println(" °C");
}
