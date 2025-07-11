#include <SPI.h>
#include <SD.h>

#define CS_PIN D8 // Pin GPIO para el Chip Select (CS)

void setup() {
  Serial.begin(115200);

  // Inicializar el SPI y la tarjeta SD
  if (!SD.begin(CS_PIN)) {
    Serial.println("Inicialización de la tarjeta SD falló!");
    return;
  }
  Serial.println("Tarjeta SD inicializada con éxito.");

  // Crear un archivo y escribir datos en él
  File dataFile = SD.open("/example.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Este es un ejemplo de escritura en la tarjeta SD.");
    dataFile.close();
    Serial.println("Datos escritos en example.txt");
  } else {
    Serial.println("Error al abrir example.txt para escritura.");
  }

  // Leer el archivo y mostrar su contenido
  dataFile = SD.open("/example.txt");
  if (dataFile) {
    Serial.println("Contenido de example.txt:");
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  } else {
    Serial.println("Error al abrir example.txt para lectura.");
  }
}

void loop() {
  // No se necesita nada en el loop para este ejemplo
}

