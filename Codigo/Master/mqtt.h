#ifndef MQTT_H
#define MQTT_H

#include "sensors.h"  // Incluir la definición de SensorData

typedef struct struct_message {
    int id;
    float temperatura;
    float humedad;
    char timestamp[20];
} struct_message;

// Declaración de las funciones de MQTT

// Inicializa el cliente MQTT y las suscripciones
void setupMQTT();

// Publica los datos de los sensores en el topic MQTT, recibe un objeto SensorData
void publishSensorData(const SensorData& sensorData);

// Mantiene la conexión MQTT activa
void loopMQTT();

#endif // MQTT_H