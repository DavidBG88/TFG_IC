#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_AHTX0.h>

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xA8, 0x48, 0xFA, 0xFF, 0xA8, 0x37};

// Set your Board ID (ESP Sender #1 = BOARD_ID 1, ESP Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 2

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    int id;
    float temperatura;
    float humedad;
} struct_message;

// Create a struct_message called myData to store variables to be sent
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 10000; // Send data every 10 seconds

Adafruit_AHTX0 aht;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  
  // Initialize the AHT10 sensor
  if (!aht.begin()) {
    Serial.println("No se pudo encontrar el sensor AHT10");
    while (1) delay(10);
  }
  Serial.println("Sensor AHT10 encontrado");
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  
  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Once ESPNow is successfully init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Read data from AHT10 sensor
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    
    // Set values to send
    myData.id = BOARD_ID;
    myData.temperatura = temp.temperature;
    myData.humedad = humidity.relative_humidity;

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    // Print the sent values for debugging
    Serial.print("Enviando - Temperatura: ");
    Serial.print(myData.temperatura);
    Serial.print(" °C, Humedad: ");
    Serial.print(myData.humedad);
    Serial.println(" %rH");

    lastTime = millis();
  }
}
