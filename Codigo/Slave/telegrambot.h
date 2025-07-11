#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <Arduino.h>
#include "mqtt.h"

// Tamaño de la EEPROM para guardar el último message_id
#define EEPROM_SIZE 4

// Funciones relacionadas con el bot de Telegram
void setupTelegramBot();
void handleNewMessages(int numNewMessages);
void sendTelegramAlert(const String& message);
void checkTelegramUpdates();
void publishSensorDataTelegram(SensorData sensordata);

#endif // TELEGRAMBOT_H