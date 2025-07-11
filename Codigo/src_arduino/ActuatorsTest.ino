// Definir los pines para los relés
#define RELAY_PIN_1 26  // GPIO 26 (D26)
#define RELAY_PIN_2 27  // GPIO 27 (D27)
#define RELAY_PIN_3 14  // GPIO 14 (D14)
#define RELAY_PIN_4 12  // GPIO 12 (D12)

// Definir los pines para las entradas
#define INPUT_PIN_22 22  // GPIO 22 (D22)
#define INPUT_PIN_23 23  // GPIO 23 (D23)

void setup() {
  // Configurar los pines de los relés como salida
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);

  // Configurar los pines 22 y 23 como entrada
  pinMode(INPUT_PIN_22, INPUT);
  pinMode(INPUT_PIN_23, INPUT);

  // Iniciar con los relés apagados
  digitalWrite(RELAY_PIN_1, LOW);
  digitalWrite(RELAY_PIN_2, LOW);
  digitalWrite(RELAY_PIN_3, LOW);
  digitalWrite(RELAY_PIN_4, LOW);

  // Iniciar comunicación serie a 9600 baudios
  Serial.begin(9600);
}

void loop() {
  // Leer los valores de los pines 22 y 23
  int rpmInput22 = digitalRead(INPUT_PIN_22);
  int rpmOutput23 = digitalRead(INPUT_PIN_23);

  // Mostrar los valores leídos en la consola
  Serial.print("PIN 22 INPUT RPM = ");
  Serial.println(rpmInput22);

  Serial.print("PIN 23 OUTPUT RPM = ");
  Serial.println(rpmOutput23);

  // Activar los relés (Encender)
  digitalWrite(RELAY_PIN_1, HIGH);
  digitalWrite(RELAY_PIN_2, HIGH);
  digitalWrite(RELAY_PIN_3, HIGH);
  digitalWrite(RELAY_PIN_4, HIGH);
  delay(5000);  // Mantener encendidos durante 5 segundos

  // Desactivar los relés (Apagar)
  digitalWrite(RELAY_PIN_1, LOW);
  digitalWrite(RELAY_PIN_2, LOW);
  digitalWrite(RELAY_PIN_3, LOW);
  digitalWrite(RELAY_PIN_4, LOW);
  delay(5000);  // Mantener apagados durante 5 segundos
}