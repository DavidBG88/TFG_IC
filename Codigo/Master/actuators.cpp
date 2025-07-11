#include "actuators.h"
#include "sensors.h"
#include "storage.h"
#include <Arduino.h>

// Pines de los actuadores
const int ledPin = 27;    // GPIO para luces
const int fanInputPin = 25; // GPIO para ventilador de entrada
const int fanOutputPin = 26; // GPIO para ventilador de salida
const int pumpPin = 14;   // GPIO para la bomba de riego
const int fan1 = 12;  // Pin del ventilador 1
const int fan2 = 13;  // Pin del ventilador 2

volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;

void IRAM_ATTR onTachPulse1() {
    pulseCount1++;
}

void IRAM_ATTR onTachPulse2() {
    pulseCount2++;
}

// Variables para controlar el tiempo de activación
unsigned long pumpStartTime = 0;
unsigned long fanStartTime = 0;
unsigned long lightStartTime = 0;
unsigned long pumpTelegramStartTime = 0;
unsigned long fanTelegramStartTime = 0;
unsigned long lightTelegramStartTime = 0;

bool ledActive = false;
bool pumpActive = false;
bool fanActive = false;

// Duraciones preestablecidas
const unsigned long pumpDuration = 20000;  // Duración máxima de riego en milisegundos
const unsigned long fanDuration = 300000;  // Duración máxima de ventilación en milisegundos (5 minutos)
const unsigned long lightDuration = 300000; // Duración máxima de iluminación en milisegundos (5 minutos)

// Umbrales preestablecidos
float lightThreshold = 1000;
float waterLevelThreshold = 30;
float soilMoistureThreshold = 30;
float co2Threshold = 500;
float aqiThreshold = 2;
float tempThreshold = 27.0;

void setupActuators() {
    pinMode(ledPin, OUTPUT);
    pinMode(fanInputPin, OUTPUT);
    pinMode(fanOutputPin, OUTPUT);
    pinMode(pumpPin, OUTPUT);
    pinMode(fan1, INPUT_PULLUP);
    pinMode(fan2, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(fan1), onTachPulse1, FALLING);
    attachInterrupt(digitalPinToInterrupt(fan2), onTachPulse2, FALLING);

    // Inicializar los actuadores apagados
    digitalWrite(ledPin, HIGH);
    digitalWrite(fanInputPin, HIGH);
    digitalWrite(fanOutputPin, HIGH);
    digitalWrite(pumpPin, HIGH);
}

void controlActuators(SensorData& data, const char* timestamp) {
    // Extraer la hora del timestamp usando funciones de C estándar
    char hourStr[3] = { timestamp[0], timestamp[1], '\0' };
    int hour = atoi(hourStr);

    // Control de luces
    if (data.veml7700 < lightThreshold && (hour >= 6 && hour < 22)) {
        digitalWrite(ledPin, LOW);  // Encender luz
        ledActive = true;
        lightStartTime = millis();  // Actualizar tiempo de inicio
        saveActivationLog("light", lightDuration / 1000);
        Serial.println("Luz encendida y datos guardados en SD");
    } else if (hour < 6 || hour >= 22) {
        digitalWrite(ledPin, HIGH); // Apagar luz por fuera del horario permitido
        Serial.println("Luz apagada por estar fuera del horario permitido");
    } else {
        digitalWrite(ledPin, HIGH); // Apagar luz por suficiente luz
        Serial.println("Luz apagada por suficiente luz");
    }

    // Control del sistema de riego
    if (data.water_level > waterLevelThreshold && data.soil_moisture < soilMoistureThreshold) {
        digitalWrite(pumpPin, LOW);  // Encender bomba
        pumpActive = true;
        pumpStartTime = millis();  // Actualizar tiempo de inicio
        saveActivationLog("pump", pumpDuration / 1000);
        Serial.println("Bomba de riego activada y datos guardados en SD");
    } 

    // Control de ventiladores
    if (data.ens160_aqi >= aqiThreshold || data.ens160_eco2 > co2Threshold || data.aht20_temp >= tempThreshold) {
        digitalWrite(fanInputPin, LOW);  // Encender ventilador de entrada
        digitalWrite(fanOutputPin, LOW); // Encender ventilador de salida
        fanActive = true;
        fanStartTime = millis();  // Actualizar tiempo de inicio
        saveActivationLog("fan", fanDuration / 1000);
        Serial.println("Ventiladores activados y datos guardados en SD");
    }

    // Cálculo de RPM para ventiladores
    noInterrupts();  
    int pulses1 = pulseCount1;
    int pulses2 = pulseCount2;
    pulseCount1 = 0;
    pulseCount2 = 0;
    interrupts();  

    data.fan1_rpm = (pulses1 * 60.0) / (2.0 * 10.0);
    data.fan2_rpm = (pulses2 * 60.0) / (2.0 * 10.0);
}

