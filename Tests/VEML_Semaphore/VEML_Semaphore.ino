#include <Wire.h>
#include "Adafruit_VEML7700.h"

Adafruit_VEML7700 veml7700;

const int pinRojo = D8;     // Pin donde está conectado el LED rojo
const int pinAmarillo = D7; // Pin donde está conectado el LED amarillo
const int pinVerde = D6;    // Pin donde está conectado el LED verde

void setup() {
  Serial.begin(9600);

  pinMode(pinRojo, OUTPUT);     // Configura el pin como salida
  pinMode(pinAmarillo, OUTPUT); // Configura el pin como salida
  pinMode(pinVerde, OUTPUT);    // Configura el pin como salida

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

  if (lux <= 400) {
    digitalWrite(pinVerde, HIGH);
    digitalWrite(pinAmarillo, LOW);
    digitalWrite(pinRojo, LOW);
  } else if (lux > 400 && lux <= 900){
    digitalWrite(pinVerde, LOW);
    digitalWrite(pinAmarillo, HIGH);
    digitalWrite(pinRojo, LOW);
  } else if (lux > 900) {
    digitalWrite(pinVerde, LOW);
    digitalWrite(pinAmarillo, LOW);
    digitalWrite(pinRojo, HIGH);
  } else {
    digitalWrite(pinVerde, LOW);
    digitalWrite(pinAmarillo, LOW);
    digitalWrite(pinRojo, LOW);
  }

  delay(1000); // Espera un segundo antes de realizar la próxima lectura
}