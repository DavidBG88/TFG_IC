#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "weatherapi.h"
#include "time.h"
#include "storage.h"
#include "utils.h"
#include "database.h"

const char* currentWeatherServer  = "https://api.openweathermap.org/data/2.5/weather";
const char* forecastWeatherServer = "https://api.openweathermap.org/data/2.5/forecast";
String apiKey, lat, lon;
WiFiClientSecure wifiClientSecure;

void setupWeatherAPI(const char* configFilePath) {
    File configFile = SD.open(configFilePath, FILE_READ);
    if (!configFile) {
        Serial.println("Error abriendo config.txt");
        return;
    }

    while (configFile.available()) {
        String line = configFile.readStringUntil('\n');
        int separatorIndex = line.indexOf('=');
        if (separatorIndex == -1) continue;

        String key = line.substring(0, separatorIndex);
        String value = line.substring(separatorIndex + 1);
        value.trim();

        if (key == "apiKey") {
            apiKey = value;
        } else if (key == "lat") {
            lat = value;
        } else if (key == "lon") {
            lon = value;
        }
    }
    configFile.close();

    printf("API Key: %s\nLatitude: %s\nLongitude: %s\n\n", apiKey.c_str(), lat.c_str(), lon.c_str());

}

void getWeather(const char* requestTimestamp) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        wifiClientSecure.setInsecure();
        String url = String(currentWeatherServer) + "?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey + "&units=metric";

        http.begin(wifiClientSecure, url);
        
        delay(1000);

        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();

            DynamicJsonDocument doc(4096);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            WeatherAPIData apidata;
            strncpy(apidata.request_timestamp, requestTimestamp, sizeof(apidata.request_timestamp) - 1);
            apidata.request_timestamp[sizeof(apidata.request_timestamp) - 1] = '\0';  // Asegurarse de que el string esté terminado en nulo

            apidata.temp = doc["main"]["temp"];
            apidata.feels_like = doc["main"]["feels_like"];
            apidata.temp_min = doc["main"]["temp_min"];
            apidata.temp_max = doc["main"]["temp_max"];
            apidata.pressure = doc["main"]["pressure"];
            apidata.hum = doc["main"]["humidity"];
            apidata.cloudiness = doc["clouds"]["all"];
            apidata.windSpeed = doc["wind"]["speed"];
            apidata.windDeg = doc["wind"]["deg"];
            apidata.precProb = doc["pop"].isNull() ? 0.0 : doc["pop"];
            apidata.rainVolume = doc["rain"]["1h"].isNull() ? 0.0 : doc["rain"]["1h"];
            apidata.snowVolume = doc["snow"]["1h"].isNull() ? 0.0 : doc["snow"]["1h"];

            saveWeatherLog(apidata);
            saveAPIToDataBase(apidata);
        } else {
            Serial.printf("Error en la solicitud HTTP: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    } else {
        Serial.println("WiFi no conectado");
    }
}

void getForecast() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        wifiClientSecure.setInsecure();
        String url = String(forecastWeatherServer) + "?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey + "&units=metric";
        http.begin(wifiClientSecure, url);

        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();
            //Serial.println(payload);

            DynamicJsonDocument doc(8192);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            saveForecastLog(doc);
        } else {
            Serial.printf("Error en la solicitud HTTP: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    } else {
        Serial.println("WiFi no conectado");
    }
}