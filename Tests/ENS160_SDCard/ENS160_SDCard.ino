#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "ScioSense_ENS160.h"
#include <SPI.h>
#include <SD.h>

Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

#define CS_PIN D8 // Pin donde está conectado el módulo SD
#define umbral_eCO2 1000 // Umbral para el nivel de eCO2 que activa el ventilador
#define NUM_LECTURAS 10 // Número de lecturas para calcular la media

int lectura_count = 0;
float sum_humedad = 0;

void setup() {
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT); // Configurar el pin del módulo SD como salida

  // Inicializar la tarjeta SD
  if (!SD.begin(CS_PIN)) {
    Serial.println("No se pudo inicializar la tarjeta SD!");
    while (1);
  }
  Serial.println("Tarjeta SD inicializada.");

  // Crear el archivo CSV y escribir encabezados si no existe
  if (!SD.exists("/data.csv")) {
    File dataFile = SD.open("/data.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Temperatura, Humedad, AQI, eCO2, Media_Humedad");
      dataFile.close();
    } else {
      Serial.println("Error al crear data.csv");
    }
  }

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
  float temperatura = temp.temperature;
  float humedad_relativa = humedad.relative_humidity;

  // Lectura del sensor ENS160
  if (ens160.available()) {
    ens160.measure(true);
    ens160.measureRaw(true);

    // Almacenar los valores del sensor en variables
    float AQI = ens160.getAQI();
    float eCO2 = ens160.geteCO2();
    float TVOC = ens160.getTVOC();

    // Sumar la humedad actual a la suma total
    sum_humedad += humedad_relativa;
    lectura_count++;

    // Imprimir los valores en el monitor serie
    Serial.print("Temperatura: "); Serial.print(temperatura); Serial.println(" grados C");
    Serial.print("Humedad: "); Serial.print(humedad_relativa); Serial.println("% rH");
    Serial.print("Air Quality Index: "); Serial.print(AQI); Serial.print("\t");
    Serial.print("Equivalent Carbon Dioxide: "); Serial.print(eCO2); Serial.println(" ppm");

    // Escribir los datos en el archivo CSV
    File dataFile = SD.open("/data.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.print(temperatura); dataFile.print(", ");
      dataFile.print(humedad_relativa); dataFile.print(", ");
      dataFile.print(AQI); dataFile.print(", ");
      dataFile.print(eCO2); 
      
      // Calcular la media de la humedad cada 10 lecturas
      if (lectura_count >= NUM_LECTURAS) {
        float media_humedad = sum_humedad / NUM_LECTURAS;
        Serial.print("Media de Humedad: "); Serial.println(media_humedad);
        dataFile.print(", ");
        dataFile.println(media_humedad);

        // Reiniciar contador y suma
        lectura_count = 0;
        sum_humedad = 0;
      } else {
        dataFile.println();
      }

      dataFile.close();
    } else {
      Serial.println("Error al escribir en data.csv");
    }
  } else {
    Serial.println("No se pudo encontrar el sensor ENS160");
  }

  delay(5000); // Espera 5 segundos antes de la próxima lectura
}