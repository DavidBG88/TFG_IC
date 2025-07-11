#include <Arduino.h>

#include "mode.h"
#include "sensors.h"
#include "actuators.h"
#include "storage.h"
#include "weatherapi.h"
#include "mqtt.h"
#include "database.h"

bool systemMode = true;
int lastSwitchState = HIGH;

extern SensorData sensordata;

// Función para verificar el estado del interruptor SPDT
void checkSwitch() {
    int switchState = digitalRead(switchPin);
    if (switchState != lastSwitchState) {
        lastSwitchState = switchState;
        if (switchState == HIGH) {
            systemMode = true;
            Serial.println("Cambiado a Modo Automatico Predefinido");
        } else {
            systemMode = false;
            Serial.println("Cambiado a Modo Sleep para ahorro de energía");
        }
    }
}

// Función para cambiar el modo del sistema desde Telegram
void setModeFromTelegram(bool mode) {
    systemMode = mode;
    if (systemMode) {
        Serial.println("Modo cambiado a Automatico Predefinido desde Telegram");
    } else {
        Serial.println("Modo cambiado a Modo Sleep desde Telegram");
    }
}

// Función para la operación en modo predefinido
void modePredefinedOperation() {
    struct tm timeinfo;

    Serial.println("Operacion en modo PREDEFINIDO");

    // Intentar obtener la hora actual
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Error al obtener la hora");
        return;
    }

    int seconds = timeinfo.tm_sec;
    int minutes = timeinfo.tm_min;
    int hours = timeinfo.tm_hour;

    // Calcular el tiempo restante hasta el siguiente múltiplo de 1 minuto
    int secondsUntilNextSend = 60 - seconds;

    // Esperar hasta el siguiente múltiplo de 1 minuto
    if (secondsUntilNextSend > 0) {
        delay(secondsUntilNextSend * 1000);
    }

    // Leer sensores y publicar los datos mediante MQTT
    sensordata = readSensors();  // Leer sensores

    controlActuators(sensordata, sensordata.timestamp);  // Controlar los actuadores

    saveToSDMasterInput(sensordata);  // Guardar datos en la SD

    saveToDataBase(sensordata); // Guardar datos en la base de datos MySQL

    publishSensorData(sensordata);  // Publicar datos de sensores mediante MQTT

    // Consultar la API del tiempo y guardar los datos en la SD
    getWeather(sensordata.timestamp);

    // Consultar la API del tiempo a las 6 a.m. y guardar los datos en la SD
    if (hours == 6 && minutes == 0 && seconds < 20) {
        getForecast(); // Solo se ejecuta una vez a las 6 a.m.
    }

    // Comprobar si los actuadores han cumplido su tiempo de activación máximo
    checkActuatorTimers();

    delay(1000);
}

// Función para el modo Sleep
void sleepMode() {
    Serial.println("Operacion en Modo Deep Sleep para ahorro de energía");

    // Configura el pin 33 como fuente de wakeup (HIGH)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)33, 1);

    Serial.println("Entrando en Deep Sleep. El sistema se reiniciará al despertar");
    delay(100);

    esp_deep_sleep_start();
}