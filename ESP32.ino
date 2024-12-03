#include <WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT settings
const char* ssid = "المعلم ابو طرف";          // Replace with your WiFi SSID
const char* password = "mazsb47364";  // Replace with your WiFi password
const char* mqttServer = "thingsboard.cloud"; // ThingsBoard server
const int mqttPort = 1883;               // ThingsBoard MQTT port
const char* mqttUser = "PPQeFbImRv4cfc5qDliR"; // Replace with your ThingsBoard access token
const char* mqttClientID = "ESP32Client"; // Unique client ID

WiFiClient espClient;
PubSubClient client(espClient);

// Serial pins
#define RXp2 16
#define TXp2 17

// Global variables
float globalTemperature = 0.0;
float globalAirQuality = 0.0;
int globalPeopleCount = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2); // Set up Serial2 for UART communication

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    Serial.println("Message Received: " + data);

    // Find the indices of the required substrings
    int tempIndex = data.indexOf("Temp:");
    int airIndex = data.indexOf("Air:");
    int pplIndex = data.indexOf("Ppl:");

    // Make sure the indices are valid
    if (tempIndex != -1 && airIndex != -1 && pplIndex != -1) {
      // Extract and convert the temperature
      String tempStr = data.substring(tempIndex + 5, data.indexOf(" C", tempIndex)); // Extract until " C"
      globalTemperature = tempStr.toFloat();

      // Extract and convert the air quality percentage
      String airStr = data.substring(airIndex + 4, data.indexOf("%", airIndex)); // Extract until "%"
      globalAirQuality = airStr.toFloat();

      // Extract and convert the people count
      globalPeopleCount = data.substring(pplIndex + 4).toInt();

      // Print to Serial Monitor for verification
      Serial.println("Parsed Temperature: " + String(globalTemperature));
      Serial.println("Parsed Air Quality: " + String(globalAirQuality) + "%");
      Serial.println("Parsed People Count: " + String(globalPeopleCount));

      // Send data to ThingsBoard
      sendToThingsBoard();
    } else {
      Serial.println("Error: Could not find all required data in the message.");
    }
  }
}

// Function to send data to ThingsBoard
void sendToThingsBoard() {
  String payload = String("{\"temperature\":") + globalTemperature + ",\"airQuality\":" + globalAirQuality + ",\"peopleCount\":" + globalPeopleCount + "}";
  Serial.println("Data sent to ThingsBoard: " + payload);
  client.publish("v1/devices/me/telemetry", payload.c_str());
}

// Function to reconnect to MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqttClientID, mqttUser, NULL)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// MQTT callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming messages (if needed)
}
