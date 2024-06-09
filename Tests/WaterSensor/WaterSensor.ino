const int sensorPin = A0;

void setup() {

  Serial.begin(9600);
  
}

void loop() {
 
  int sensorValue = analogRead(sensorPin);              // Lee el valor analógico del sensor de agua
  int mappedValue = map(sensorValue, 0, 620, 0, 100);   // Mapea el valor leído a un rango conocido
  
  Serial.print("Nivel de agua: ");  // Imprime el valor mapeado por el puerto serial
  Serial.print(mappedValue);        // Imprime el valor mapeado
  Serial.println("%");              // Agrega el símbolo de porcentaje
  
  delay(1000);  // Espera un segundo antes de realizar la próxima lectura
}