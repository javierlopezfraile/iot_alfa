#include <Wire.h>
#include <MPU6050_light.h>

class StepCounter {
  private:
    MPU6050 mpu;
    volatile unsigned long stepCount;
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
          printNoStepData(ax, ay, az, mag);
      }
    }

    unsigned long getStepCount() {
      return stepCount;
    }
};


StepCounter contadorPasos(Wire, 0.25, 500); 

void setup() {
  Serial.begin(115200);
  contadorPasos.iniciar();
}

void loop() {
  contadorPasos.actualizar();
}
