#ifndef AHT10_H
#define AHT10_H

#include <Adafruit_AHTX0.h>
#include "mqtt.h"  // Incluir mqtt.h para acceder a struct_message

// Funciones relacionadas con el sensor AHT10
void setupSensor();            // Inicializar el sensor AHT10
struct_message readSensor(const char* timestamp);  // Leer los datos del sensor y devolver struct_message

#endif // AHT10_H