void checkActuatorTimers() {
    // Verificación de luces
    if (ledActive && millis() - lightStartTime >= lightDuration) {
        digitalWrite(ledPin, HIGH);
        ledActive = false;
        Serial.println("Luz apagada despues de la activacion por sensor");
    }

    // Verificacion de bomba
    if (pumpActive && millis() - pumpStartTime >= pumpDuration) {
        digitalWrite(pumpPin, HIGH);
        pumpActive = false;
        Serial.println("Bomba apagada despues de la activacion por sensor");
    }

    // Verificación de ventiladores
    if (fanActive && millis() - fanStartTime >= fanDuration) {
        digitalWrite(fanInputPin, HIGH);
        digitalWrite(fanOutputPin, HIGH);
        fanActive = false;
        Serial.println("Ventiladores apagados despues de la activacion por sensor");
    }
}

void activateActuatorTelegram(String actuator, int duration) {
    unsigned long telegramDuration;  // Duracion de activación desde Telegram

    if (actuator == "light") {
        telegramDuration = min((unsigned long)(duration * 1000), lightDuration);  // Usar el menor valor entre la duración especificada y el tiempo máximo permitido
        digitalWrite(ledPin, LOW);  // Encender el LED
        lightTelegramStartTime = millis();  // Registrar el tiempo de inicio
        ledActive = true;
        Serial.println("Luz activada por Telegram");
    } else if (actuator == "pump") {
        telegramDuration = min((unsigned long)(duration * 1000), pumpDuration);
        digitalWrite(pumpPin, LOW);  // Encender la bomba
        pumpTelegramStartTime = millis();
        pumpActive = true;
        Serial.println("Bomba activada por Telegram");
    } else if (actuator == "fan") {
        telegramDuration = min((unsigned long)(duration * 1000), fanDuration);
        digitalWrite(fanInputPin, LOW);  // Encender el ventilador de entrada
        digitalWrite(fanOutputPin, LOW); // Encender el ventilador de salida
        fanTelegramStartTime = millis();
        fanActive = true;
        Serial.println("Ventilador activado por Telegram");
    } else {
        Serial.println("Actuador desconocido");
    }

    // Apagar el actuador seleccionado después de que pase el tiempo especificado
    delay(telegramDuration);
    
    if (actuator == "light") {
        digitalWrite(ledPin, HIGH);  // Apagar el LED
        ledActive = false;
        Serial.println("Luz apagada despues de activacion por Telegram");
    } else if (actuator == "pump") {
        digitalWrite(pumpPin, HIGH);  // Apagar la bomba
        pumpActive = false;
        Serial.println("Bomba apagada despues de activacion por Telegram");
    } else if (actuator == "fan") {
        digitalWrite(fanInputPin, HIGH);  // Apagar el ventilador de entrada
        digitalWrite(fanOutputPin, HIGH); // Apagar el ventilador de salida
        fanActive = false;
        Serial.println("Ventilador apagado despues de activacion por Telegram");
    }
}