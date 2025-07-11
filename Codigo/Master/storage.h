#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include "sensors.h"
#include "weatherapi.h"
#include <ArduinoJson.h>
#include "utils.h"

// Declaración de las funciones para manejo de almacenamiento
void setupStorage();
void saveToSDMasterInput(const SensorData& data);
void saveToSDSlaveInput(const struct_message& data);
void saveActivationLog(String actuator, int duration);
void saveWeatherLog(const WeatherAPIData& apidata);
void saveForecastLog(const JsonDocument& doc);

// Función para crear y verificar archivos de registro en la tarjeta SD
void createLogFile(const char* filename, const char* header);

// Función para obtener la hora actual
String getCurrentTime();

#endif // STORAGE_H
