#include <Arduino.h>
#include "mqtt.h"
#include "telegrambot.h"
#include "aht10.h"
#include <EEPROM.h>

// Intervalo de tiempo para lecturas del sensor (1 minuto)
unsigned long lastSensorRead = 0;

// Zona horaria y servidores NTP para España
const char* ntpServer0 = "0.es.pool.ntp.org";
const char* ntpServer1 = "1.es.pool.ntp.org";
const char* ntpServer2 = "2.es.pool.ntp.org";
const long gmtOffset_sec = 3600;        // Offset de GMT para España (GMT+1)
const int daylightOffset_sec = 3600;    // Horario de verano

void setupTime() {
    for (int i = 0; i < 5; i++) { // Reintenta hasta 5 veces
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer0, ntpServer1, ntpServer2);
        delay(2000);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            Serial.println("Hora obtenida con exito");
            return;
        }
        Serial.println("Error al obtener la hora, reintentando...");
    }
    Serial.println("No se pudo obtener la hora despues de varios intentos");
}

void setup() {
    Serial.begin(115200);

    // Inicializar el sensor AHT10
    setupSensor();

    // Configurar y arrancar MQTT
    setupMQTT();

    // Configurar la sincronización de tiempo NTP
    setupTime();

    // Configurar y arrancar el bot de Telegram
    setupTelegramBot();
}

void loop() {
    static unsigned long lastUpdate = 0;
    unsigned long currentMillis = millis();
    struct tm timeinfo;
    char timestamp[20];

    // Verificar y manejar las interacciones del bot de Telegram cada 2 segundos
    if (currentMillis - lastUpdate > 2000) {
        checkTelegramUpdates();
        lastUpdate = currentMillis;
    }

    // Sincronizar la lectura del sensor entre los segundos 0 y 5 del minuto
    if (getLocalTime(&timeinfo)) {
        int currentSecond = timeinfo.tm_sec;
        int currentMinute = timeinfo.tm_min;

        // Verificar si estamos en los primeros 5 segundos del minuto
        if (currentSecond >= 0 && currentSecond <= 5) {
            // Verificar que no hemos hecho ya una lectura en este minuto
            if (lastSensorRead != currentMinute) {
                // Formatear el timestamp con segundos 00
                strftime(timestamp, sizeof(timestamp), "%H:%M:%S %d/%m/%Y", &timeinfo);

                // Ajustar el timestamp para asegurar que siempre tenga "00" segundos
                strncpy(&timestamp[6], "00", 2);

                // Leer datos del sensor AHT10 y publicar los datos vía MQTT
                struct_message sensorData = readSensor(timestamp);

                publishSensorData(sensorData);

                // Actualizar la marca de tiempo del último minuto de lectura
                lastSensorRead = currentMinute;
            }
        }
    } else {
        Serial.println("Error al obtener la hora");
    }

    // Mantener la conexión MQTT
    loopMQTT();
}