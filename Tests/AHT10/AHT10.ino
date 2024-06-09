#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);

  if (!aht.begin()) {
    Serial.println("No se pudo encontrar el sensor AHT10");
    while (1) delay(10);
  }
  Serial.println("Sensor AHT10 encontrado");
}

void loop() {
  sensors_event_t humedad, temp;
  aht.getEvent(&humedad, &temp);
  Serial.print("Temperatura: "); Serial.print(temp.temperature); Serial.println(" grados C");
  Serial.print("Humedad: "); Serial.print(humedad.relative_humidity); Serial.println("% rH");

  delay(500);
}