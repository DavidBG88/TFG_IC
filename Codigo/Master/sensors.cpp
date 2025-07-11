#include "sensors.h"
#include <Adafruit_VEML7700.h>
#include <Adafruit_AHTX0.h>
#include "ScioSense_ENS160.h"
#include <Arduino.h>
#include <Wire.h>
#include "time.h"
#include "storage.h"

// Definir los pines de los sensores
#define WATER_LEVEL_PIN 34  // Pin ADC1_CH6
#define SOIL_MOISTURE_PIN 35 // Pin ADC1_CH7

// Dirección I2C del ENS160 configurada según el pin ADD
#define ENS160_I2C_ADDRESS 0x53

// Definir los pines I2C para ESP32
#define SDA_PIN 21
#define SCL_PIN 22

const int water_AirValue = 0;   // Valor del sensor de nivel de agua en aire
const int water_WaterValue = 620;  // Valor del sensor de nivel de agua sumergido al completo
const int soil_AirValue = 2600;   // Valor del sensor de humedad del terreno en aire
const int soil_WaterValue = 1100;  // Valor del sensor de humedad del terreno en agua

// Definición de la variable global sensordata, que almacena los datos de los sensores
SensorData sensordata;

// Crear instancias de sensores
Adafruit_AHTX0 aht;
Adafruit_VEML7700 veml = Adafruit_VEML7700();
ScioSense_ENS160 ens160(ENS160_I2CADDR_0);

void setupSensors() {
    // Inicializar bus I2C
    Wire.begin(SDA_PIN, SCL_PIN);

    // Inicializar AHT10/AHT20
    int retryCount = 0;
    while (!aht.begin() && retryCount < 5) {
        Serial.println("No se pudo inicializar AHT20, reintentando...");
        delay(1000);
        retryCount++;
    }
    if (retryCount == 5) {
        Serial.println("Fallo critico: No se pudo inicializar AHT20.");
    } else {
        Serial.println("Sensor AHT20 inicializado");
    }

    // Retraso para asegurar que el bus I2C esté estable
    delay(100);

    // Inicializar ENS160
    retryCount = 0;  // Reiniciar el contador de reintentos
    while (!ens160.begin() && retryCount < 5) {
        Serial.println("No se pudo inicializar ENS160, reintentando...");
        delay(1000);
        retryCount++;
    }
    if (retryCount == 5) {
        Serial.println("Fallo critico: No se pudo inicializar ENS160.");
    } else {
        Serial.println("Sensor ENS160 inicializado");
        Serial.print("\tRev: ");
        Serial.print(ens160.getMajorRev());
        Serial.print(".");
        Serial.print(ens160.getMinorRev());
        Serial.print(".");
        Serial.println(ens160.getBuild());
    
        Serial.print("\tStandard mode ");
        if (ens160.setMode(ENS160_OPMODE_STD)) {
            Serial.println("done.");
        } else {
            Serial.println("failed!");
        }
    }

    // Inicializar VEML7700
    retryCount = 0;
    while (!veml.begin() && retryCount < 5) {
        Serial.println("No se pudo inicializar VEML7700, reintentando...");
        delay(1000);
        retryCount++;
    }
    if (retryCount == 5) {
        Serial.println("Fallo crítico: No se pudo inicializar VEML7700");
    } else {
        Serial.println("Sensor VEML7700 inicializado\n");
    }
    
    // Configurar pines de los sensores analógicos
    pinMode(WATER_LEVEL_PIN, INPUT);
    pinMode(SOIL_MOISTURE_PIN, INPUT);
}

SensorData readSensors() {

    // Asignar el timestamp de forma segura
    String currentTime = getCurrentTime();  // Obtener la fecha y hora actuales
    strncpy(sensordata.timestamp, currentTime.c_str(), sizeof(sensordata.timestamp) - 1);
    sensordata.timestamp[sizeof(sensordata.timestamp) - 1] = '\0';  // Asegurar el terminador nulo

    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
        sensordata.aht20_temp = temp.temperature;
        sensordata.aht20_hum = humidity.relative_humidity;
    } else {
        // Suponer valores normales si el sensor falla
        sensordata.aht20_temp = 25.0;
        sensordata.aht20_hum = 50.0;
    }

    sensordata.veml7700 = veml.readLux();  // Leer luz ambiente

    if (ens160.available()) {
        ens160.measure(true); // Iniciar medición del ENS160
        ens160.measureRaw(true); // Medición RAW
        sensordata.ens160_aqi = ens160.getAQI();
        sensordata.ens160_eco2 = ens160.geteCO2();
        sensordata.ens160_tvoc = ens160.getTVOC();
    } else {
        // Suponer valores normales si el sensor falla
        sensordata.ens160_aqi = 0;
        sensordata.ens160_eco2 = 400;
        sensordata.ens160_tvoc = 0;
    }

    // Leer y mapear a porcentaje el nivel de agua
    sensordata.water_level = analogRead(WATER_LEVEL_PIN); 
    sensordata.water_level = map(sensordata.water_level, water_AirValue, water_WaterValue, 0, 100);

    // Asegurar que el valor mapeado esté dentro de 0-100%
    sensordata.water_level = constrain(sensordata.water_level, 0, 100);

    // Leer y mapear a porcentaje la humedad del terreno
    sensordata.soil_moisture = analogRead(SOIL_MOISTURE_PIN);
    sensordata.soil_moisture = map(sensordata.soil_moisture, soil_AirValue, soil_WaterValue, 0, 100);

    // Asegurar que el valor mapeado esté dentro de 0-100%
    sensordata.soil_moisture = constrain(sensordata.soil_moisture, 0, 100);
    
    return sensordata;
}