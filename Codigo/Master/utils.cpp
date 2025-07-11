#include <WiFi.h>
#include <SD.h>
#include <Arduino.h>

#include "utils.h"
#include "sensors.h"
#include "mqtt.h"
#include "weatherapi.h"

// Variables globales para las credenciales de WiFi y el servidor MQTT
String ssid;
String password;
String mqtt_server;

// Función para leer las credenciales de WiFi y el servidor MQTT desde un archivo de configuración en la tarjeta SD
void readWiFiCredentials() {
    File file = SD.open("/config.txt");
    if (!file) {
        Serial.println("Failed to open config.txt");
        return;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        int separatorIndex = line.indexOf('=');
        if (separatorIndex > 0) {
            String key = line.substring(0, separatorIndex);
            String value = line.substring(separatorIndex + 1);
            value.trim();

            if (key == "SSID") {
                ssid = value;
            } else if (key == "PASSWORD") {
                password = value;
            } else if (key == "MQTT_SERVER") {
                mqtt_server = value;
            }
        }
    }
    file.close();
}

// Función para conectar a WiFi
void setupWiFi() {
    readWiFiCredentials();
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid.c_str(), password.c_str());

    // Intentar conectarse hasta 10 veces con un delay de 1 segundo entre intentos
    for (int i = 0; i < 10; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to WiFi");
            return;
        }
        delay(1000);
        Serial.print(".");
    }

    // Si no se conecta después de 10 intentos, imprimir mensaje de error
    Serial.println("\nFailed to connect to WiFi");
}

// Función para obtener la fecha y hora actual en formato "HH:MM:SS DD/MM/YYYY"
String getCurrentTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Error al obtener la fecha y hora");
        return "00:00:00 00/00/0000";  // Valor por defecto si no se obtiene la hora
    }
    char timeString[64];
    strftime(timeString, sizeof(timeString), "%H:%M:%S %d/%m/%Y", &timeinfo);
    return String(timeString);
}

// Zona horaria y servidores NTP para España
const char* ntpServer0 = "0.es.pool.ntp.org";
const char* ntpServer1 = "1.es.pool.ntp.org";
const char* ntpServer2 = "2.es.pool.ntp.org";
const long gmtOffset_sec = 3600;  // Offset de GMT para España (GMT+1)
const int daylightOffset_sec = 3600; // Horario de verano

// Función para configurar la hora utilizando NTP
void setupTime() {
    for (int i = 0; i < 5; i++) { // Reintenta hasta 5 veces
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer0, ntpServer1, ntpServer2);
        delay(2000); // Espera 2 segundos antes de reintentar
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            Serial.println("Hora obtenida con exito");
            return; // Sale de la función si la hora se obtiene correctamente
        }
        Serial.println("Error al obtener la hora, reintentando...");
    }
    Serial.println("No se pudo obtener la hora despues de varios intentos");
}

// Imprime los datos del sensor maestro en el monitor serial.
void printMasterSensorData(const SensorData& data) {
    Serial.println("*****************************************************");
    Serial.println("**                 Master Sensor Data              **");
    Serial.println("*****************************************************");
    Serial.printf("*  Timestamp: %s\n", data.timestamp);
    Serial.printf("*  Temperature: %.2f °C\n", data.aht20_temp);
    Serial.printf("*  Humidity: %.2f %%\n", data.aht20_hum);
    Serial.printf("*  Light: %.2f lux\n", data.veml7700);
    Serial.printf("*  Air Quality Index: %.2f\n", data.ens160_aqi);
    Serial.printf("*  Equivalent CO2: %.2f ppm\n", data.ens160_eco2);
    Serial.printf("*  Total VOC: %.2f ppb\n", data.ens160_tvoc);
    Serial.printf("*  Water Level: %.2f %%\n", data.water_level);
    Serial.printf("*  Soil Moisture: %.2f %%\n", data.soil_moisture);
    Serial.printf("*  Fan 1 Speed: %.2f RPM\n", data.fan1_rpm);
    Serial.printf("*  Fan 2 Speed: %.2f RPM\n", data.fan2_rpm);
    Serial.println("*****************************************************\n");
}

