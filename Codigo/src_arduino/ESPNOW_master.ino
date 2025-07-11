#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>

// Definir los pines de la tarjeta SD
#define SD_CS 5

// Estructura de datos para recibir
typedef struct struct_message {
    int id;
    float temperatura;
    float humedad;
} struct_message;

struct_message receivedData;

// Callback function que se ejecutará cuando se reciban datos vía ESP-NOW
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("Datos recibidos -> ID: ");
  Serial.print(receivedData.id);
  Serial.print(", Temperatura: ");
  Serial.print(receivedData.temperatura);
  Serial.print(" °C, Humedad: ");
  Serial.println(receivedData.humedad);

  // Guardar los datos en la tarjeta SD
  File dataFile = SD.open("/data_slave.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.printf("%d,%.2f,%.2f\n", receivedData.id, receivedData.temperatura, receivedData.humedad);
    dataFile.close();
    Serial.println("Datos guardados en la tarjeta SD");
  } else {
    Serial.println("Error abriendo /data_slave.csv para escribir");
  }
}

void setup() {
  Serial.begin(115200);

  // Configurar el dispositivo como estación Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }

  // Registrar la función de callback para recibir datos
  esp_now_register_recv_cb(OnDataRecv);

  // Inicializar la tarjeta SD
  if (!SD.begin(SD_CS)) {
    Serial.println("Error inicializando la tarjeta SD");
    return;
  }

  Serial.println("Tarjeta SD inicializada correctamente");

  // Verificar si el archivo existe, si no, crearlo
  if (!SD.exists("/data_slave.csv")) {
    File dataFile = SD.open("/data_slave.csv", FILE_WRITE);
    if (dataFile) {
      Serial.println("Archivo /data_slave.csv creado");
      dataFile.println("ID,Temperatura,Humedad");
      dataFile.close();
    } else {
      Serial.println("Error creando /data_slave.csv");
    }
  }
}

void loop() {
  // No es necesario hacer nada en el loop, los datos se recibirán y guardarán automáticamente
}