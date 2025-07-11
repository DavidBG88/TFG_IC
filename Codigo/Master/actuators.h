// actuators.h

#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "sensors.h"

// Declaración de funciones
void setupActuators();
void controlActuators(SensorData& data, const char* timestamp);
void activateActuatorTelegram(String actuator, int duration);
void checkActuatorTimers();

// Declaración de pines de actuadores (extern)
extern const int ledPin;
extern const int fanInputPin;
extern const int fanOutputPin;
extern const int pumpPin;

// Declaración de variables de tiempo de activación (extern)
extern unsigned long pumpStartTime;
extern unsigned long fanStartTime;
extern unsigned long lightStartTime;
extern const unsigned long pumpDuration;
extern const unsigned long fanDuration;
extern const unsigned long lightDuration;

// Declaración de umbrales (extern)
extern float lightThreshold;
extern float waterLevelThreshold;
extern float soilMoistureThreshold;
extern float co2Threshold;
extern float aqiThreshold;

#endif // ACTUATORS_H