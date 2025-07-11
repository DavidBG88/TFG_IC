#include "aht10.h"
#include "mqtt.h"

// Instancia del sensor AHT10
Adafruit_AHTX0 aht;

// Instancia de la estructura de mensaje
struct_message sensorMessage;

void setupSensor() {
    if (!aht.begin()) {
        Serial.println("No se pudo encontrar el sensor AHT10");
        while (1) delay(10);
    }
    Serial.println("Sensor AHT10 encontrado");
}

// Función para leer los datos del sensor y prepararlos para enviar
struct_message readSensor(const char* timestamp) {
    struct_message data;

    // Leer los datos del sensor
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    data.id = 1;  // ID del dispositivo o sensor
    data.temperatura = temp.temperature;
    data.humedad = humidity.relative_humidity;

    // Asignar el timestamp recibido
    strncpy(data.timestamp, timestamp, sizeof(data.timestamp) - 1);
    data.timestamp[sizeof(data.timestamp) - 1] = '\0';  // Asegurar que el string esté terminado en nulo

    // Imprimir los datos en la terminal
    Serial.printf("Datos del sensor -> Temperatura: %.2f °C, Humedad: %.2f%%, Timestamp: %s\n", 
        data.temperatura, data.humedad, data.timestamp);

    return data;  // Devolver los datos
}