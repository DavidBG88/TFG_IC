#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "telegrambot.h"
#include "mqtt.h"
#include <EEPROM.h>

const String TELEGRAM_BOT_TOKEN = "X";
const String CHAT_ID = "X";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, secured_client);

// Declaración de la estructura de datos de sensores
extern SensorData sensordata;

void setupTelegramBot() {
    secured_client.setInsecure();  // No SSL certificate validation
    Serial.println("Bot de Telegram iniciado.");

    EEPROM.begin(EEPROM_SIZE);  // Inicializar EEPROM
}

void handleNewMessages(int numNewMessages) {
    // Mensaje por terminal indicando la cantidad de mensajes nuevos recibidos
    Serial.printf("Nuevos mensajes recibidos: %d\n", numNewMessages);

    for (int i = 0; i < numNewMessages; i++) {
        String text = bot.messages[i].text;
        
        // Mostrar el mensaje recibido en la terminal
        Serial.printf("Mensaje recibido: %s\n", text.c_str());

        if (text == "/start") {
            bot.sendMessage(CHAT_ID, "Bienvenido al sistema de control del invernadero");
            Serial.println("Comando /start ejecutado: Se ha enviado mensaje de bienvenida.");

        } else if (text == "/help") {
            String helpMessage = "/start - Iniciar el bot\n"
                                 "/help - Mostrar esta ayuda\n"
                                 "/mode_sleep - Activar modo sleep\n"
                                 "/mode_predefined - Activar modo predefinido\n"
                                 "/activate_actuator <actuator> <duration> - Activar un actuador por la duración especificada (en segundos) - Actuator = [fan, light, pump]\n"
                                 "/read_sensors - Obtener las lecturas actuales de los sensores\n"
                                 "/get_log - Obtener un archivo comprimido con los registros de datos";
            bot.sendMessage(CHAT_ID, helpMessage);
            Serial.println("Comando /help ejecutado: Se ha enviado el mensaje de ayuda.");

        } else if (text == "/mode_predefined") {
            bot.sendMessage(CHAT_ID, "Modo predefinido activado");
            publishCommand("predefinido");  // Publicar comando en MQTT
            Serial.println("Comando /mode_predefined ejecutado: Modo predefinido activado y enviado por MQTT.");

        } else if (text == "/mode_sleep") {
            bot.sendMessage(CHAT_ID, "Modo sleep activado");
            publishCommand("sleep");  // Publicar comando en MQTT
            Serial.println("Comando /mode_sleep ejecutado: Modo sleep activado y enviado por MQTT.");

        } else if (text.startsWith("/activate_actuator")) {
            char actuator[20];
            int duration;

            // Mostrar el comando recibido para activación de actuador en la terminal
            Serial.println("Comando /activate_actuator recibido: Analizando...");
    
            if (sscanf(text.c_str(), "/activate_actuator %19s %d", actuator, &duration) == 2) {
                String commandMessage = String(actuator) + " " + String(duration);
                publishCommand(commandMessage);  // Publicar comando en MQTT
                bot.sendMessage(CHAT_ID, String(actuator) + " activado por " + String(duration) + " segundos");
                Serial.printf("Comando /activate_actuator ejecutado: Actuador %s activado por %d segundos.\n", actuator, duration);
            } else {
                bot.sendMessage(CHAT_ID, "Error: comando incorrecto");
                Serial.println("Error: El formato del comando /activate_actuator es incorrecto.");
            }

        } else if (text == "/read_sensors") {
            bot.sendMessage(CHAT_ID, "Solicitud de lectura de sensores enviada");
            publishCommand("read_sensors");  // Publicar comando en MQTT
            Serial.println("Comando /read_sensors ejecutado: Solicitud de lectura de sensores enviada por MQTT.");

        } else if (text == "/get_log") {
            /*
            if (compressLogs("/data.zip")) {
                bot.sendDocument(CHAT_ID, "/data.zip", "Logs comprimidos");
            } else {
                bot.sendMessage(CHAT_ID, "Error al comprimir los logs.");
            }
            */
            bot.sendMessage(CHAT_ID, "Logs enviados");
            Serial.println("Comando /get_log ejecutado: Logs enviados (falta compresión).");
        }
    }
}

void checkTelegramUpdates() {
    static int lastProcessedMessageId = 0;

    if (lastProcessedMessageId == 0) {
        // Leer el último message_id procesado desde EEPROM al inicio
        EEPROM.get(0, lastProcessedMessageId);
        Serial.printf("Ultimo message_id procesado leido desde EEPROM: %d\n", lastProcessedMessageId);
    }

    // Obtener las actualizaciones del bot de Telegram
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    // Mostrar en la terminal cuántos mensajes nuevos se han recibido
    Serial.printf("Mensajes nuevos recibidos: %d\n", numNewMessages);

    if (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            int messageId = bot.messages[i].message_id;

            // Verificar si el messageId es mayor al último procesado
            if (messageId > lastProcessedMessageId) {
                handleNewMessages(numNewMessages);
                lastProcessedMessageId = messageId;

                // Guardar el último message_id procesado en EEPROM
                EEPROM.put(0, lastProcessedMessageId);
                EEPROM.commit();  // Asegurarse de que se escriba en la EEPROM
                Serial.printf("Ultimo message_id guardado en EEPROM: %d\n", lastProcessedMessageId);
            }
        }
    }
}

void publishSensorDataTelegram(SensorData sensordata) {
    // Preparar los datos para enviar al bot de Telegram
    String sensorInfo = "Datos de sensores:\n";
    sensorInfo += "Temperatura: " + String(sensordata.aht20_temp) + " °C\n";
    sensorInfo += "Humedad: " + String(sensordata.aht20_hum) + " %\n";
    sensorInfo += "Luz: " + String(sensordata.veml7700) + " lux\n";
    sensorInfo += "AQI: " + String(sensordata.ens160_aqi) + "\n";
    sensorInfo += "CO2 equivalente: " + String(sensordata.ens160_eco2) + " ppm\n";
    sensorInfo += "TVOC: " + String(sensordata.ens160_tvoc) + " ppb\n";
    sensorInfo += "Nivel de agua: " + String(sensordata.water_level) + " %\n";
    sensorInfo += "Humedad del suelo: " + String(sensordata.soil_moisture) + " %\n";
    sensorInfo += "Ventilador 1: " + String(sensordata.fan1_rpm) + " RPM\n";
    sensorInfo += "Ventilador 2: " + String(sensordata.fan2_rpm) + " RPM\n";
    
    // Enviar los datos al usuario de Telegram
    bot.sendMessage(CHAT_ID, sensorInfo);
    Serial.println("Datos de sensores enviados al bot de Telegram.");
}

void sendTelegramAlert(const String& message) {
    bot.sendMessage(CHAT_ID, message);
    Serial.println("Alerta enviada por Telegram: " + message);
}
