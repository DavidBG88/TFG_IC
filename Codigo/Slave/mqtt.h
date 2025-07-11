#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <TinyMqtt.h>

// Estructura para enviar datos del esclavo al maestro
typedef struct struct_message {
    int id;
    float temperatura;
    float humedad;
    char timestamp[20];
} struct_message;

// Estructura para los datos del maestro que se envían al esclavo
struct SensorData {
    char timestamp[20];       // Fecha y hora de la medición
    float aht20_temp;       // Temperatura ambiente en grados Celsius
    float aht20_hum;        // Humedad ambiente en porcentaje
    float veml7700;         // Luz medida en lux
    float ens160_aqi;       // Índice de calidad del aire (AQI)
    float ens160_eco2;      // CO2 equivalente en ppm
    float ens160_tvoc;      // Compuestos Orgánicos Volátiles Totales (TVOC) en ppb
    float water_level;      // Nivel de agua del depósito en porcentaje
    float soil_moisture;    // Humedad del suelo en porcentaje
    float fan1_rpm;        // Velocidad del ventilador 1 medida en RPM
    float fan2_rpm;        // Velocidad del ventilador 2 medida en RPM
};

extern SensorData sensordata;

// Declaración de funciones para inicializar y mantener la conexión MQTT
void setupMQTT();  // Inicializa el broker y cliente MQTT
void loopMQTT();   // Mantiene activa la conexión MQTT

// Publica los datos del sensor AHT10 usando struct_message
void publishSensorData(const struct_message& data);

// Publica un comando desde Telegram usando MQTT
void publishCommand(const String& command);

// Callback que se ejecuta cuando se recibe un mensaje en un topic suscrito
void onMessageReceived(const MqttClient* source, const Topic& topic, const char* payload, size_t len);

#endif // MQTT_H