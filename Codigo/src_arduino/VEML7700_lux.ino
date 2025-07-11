#include <Wire.h>
#include "Adafruit_VEML7700.h"

Adafruit_VEML7700 veml7700;

void setup() {
  Serial.begin(9600);
  
  if (!veml7700.begin()) {
    Serial.println("No se pudo encontrar el sensor VEML7700");
    while (1);
  }
  
  Serial.println("Sensor VEML7700 encontrado!");
}

void loop() {
  float lux = veml7700.readLux();
  Serial.print("Lux: ");
  Serial.println(lux);
  delay(1000); // Espera un segundo antes de realizar la próxima lectura
}
