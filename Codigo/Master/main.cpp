#include <Arduino.h>

#include "sensors.h"
#include "actuators.h"
#include "storage.h"
#include "weatherapi.h"
#include "mode.h"
#include "time.h"
#include "mqtt.h"
#include "esp32_zip.h"
#include "driver/rtc_io.h"

void setup() {
    Serial.begin(115200);

    // Libera el pin del modo RTC IO (Deep Sleep)
    rtc_gpio_deinit((gpio_num_t)33);

    // Configurar el pin del interruptor
    pinMode(switchPin, INPUT);

    // Inicializar la tarjeta SD y crear archivos necesarios si no existen
    setupStorage();

    // Configurar la conexión WiFi
    setupWiFi();

    // Configura el reloj y sincroniza la hora con un servidor NTP
    setupTime();

    // Inicializa y configura los sensores conectados
    setupSensors();

    // Configura los actuadores para su control
    setupActuators();

    // Configura la API del clima, leyendo datos de configuración desde un archivo en la SD
    setupWeatherAPI("/config.txt");

    // Configura MQTT para la comunicación
    setupMQTT();  // Iniciar la conexión MQTT y las suscripciones
}

void loop() {
    // Verificar si el interruptor ha cambiado y actualizar el modo del sistema
    checkSwitch();

    // Ejecutar la operación según el modo actual del sistema -> true = Modo predefinido, false = Modo sleep
    if (systemMode) {
        modePredefinedOperation();  // Modo predefinido
    } else {
        sleepMode();  // Modo sleep para ahorro de energía
    }

    // Mantener la conexión MQTT activa
    loopMQTT();
}