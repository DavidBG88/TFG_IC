#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "ScioSense_ENS160.h"

Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

#define VENTILADOR_PIN D6 // Pin donde está conectado el ventilador
#define umbral_eCO2 1000 // Umbral para el nivel de eCO2 que activa el ventilador

void setup() {
  Serial.begin(115200);
  pinMode(VENTILADOR_PIN, OUTPUT); // Configurar el pin del ventilador como salida
  
  // Inicialización del sensor AHT
  if (!aht.begin()) {
    Serial.println("No se pudo encontrar el sensor AHT10");
    while (1) delay(10);
  }
  Serial.println("Sensor AHT10 encontrado");

  // Inicialización del sensor ENS160
  Serial.println("------------------------------------------------------------");
  Serial.println("ENS160 - Digital air quality sensor");
  Serial.println("Sensor readout in standard mode");
  Serial.println("------------------------------------------------------------");
  delay(1000);
  Serial.print("ENS160...");
  ens160.begin();
  Serial.println(ens160.available() ? "done." : "failed!");

  if (ens160.available()) {
    Serial.print("\tRev: "); Serial.print(ens160.getMajorRev());
    Serial.print("."); Serial.print(ens160.getMinorRev());
    Serial.print("."); Serial.println(ens160.getBuild());
  
    Serial.print("\tStandard mode ");
    Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");
  }
}

void loop() {
  // Lectura del sensor AHT
  sensors_event_t humedad, temp;
  aht.getEvent(&humedad, &temp);
  Serial.print("Temperatura: "); Serial.print(temp.temperature); Serial.println(" grados C");
  Serial.print("Humedad: "); Serial.print(humedad.relative_humidity); Serial.println("% rH");

  // Lectura del sensor ENS160
  if (ens160.available()) {
    Serial.println("Sensor ENS160 encontrado!");

    ens160.measure(true);
    ens160.measureRaw(true);
  
    // Almacenar los valores del sensor en variables
    float AQI = ens160.getAQI();
    float eCO2 = ens160.geteCO2();
    float TVOC = ens160.getTVOC();
  
    // Imprimir los valores
    Serial.print("Air Quality Index: "); Serial.print(AQI); Serial.print("\t");
    Serial.print("Equivalent Carbon Dioxide: "); Serial.print(eCO2); Serial.print("ppm\t");
    Serial.print("Total Volatile Organic Compounds: "); Serial.print(TVOC); Serial.print("ppb\t");
    
    // Control del ventilador
    if (AQI > 1 || eCO2 > umbral_eCO2) {
      digitalWrite(VENTILADOR_PIN, HIGH); // Encender el ventilador
      delay(5000);
    } else {
      digitalWrite(VENTILADOR_PIN, LOW); // Apagar el ventilador
      delay(5000);
    }  
  } else {
    Serial.println("No se pudo encontrar el sensor ENS160");
  }
  delay(1000);
}