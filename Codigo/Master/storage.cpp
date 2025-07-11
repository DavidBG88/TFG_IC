#include "storage.h"
#include <SD.h>
#include <SPI.h>
#include "sensors.h"
#include "time.h"
#include "weatherapi.h"
#include "utils.h"
#include <ArduinoJson.h>

// Definir los pines de la tarjeta SD para VSPI
#define SD_SCK  18
#define SD_MISO  19
#define SD_MOSI  23
#define SD_CS  5  // Define el pin CS para la tarjeta SD

// Instancia de SPIClass para VSPI
SPIClass spi = SPIClass(VSPI);

void setupStorage() {
    // Inicializar el bus SPI con la instancia de VSPI
    spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    // Inicializar la tarjeta SD
    if (!SD.begin(SD_CS, spi, 1000000)) {
        Serial.println("Error inicializando la tarjeta SD");
        return;
    }

    Serial.println("Tarjeta SD inicializada correctamente");

    // Crear archivos necesarios en la SD si no existen
    createLogFile("/datalog.csv", "Timestamp,Temperature,Humidity,Light,Air Quality Index,Equivalent CO2,Total Volatile Organic Compounds,Water Level,Soil Moisture");
    createLogFile("/activation_log.csv", "Actuator,Duration,Timestamp");
    createLogFile("/weatherlog.csv", "Timestamp,Temperature,Feels_Like,Temp_Min,Temp_Max,Pressure,Humidity,Cloudiness,Wind_Speed,Wind_Direction,Precipitation_Probability,Rain_Volume,Snow_Volume");
    createLogFile("/forecastlog.csv", "Timestamp,Temperature,Feels_Like,Temp_Min,Temp_Max,Pressure,Humidity,Cloudiness,Wind_Speed,Wind_Direction,Precipitation_Probability,Rain_Volume,Snow_Volume");
    createLogFile("/data_slave.csv", "ID,Timestamp,Temperatura,Humedad");
}

void createLogFile(const char* path, const char* header) {
    if (!SD.exists(path)) {
        File dataFile = SD.open(path, FILE_WRITE);
        if (dataFile) {
            Serial.printf("%s creado\n", path);
            dataFile.println(header);
            dataFile.close();
        } else {
            Serial.printf("Error creando %s\n", path);
        }
    } else {
        Serial.printf("%s ya existe\n", path);
    }
}

// Guardar datos de los sensores del maestro en la SD
void saveToSDMasterInput(const SensorData& data) {
    File dataFile = SD.open("/datalog.csv", FILE_APPEND);
    if (dataFile) {
        dataFile.printf("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            data.timestamp,
            data.aht20_temp,
            data.aht20_hum,
            data.veml7700,
            data.ens160_aqi,
            data.ens160_eco2,
            data.ens160_tvoc,
            data.water_level,
            data.soil_moisture,
            data.fan1_rpm,
            data.fan2_rpm);
        dataFile.close();
        Serial.println("Datos de sensores guardados en la tarjeta SD\n");
        printMasterSensorData(data);
    } else {
        Serial.println("Error abriendo /datalog.csv para escribir\n");
    }
}

void saveToSDSlaveInput(const struct_message& data) {
    File dataSlaveFile = SD.open("/data_slave.csv", FILE_APPEND);
    if (dataSlaveFile) {
        dataSlaveFile.printf("%d,%s,%.2f,%.2f\n", data.id, data.timestamp, data.temperatura, data.humedad);
        dataSlaveFile.close();
        Serial.println("Datos del esclavo guardados en la tarjeta SD\n");
        printMQTTSlaveData(data);
    } else {
        Serial.println("Error abriendo /data_slave.csv para escribir\n");
    }
}

void saveActivationLog(String actuator, int duration) {
    File actuatorFile = SD.open("/activation_log.csv", FILE_APPEND);
    if (actuatorFile) {
        String timestamp = getCurrentTime();
        actuatorFile.printf("%s,%d,%s\n", actuator.c_str(), duration, timestamp.c_str());
        actuatorFile.close();
        Serial.println("Activacion guardada en la tarjeta SD");
        printActivationLog(actuator, duration, timestamp);
    } else {
        Serial.println("Error abriendo /activation_log.csv para escribir");
    }
}

void saveWeatherLog(const WeatherAPIData& apidata) {
    File apiDataFile = SD.open("/weatherlog.csv", FILE_APPEND);
    if (apiDataFile) {
        apiDataFile.printf("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                           apidata.request_timestamp, apidata.temp, apidata.feels_like, apidata.temp_min, apidata.temp_max,
                           apidata.pressure, apidata.hum, apidata.cloudiness, apidata.windSpeed, apidata.windDeg,
                           apidata.precProb * 100, apidata.rainVolume, apidata.snowVolume);
        apiDataFile.close();
        Serial.println("Datos API guardados en la tarjeta SD");
        printWeatherData(apidata);
    } else {
        Serial.println("Error abriendo /weatherlog.csv para escribir");
    }
}

void saveForecastLog(const JsonDocument& doc) {
    File forecastDataFile = SD.open("/forecastlog.csv", FILE_WRITE);

    if (forecastDataFile) {
        forecastDataFile.println("Timestamp,Temperature,Feels_Like,Temp_Min,Temp_Max,Pressure,Humidity,Cloudiness,Wind_Speed,Wind_Direction,Precipitation_Probability,Rain_Volume,Snow_Volume");

        for (size_t i = 0; i < doc["list"].size(); i++) {
            JsonObjectConst item = doc["list"][i];

            ForecastAPIData forecast;
            strlcpy(forecast.timestamp, item["dt_txt"], sizeof(forecast.timestamp));
            forecast.temp = item["main"]["temp"];
            forecast.feels_like = item["main"]["feels_like"];
            forecast.temp_min = item["main"]["temp_min"];
            forecast.temp_max = item["main"]["temp_max"];
            forecast.pressure = item["main"]["pressure"];
            forecast.hum = item["main"]["humidity"];
            forecast.cloudiness = item["clouds"]["all"];
            forecast.windSpeed = item["wind"]["speed"];
            forecast.windDeg = item["wind"]["deg"];
            forecast.precProb = item["pop"].isNull() ? 0.0 : item["pop"].as<float>();
            forecast.rainVolume = item["rain"]["3h"].isNull() ? 0.0 : item["rain"]["3h"].as<float>();
            forecast.snowVolume = item["snow"]["3h"].isNull() ? 0.0 : item["snow"]["3h"].as<float>();

            // Guardar en la tarjeta SD
            forecastDataFile.printf("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                forecast.timestamp, forecast.temp, forecast.feels_like, forecast.temp_min, forecast.temp_max,
                forecast.pressure, forecast.hum, forecast.cloudiness, forecast.windSpeed, forecast.windDeg,
                forecast.precProb * 100, forecast.rainVolume, forecast.snowVolume);

            // Imprimir en la terminal
            printForecastData(forecast);
        }

        forecastDataFile.close();
        Serial.println("Datos forecast guardados en la tarjeta SD");
    } else {
        Serial.println("Error abriendo /forecastlog.csv para escribir");
    }
}