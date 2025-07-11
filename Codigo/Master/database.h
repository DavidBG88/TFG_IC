#ifndef DATABASE_H
#define DATABASE_H

#include "sensors.h"
#include "mqtt.h"
#include "weatherapi.h"

void saveToDataBase(const SensorData& sensorData);
void saveSlaveToDataBase(const struct_message& data);
void saveAPIToDataBase(const WeatherAPIData& apiData);

#endif // DATABASE_H