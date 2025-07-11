#include <TinyMqtt.h>

#include <WiFi.h>

// Configuración de la red WiFi
const char* ssid = "X";
const char* password = "X";

WiFiServer server(1883);  // Crear el servidor WiFi en el puerto 1883
MqttBroker broker(server);  // Instanciar el broker usando el servidor

void setup() {
  Serial.begin(115200);

  // Conectar a WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a la red WiFi");

  // Iniciar el servidor y el broker MQTT
  server.begin();
  broker.begin();
  Serial.println("Broker MQTT iniciado en el puerto 1883");
}

void loop() {
  // Ejecutar el broker MQTT
  Serial.println(WiFi.localIP());
  broker.loop();
  delay(1000);
}