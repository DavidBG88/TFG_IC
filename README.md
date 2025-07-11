# SIDCMOCS  
**Sistema Inteligente Distribuido de Control y Monitorización para la Optimización de Cultivos Sostenibles sobre una Arquitectura Edge-IoT basada en ESP32**

## Descripción

Este proyecto desarrolla **SIDCMOCS**, una solución modular y económica diseñada para automatizar el control climático de invernaderos mediante tecnologías abiertas. El sistema utiliza microcontroladores ESP32 en una arquitectura distribuida IoT para monitorizar variables ambientales y activar actuadores en función de las condiciones detectadas.

Entre sus funcionalidades destacan:

- Monitorización de temperatura, humedad, iluminación, calidad del aire y condiciones del suelo.
- Activación automática de ventiladores, luces LED y sistemas de riego.
- Comunicación distribuida mediante MQTT.
- Visualización remota con Grafana Cloud y alertas vía bot de Telegram.
- Almacenamiento local en tarjetas microSD y backend con Node.js y MySQL.
- Análisis de datos con Python y generación de gráficas.

El sistema ha sido validado mediante una maqueta funcional en un entorno doméstico controlado.

## Estructura del repositorio

- Codigo/ # Código fuente para ESP32 (C++), dividido en nodo maestro y esclavos, pruebas y scripts del backend

- Diagramas/ # Esquemas de arquitectura, montaje, etc

- Gastos.xlsx # Hoja de cálculo con presupuesto y materiales utilizados

- README.md # Archivo descriptivo del proyecto

## Tecnologías utilizadas

- **Hardware**: ESP32, sensores AHT10, ENS160, VEML7700, etc., actuadores (ventiladores, bombas, LEDs), panel solar, microSD
- **Firmware**: C++ (Arduino IDE, MQTT)
- **Backend**: Node.js, MySQL
- **Frontend**: Grafana Cloud (mediante Private Data Connection)
- **Otros**: Python (análisis de datos), Telegram Bot API, OpenWeatherMap API
