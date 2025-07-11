#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

struct SensorData {
    char timestamp[20];     // Fecha y hora de la medición
    float aht20_temp;       // Temperatura ambiente en grados Celsius
    float aht20_hum;        // Humedad ambiente en porcentaje
    float veml7700;         // Luz medida en lux
    float ens160_aqi;       // Índice de calidad del aire (AQI)
    float ens160_eco2;      // CO2 equivalente en ppm
    float ens160_tvoc;      // Compuestos Orgánicos Volátiles Totales (TVOC) en ppb
    float water_level;      // Nivel de agua del depósito en porcentaje
    float soil_moisture;    // Humedad del suelo en porcentaje
    float fan1_rpm;         // Velocidad del ventilador 1 medida en RPM (Revoluciones Por Minuto)
    float fan2_rpm;         // Velocidad del ventilador 2 medida en RPM (Revoluciones Por Minuto)
};

extern SensorData sensordata;

void setupSensors();
SensorData readSensors();

#endif // SENSORS_H