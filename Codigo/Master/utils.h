#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "sensors.h"
#include "mqtt.h"
#include "weatherapi.h"

// Declaración de las constantes de configuración NTP
extern const char* ntpServer0;
extern const char* ntpServer1;
extern const char* ntpServer2;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// Declaración de las funciones para manejo de la hora y fecha
void setupTime();
String getCurrentTime();

//
void readWiFiCredentials();
void setupWiFi();

// Declaración de las funciones para imprimir datos en la terminal con formato
void printMasterSensorData(const SensorData& data);
void printMQTTSlaveData(const struct_message& data);
void printActivationLog(const String& actuator, int duration, const String& timestamp);
void printWeatherData(const WeatherAPIData& apidata);
void printForecastData(const ForecastAPIData& forecast);

#endif // UTILS_H
