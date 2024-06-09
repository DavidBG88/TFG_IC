#include <ESP8266WiFi.h>
#include <espnow.h>
#include <SD.h>
#include <SPI.h>

// Pines para la tarjeta micro SD
#define SD_CS_PIN D8 // Configura el pin CS según tu conexión

// Estructura para recibir datos
typedef struct struct_message {
    int id;
    float temperatura;
    float humedad;
} struct_message;

// Crear una instancia de la estructura
struct_message myData;

// Variables para almacenar las lecturas y calcular las medias
float temperaturas[10];
float humedades[10];
int lecturaCount = 0;
int maxLecturas = 5;
bool encabezadoEscrito = false;

// Callback function que se ejecutará cuando se reciban datos
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  char macStr[18];
  Serial.print("Paquete recibido de: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("ID de la Placa %u: %u bytes\n", myData.id, len);
  Serial.printf("Temperatura: %.2f \n", myData.temperatura);
  Serial.printf("Humedad: %.2f \n", myData.humedad);
  Serial.println();

  // Abrir el archivo CSV y escribir el encabezado si es necesario
  File dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    if (!encabezadoEscrito) {
      dataFile.println("ID,Temperatura,Humedad");
      encabezadoEscrito = true;
    }
    dataFile.printf("%d,%.2f,%.2f\n", myData.id, myData.temperatura, myData.humedad);
    dataFile.close();
  } else {
    Serial.println("Error abriendo data.csv");
  }

  // Almacenar las lecturas en los arrays
  temperaturas[lecturaCount] = myData.temperatura;
  humedades[lecturaCount] = myData.humedad;
  lecturaCount++;

  // Si se han recibido maxLecturas, calcular y mostrar las medias
  if (lecturaCount == maxLecturas) {
    float sumaTemperatura = 0;
    float sumaHumedad = 0;
    for (int i = 0; i < maxLecturas; i++) {
      sumaTemperatura += temperaturas[i];
      sumaHumedad += humedades[i];
    }
    float mediaTemperatura = sumaTemperatura / maxLecturas;
    float mediaHumedad = sumaHumedad / maxLecturas;
    Serial.printf("MEDIA TEMPERATURA: %.2f \n", mediaTemperatura);
    Serial.printf("MEDIA HUMEDAD: %.2f \n", mediaHumedad);

    // Guardar la media en el archivo CSV
    dataFile = SD.open("media.csv", FILE_WRITE);
    if (dataFile) {
      if (!dataFile.size()) {
        dataFile.println("Media Temperatura,Media Humedad");
      }
      dataFile.printf("%.2f,%.2f\n", mediaTemperatura, mediaHumedad);
      dataFile.close();
    } else {
      Serial.println("Error abriendo media.csv");
    }

    // Reiniciar el contador de lecturas
    lecturaCount = 0;
  }
}

void setup() {
  // Inicializar el monitor serie
  Serial.begin(115200);
  
  // Configurar el pin CS del SD
  pinMode(SD_CS_PIN, OUTPUT);
  
  // Inicializar la tarjeta SD
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Inicialización de la tarjeta SD falló!");
    return;
  }
  Serial.println("Tarjeta SD inicializada.");

  // Configurar el dispositivo como una estación Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Inicializar ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }
  
  // Registrar la función de callback para recibir datos
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // El bucle principal está vacío ya que todo se maneja en el callback OnDataRecv
}
