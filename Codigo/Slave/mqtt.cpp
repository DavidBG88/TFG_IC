#include <Arduino.h>
#include <TinyMqtt.h>
#include <WiFi.h>
#include "aht10.h"
#include "mqtt.h"
#include "telegrambot.h"
#include <ArduinoJson.h>

#define TEMP_MAX 28.0
#define SOIL_MOISTURE_MIN 30.0

// Configuración de la red WiFi
const char* ssid = "X";
const char* password = "X";

// Definición de la variable sensordata para almacenar datos de sensores
SensorData sensordata;

const uint16_t PORT = 1883;
const uint8_t RETAIN = 10;  // Máximo número de mensajes retenidos

// Crear el broker MQTT
MqttBroker broker(PORT, RETAIN);

// Crear el cliente MQTT
MqttClient mqttClient("ESP32_Esclavo");

#include <ArduinoJson.h>

// Función de callback para recibir mensajes
void onMessageReceived(const MqttClient* source, const Topic& topic, const char* payload, size_t payload_length) {
    String topicStr = topic.c_str();  // Convertir Topic a String
    Serial.printf("Mensaje recibido en el topic '%s': %s\n", topicStr.c_str(), payload);

    if (topicStr == "master/data") {
        // Crear un objeto JSON para deserializar el payload
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.print("Error al deserializar JSON: ");
            Serial.println(error.c_str());
            return;
        }

        // Actualizar los datos en la estructura sensordata
        strlcpy(sensordata.timestamp, doc["timestamp"], sizeof(sensordata.timestamp));
        sensordata.aht20_temp    = doc["aht20_temp"]    | 0.0;
        sensordata.aht20_hum     = doc["aht20_hum"]     | 0.0;
        sensordata.veml7700      = doc["veml7700"]      | 0.0;
        sensordata.ens160_aqi    = doc["ens160_aqi"]    | 0.0;
        sensordata.ens160_eco2   = doc["ens160_eco2"]   | 0.0;
        sensordata.ens160_tvoc   = doc["ens160_tvoc"]   | 0.0;
        sensordata.water_level   = doc["water_level"]   | 0.0;
        sensordata.soil_moisture = doc["soil_moisture"] | 0.0;
        sensordata.fan1_rpm = doc["fan1_rpm"].isNull() ? 0.0f : doc["fan1_rpm"].as<float>();
        sensordata.fan2_rpm = doc["fan2_rpm"].isNull() ? 0.0f : doc["fan2_rpm"].as<float>();

        Serial.println("Datos del maestro actualizados en sensordata:");

        // Imprimir los datos actualizados para verificar
        Serial.printf("Temperatura: %.2f °C\n", sensordata.aht20_temp);
        Serial.printf("Humedad: %.2f %%\n", sensordata.aht20_hum);
        Serial.printf("Luz: %.2f lux\n", sensordata.veml7700);
        Serial.printf("AQI: %.2f\n", sensordata.ens160_aqi);
        Serial.printf("CO2: %.2f ppm\n", sensordata.ens160_eco2);
        Serial.printf("TVOC: %.2f ppb\n", sensordata.ens160_tvoc);
        Serial.printf("Nivel de agua: %.2f %%\n", sensordata.water_level);
        Serial.printf("Humedad del suelo: %.2f %%\n", sensordata.soil_moisture);
        Serial.printf("Ventilador 1 RPM: %.2f\n", sensordata.fan1_rpm);
        Serial.printf("Ventilador 2 RPM: %.2f\n", sensordata.fan2_rpm);
        Serial.printf("Timestamp: %s\n", sensordata.timestamp);

        if (sensordata.aht20_temp > TEMP_MAX) {
                    sendTelegramAlert("¡Alerta! Temperatura elevada detectada: " + String(sensordata.aht20_temp) + " °C");
        }
        if (sensordata.soil_moisture < SOIL_MOISTURE_MIN) {
            sendTelegramAlert("¡Alerta! Humedad del sustrato baja: " + String(sensordata.soil_moisture) + " %");
        }

        // Llamar a una función que maneje la publicación de estos datos al bot de Telegram
        publishSensorDataTelegram(sensordata);
    }
}

void setupMQTT() {
  // Conectar a WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.print("Conectado a la red WiFi con IP: ");
  Serial.println(WiFi.localIP());

  // Iniciar el broker MQTT
  broker.begin();
  Serial.print("Broker MQTT iniciado en la IP: ");
  Serial.print(WiFi.localIP());
  Serial.println(" en el puerto 1883");

  // Conectar como cliente MQTT al broker local (este mismo dispositivo)
  mqttClient.connect("127.0.0.1", PORT);

  if (mqttClient.connected()) {
    Serial.println("Cliente MQTT conectado al broker");

    // Establecer el callback para mensajes
    mqttClient.setCallback(onMessageReceived);

    // Intentar suscribirse al topic
    MqttError err = mqttClient.subscribe("master/data");
    if (err == MqttOk) {
      Serial.println("Suscripcion al topic 'master/data' exitosa");
    } else {
      Serial.printf("Error al suscribirse al topic 'master/data': %d\n", err);
    }
  } else {
    Serial.println("Error al conectar al broker MQTT");
  }
}

// Función para mantener la conexión con el broker activa
void loopMQTT() {
    broker.loop();    // Ejecutar el loop del broker MQTT
    mqttClient.loop(); // Mantener la conexión del cliente MQTT

    // Verificar el estado de la conexión
    static bool wasConnected = false; // Variable para verificar cambios en el estado de conexión

    if (mqttClient.connected()) {
        if (!wasConnected) {
            // Solo imprime si acaba de conectarse
            Serial.println("Cliente ESP32_Esclavo conectado al broker MQTT");
            wasConnected = true; // Cambiar el estado
        }
    } else {
        if (wasConnected) {
            // Solo imprime si acaba de desconectarse
            Serial.println("Cliente ESP32_Esclavo desconectado del broker MQTT");
            wasConnected = false; // Cambiar el estado
        }
    }
}

// Publicar los datos del sensor (estructura struct_message) en formato JSON
void publishSensorData(const struct_message& data) {
  // Convertir la estructura a formato JSON para enviar como payload MQTT
  char payload[100];
  snprintf(payload, sizeof(payload), "{\"id\":%d,\"temperatura\":%.2f,\"humedad\":%.2f,\"timestamp\":\"%s\"}",
             data.id, data.temperatura, data.humedad, data.timestamp);

  // Publicar los datos en el topic "broker/data"
  MqttError err = mqttClient.publish("broker/data", payload, true);
  if (err == MqttOk) {
    Serial.println("Datos del sensor publicados correctamente en 'broker/data'");
  } else {
    Serial.printf("Error al publicar datos: %d\n", err);
  }
}

// Función para publicar comandos desde Telegram usando MQTT
void publishCommand(const String& command) {
    // Publicar el comando en el topic de comandos
    MqttError err = mqttClient.publish("broker/commands", command.c_str(), true); // Usar retain true para guardar el mensaje
    if (err == MqttOk) {
        Serial.printf("Comando '%s' publicado correctamente en 'broker/commands'\n", command.c_str());
    } else {
        Serial.printf("Error al publicar comando '%s': %d\n", command.c_str(), err);
    }
}