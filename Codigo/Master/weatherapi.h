#ifndef WEATHERAPI_H
#define WEATHERAPI_H

#include <Arduino.h>

struct WeatherAPIData {
    char request_timestamp[20];  // Timestamp de la solicitud a la API
    float temp;                  // Temperatura API
    float feels_like;            // Sensación térmica
    float temp_min;              // Temperatura mínima
    float temp_max;              // Temperatura máxima
    float pressure;              // Presión atmosférica
    float hum;                   // Humedad API
    float cloudiness;            // Nubosidad
    float windSpeed;             // Velocidad del viento
    float windDeg;               // Dirección del viento
    float precProb;              // Probabilidad de precipitación
    float rainVolume;            // Volumen de lluvia en la última hora
    float snowVolume;            // Volumen de nieve en la última hora
};

struct ForecastAPIData {
    char timestamp[20];
    float temp;
    float feels_like;
    float temp_min;
    float temp_max;
    float pressure;
    float hum;
    float cloudiness;
    float windSpeed;
    float windDeg;
    float precProb;
    float rainVolume;
    float snowVolume;
};

void setupWeatherAPI(const char* configFilePath);
void getWeather(const char* requestTimestamp);
void getForecast();

#endif // WEATHERAPI_H