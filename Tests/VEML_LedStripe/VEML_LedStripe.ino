#include <Wire.h>
#include "Adafruit_VEML7700.h"

Adafruit_VEML7700 veml7700;

const int led = D8;     // Pin donde está conectada la tira de leds

void setup() {
  Serial.begin(9600);

  pinMode(led, OUTPUT);     // Configura el pin como salida

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

  if (lux <= 700) digitalWrite(led, HIGH);
  else digitalWrite(led, LOW);

  delay(1000); // Espera un segundo antes de realizar la próxima lectura
}