#include "database.h"
#include "mqtt.h"
#include "weatherapi.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Endpoints del servidor
const char* serverNameMaster = "http://192.168.X.XX:4000/master_data";
const char* serverNameSlave  = "http://192.168.X.XX:4000/slave_data";
const char* serverNameAPI    = "http://192.168.X.XX:4000/api_data";

// Función para enviar los datos del maestro
void saveToDataBase(const SensorData& data) {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conectado a WiFi, preparando para enviar los datos...");

        HTTPClient http;
        http.begin(serverNameMaster);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"timestamp\":\"" + String(data.timestamp) + "\",";
        jsonPayload += "\"aht20_temp\":" + String(data.aht20_temp) + ",";
        jsonPayload += "\"aht20_hum\":" + String(data.aht20_hum) + ",";
        jsonPayload += "\"veml7700\":" + String(data.veml7700) + ",";
        jsonPayload += "\"ens160_aqi\":" + String(data.ens160_aqi) + ",";
        jsonPayload += "\"ens160_eco2\":" + String(data.ens160_eco2) + ",";
        jsonPayload += "\"ens160_tvoc\":" + String(data.ens160_tvoc) + ",";
        jsonPayload += "\"water_level\":" + String(data.water_level) + ",";
        jsonPayload += "\"soil_moisture\":" + String(data.soil_moisture) + ",";
        jsonPayload += "\"fan1_rpm\":" + String(data.fan1_rpm) + ",";
        jsonPayload += "\"fan2_rpm\":" + String(data.fan2_rpm) + "}";

        Serial.printf("Payload JSON: %s\n", jsonPayload.c_str());
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.printf("Datos enviados correctamente. Codigo de respuesta: %d\n", httpResponseCode);
        } else {
            Serial.printf("Error al enviar los datos. Codigo de error: %d\n", httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("Error: No conectado a WiFi");
    }
}

// Función para enviar los datos del esclavo
void saveSlaveToDataBase(const struct_message& data) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverNameSlave);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"id\":" + String(data.id) + ",";
        jsonPayload += "\"temperatura\":" + String(data.temperatura) + ",";
        jsonPayload += "\"humedad\":" + String(data.humedad) + ",";
        jsonPayload += "\"timestamp\":\"" + String(data.timestamp) + "\"}";

        Serial.printf("Payload JSON (slave): %s\n", jsonPayload.c_str());
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.printf("Datos del esclavo enviados correctamente. Codigo de respuesta: %d\n", httpResponseCode);
        } else {
            Serial.printf("Error al enviar los datos del esclavo. Codigo de error: %d\n", httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("Error: No conectado a WiFi");
    }
}

// Función para enviar los datos de la API de clima
void saveAPIToDataBase(const WeatherAPIData& apidata) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverNameAPI);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{";
        jsonPayload += "\"timestamp\":\"" + String(apidata.request_timestamp) + "\",";
        jsonPayload += "\"temp\":" + String(apidata.temp) + ",";
        jsonPayload += "\"feels_like\":" + String(apidata.feels_like) + ",";
        jsonPayload += "\"temp_min\":" + String(apidata.temp_min) + ",";
        jsonPayload += "\"temp_max\":" + String(apidata.temp_max) + ",";
        jsonPayload += "\"pressure\":" + String(apidata.pressure) + ",";
        jsonPayload += "\"humidity\":" + String(apidata.hum) + ",";
        jsonPayload += "\"cloudiness\":" + String(apidata.cloudiness) + ",";
        jsonPayload += "\"wind_speed\":" + String(apidata.windSpeed) + ",";
        jsonPayload += "\"wind_deg\":" + String(apidata.windDeg) + ",";
        jsonPayload += "\"prec_prob\":" + String(apidata.precProb) + ",";
        jsonPayload += "\"rain_volume\":" + String(apidata.rainVolume) + ",";
        jsonPayload += "\"snow_volume\":" + String(apidata.snowVolume);
        jsonPayload += "}";

        Serial.printf("Payload JSON (API): %s\n", jsonPayload.c_str());
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.printf("Datos de la API enviados correctamente. Codigo de respuesta: %d\n", httpResponseCode);
        } else {
            Serial.printf("Error al enviar los datos de la API. Codigo de error: %d\n", httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("Error: No conectado a WiFi");
    }
}