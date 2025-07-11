#include "sensors.h"
#include "actuators.h"
#include "storage.h"
#include "mode.h"
#include "database.h"
#include <TinyMqtt.h>
#include <ArduinoJson.h>

// Configuración del MQTT
MqttClient mqttClient("ESP32_Maestro");

extern SensorData sensordata;

// Función de callback para manejar mensajes recibidos
void onMQTTMessageReceived(const MqttClient* source, const Topic& topic, const char* payload, size_t payload_length) {
    // Convertir el payload a un string
    String message(payload, payload_length);

    // Convertir el topic a string para la comparación
    String receivedTopic = topic.c_str();

    if (receivedTopic == "broker/data") {
        // Procesar los datos de sensores enviados por el esclavo
        Serial.println("Datos de sensores recibidos del esclavo:");
        Serial.println(message);

        // Crear un objeto JSON para deserializar el payload
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.print("Error al deserializar JSON: ");
            Serial.println(error.c_str());
            return;
        }

        // Parsear los datos JSON a la estructura struct_message
        struct_message receivedData;
        receivedData.id = doc["id"]; 
        receivedData.temperatura = doc["temperatura"];
        receivedData.humedad = doc["humedad"];
        strlcpy(receivedData.timestamp, doc["timestamp"], sizeof(receivedData.timestamp));

        // Guardar los datos recibidos en la SD
        saveToSDSlaveInput(receivedData);
        // Guardar los datos en la base de datos
        saveSlaveToDataBase(receivedData);

        // Imprimir los datos recibidos para verificar
        Serial.println("Datos del esclavo guardados en la tarjeta SD:");
        Serial.printf("ID: %d\n", receivedData.id);
        Serial.printf("Temperatura: %.2f\n", receivedData.temperatura);
        Serial.printf("Humedad: %.2f\n", receivedData.humedad);
        Serial.printf("Timestamp: %s\n", receivedData.timestamp);

    } else if (receivedTopic == "broker/commands") {
        // Procesar los comandos recibidos desde Telegram
        if (message == "predefinido") {
            systemMode = true;
            Serial.println("Modo predefinido activado por comando MQTT");
        } else if (message == "sleep") {
            systemMode = false;
            Serial.println("Modo sleep para ahorro de energía activado por comando MQTT");
        } else if (message.startsWith("activate")) {
            // Comando para activar actuador (ejemplo: "activate pump 20")
            char actuator[20];
            int duration;
            sscanf(message.c_str(), "activate %s %d", actuator, &duration);
            activateActuatorTelegram(String(actuator), duration);
        } else if (message == "read_sensors") {
            Serial.println("Comando 'read_sensors' recibido, enviando datos de sensores...");

            // Enviar los datos de los sensores almacenados
            publishSensorData(sensordata);
        } else {
            Serial.println("Comando no reconocido");
        }
    }
}

// Función para reconectar al broker MQTT
void reconnectMQTT() {
    while (!mqttClient.connected()) {
        Serial.println("Intentando conectar al broker MQTT...");

        mqttClient.connect("192.168.X.XX", 1883);  // IP del broker (ESP32 esclavo)

        if (mqttClient.connected()) {
            Serial.println("Conectado al broker MQTT");

            // Configurar callback para mensajes recibidos
            mqttClient.setCallback(onMQTTMessageReceived);

            // Suscribirse a los topics importantes
            MqttError err = mqttClient.subscribe("broker/data");
            if (err == MqttOk) {
                Serial.println("Suscripcion al topic 'broker/data' exitosa");
            } else {
                Serial.printf("Error al suscribirse al topic 'broker/data': %d\n", err);
            }

            err = mqttClient.subscribe("broker/commands");
            if (err == MqttOk) {
                Serial.println("Suscripcion al topic 'broker/commands' exitosa");
            } else {
                Serial.printf("Error al suscribirse al topic 'broker/commands': %d\n", err);
            }
        } else {
            Serial.println("Error al conectar al broker MQTT. Reintentando en 5 segundos...");
            delay(5000);
        }
    }
}

// Inicialización de MQTT
void setupMQTT() {
    // Intentar la conexión inicial al broker
    reconnectMQTT();
}

// Función para publicar los datos de los sensores usando JSON
void publishSensorData(const SensorData& sensorData) {
    // Formatear los datos de los sensores en un buffer JSON, incluyendo el timestamp
    char payload[300];
    snprintf(payload, sizeof(payload),
        "{\"timestamp\":\"%s\",\"aht20_temp\":%.2f,\"aht20_hum\":%.2f,\"veml7700\":%.2f,"
        "\"ens160_aqi\":%.2f,\"ens160_eco2\":%.2f,\"ens160_tvoc\":%.2f,"
        "\"water_level\":%.2f,\"soil_moisture\":%.2f,\"fan1_rpm\":%.2f,\"fan2_rpm\":%.2f}",
        sensorData.timestamp,
        sensorData.aht20_temp,
        sensorData.aht20_hum,
        sensorData.veml7700,
        sensorData.ens160_aqi,
        sensorData.ens160_eco2,
        sensorData.ens160_tvoc,
        sensorData.water_level,
        sensorData.soil_moisture,
        sensorData.fan1_rpm,
        sensorData.fan2_rpm);


    // Publicar los datos de los sensores al topic 'master/data'
    MqttError err = mqttClient.publish("master/data", payload, true);
    if (err == MqttOk) {
        Serial.println("Datos de sensores publicados correctamente en 'master/data'");
    } else {
        Serial.printf("Error al publicar los datos: %d\n", err);
    }
}

// Función para mantener la conexión con el broker activa
void loopMQTT() {
    mqttClient.loop();
}