// Imprime los datos recibidos del esclavo MQTT en el monitor serial.
void printMQTTSlaveData(const struct_message& data) {
    Serial.println("*****************************************************");
    Serial.println("**               MQTT Data Received from Slave     **");
    Serial.println("*****************************************************");
    Serial.printf("*  ID: %d\n", data.id);
    Serial.printf("*  Temperature: %.2f °C\n", data.temperatura);
    Serial.printf("*  Humidity: %.2f %%\n", data.humedad);
    Serial.printf("*  Timestamp: %s\n", data.timestamp);
    Serial.println("*****************************************************\n");
}

// Imprime el registro de activación del actuador en el monitor serial.
void printActivationLog(const String& actuator, int duration, const String& timestamp) {
    Serial.println("*****************************************************");
    Serial.println("**               Actuator Activation Log           **");
    Serial.println("*****************************************************");
    Serial.printf("*  Actuator: %s\n", actuator.c_str());
    Serial.printf("*  Duration: %d seconds\n", duration);
    Serial.printf("*  Timestamp: %s\n", timestamp.c_str());
    Serial.println("*****************************************************\n");
}

// Imprime los datos del clima recibidos de la API en el monitor serial.
void printWeatherData(const WeatherAPIData& apidata) {
    Serial.println("*****************************************************");
    Serial.println("**               API Weather Data                  **");
    Serial.println("*****************************************************");
    Serial.printf("*  Timestamp: %s\n", apidata.request_timestamp);
    Serial.printf("*  Temperature: %.2f °C\n", apidata.temp);
    Serial.printf("*  Feels Like: %.2f °C\n", apidata.feels_like);
    Serial.printf("*  Min Temp: %.2f °C\n", apidata.temp_min);
    Serial.printf("*  Max Temp: %.2f °C\n", apidata.temp_max);
    Serial.printf("*  Pressure: %.2f hPa\n", apidata.pressure);
    Serial.printf("*  Humidity: %.2f %%\n", apidata.hum);
    Serial.printf("*  Cloudiness: %.2f %%\n", apidata.cloudiness);
    Serial.printf("*  Wind Speed: %.2f m/s\n", apidata.windSpeed);
    Serial.printf("*  Wind Direction: %.2f°\n", apidata.windDeg);
    Serial.printf("*  Precipitation Probability: %.2f %%\n", apidata.precProb * 100);
    Serial.printf("*  Rain Volume: %.2f mm\n", apidata.rainVolume);
    Serial.printf("*  Snow Volume: %.2f mm\n", apidata.snowVolume);
    Serial.println("*****************************************************\n");
}

// Imprime los datos de pronóstico del clima recibidos de la API en el monitor serial.
void printForecastData(const ForecastAPIData& forecast) {
    Serial.println("*****************************************************");    
    Serial.println("**               API Forecast Data                 **");
    Serial.println("*****************************************************");
    Serial.printf("*  Timestamp: %s\n", forecast.timestamp);
    Serial.printf("*  Temperature: %.2f °C\n", forecast.temp);
    Serial.printf("*  Feels Like: %.2f °C\n", forecast.feels_like);
    Serial.printf("*  Min Temp: %.2f °C\n", forecast.temp_min);
    Serial.printf("*  Max Temp: %.2f °C\n", forecast.temp_max);
    Serial.printf("*  Pressure: %.2f hPa\n", forecast.pressure);
    Serial.printf("*  Humidity: %.2f %%\n", forecast.hum);
    Serial.printf("*  Cloudiness: %.2f %%\n", forecast.cloudiness);
    Serial.printf("*  Wind Speed: %.2f m/s\n", forecast.windSpeed);
    Serial.printf("*  Wind Direction: %.2f°\n", forecast.windDeg);
    Serial.printf("*  Precipitation Probability: %.2f %%\n", forecast.precProb * 100);
    Serial.printf("*  Rain Volume: %.2f mm\n", forecast.rainVolume);
    Serial.printf("*  Snow Volume: %.2f mm\n", forecast.snowVolume);
    Serial.println("*****************************************************\n");
}