#ifndef MODE_H
#define MODE_H

// Pin para el interruptor SPDT
const int switchPin = 33;

extern bool systemMode;  // true = Modo predefinido, false = Modo Sleep

// Umbrales preestablecidos
extern float lightThreshold;
extern float waterLevelThreshold;
extern float soilMoistureThreshold;
extern float co2Threshold;
extern float aqiThreshold;

void modePredefinedOperation();
void sleepMode();
void checkSwitch();
void setModeFromTelegram(bool mode);

#endif // MODE_H