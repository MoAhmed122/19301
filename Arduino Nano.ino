#include <Wire.h>
#include <LCD_I2C.h>
#include <DHT.h>  // Include the DHT library

// Initialize the LCD with the I2C address and dimensions (16x2)
LCD_I2C lcd(0x27, 16, 2); // Change the address if needed

// Define the IR sensor pins
const int irSensor1Pin = 2;  // First sensor pin (outside)
const int irSensor2Pin = 3;  // Second sensor pin (inside)

// DHT11 sensor pin and type
const int dhtPin = 4;         // Pin connected to DHT11
DHT dht(dhtPin, DHT11);       // Create DHT11 object

// MQ-135 sensor pin
const int mq135Pin = A0;      // Analog pin connected to MQ-135

// Buzzer pin
const int buzzerPin = 5;      // Pin connected to the buzzer


const int tempLEDPin = 6; // Pin for the temperature LED
const int peopleLEDPin = 7; // Pin for the people count LED
const int airQualityLEDPin = 8; // Pin for the air quality LED

float airQualityPercent = (analogRead(mq135Pin) / 1023.0) * 100;
float temperature = dht.readTemperature();

 


// Variables to keep track of sensor states, timers, and people count
bool sensor1Triggered = false;
bool sensor2Triggered = false;
unsigned long entryStartTime = 0;
unsigned long exitStartTime = 0;
int peopleCount = 0;




void setup() {
  // Start serial communication
  Serial.begin(9600);
  
  // Set the IR sensor pins as input
  pinMode(irSensor1Pin, INPUT);
  pinMode(irSensor2Pin, INPUT);
  
  // Initialize the DHT11 sensor
  dht.begin();
  
  // Initialize the LCD
  lcd.begin();  // Initialize the LCD
  lcd.backlight();  // Turn on the backlight
  
  // Initialize buzzer pin
  pinMode(buzzerPin, OUTPUT);
// the led thingy
  pinMode(tempLEDPin, OUTPUT);
  pinMode(peopleLEDPin, OUTPUT);
  pinMode(airQualityLEDPin, OUTPUT);

  // Print initial values on the LCD
  lcd.print("Air:0% Ppl:");
}

void loop() {
  // Read the sensor values
  int sensor1Value = digitalRead(irSensor1Pin);
  int sensor2Value = digitalRead(irSensor2Pin);

  // Read temperature and humidity from DHT11
  float temperature = dht.readTemperature();  // Get temperature in Celsius

  // Read air quality from MQ-135
  int mq135Value = analogRead(mq135Pin); // Read the analog value from MQ-135
  float airQualityPercent = (mq135Value / 1023.0) * 100; // Calculate air quality percentage
  
  

  bool temperatureExceeded = temperature > 25;
  bool peopleExceeded = peopleCount > 10;
  bool airQualityExceeded = airQualityPercent > 20;

  // Activate buzzer if any threshold is exceeded
  if (temperatureExceeded || peopleExceeded || airQualityExceeded) {
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
  } else {
    digitalWrite(buzzerPin, LOW);  // Turn off the buzzer
  }



  // Update the LCD display with air quality percentage and people count
  lcd.setCursor(0, 0);  // Move to the start of the first row
  lcd.print("Air:");
  lcd.print(airQualityPercent, 1); // Display air quality percentage to 1 decimal place
  lcd.print("% Ppl:");
  lcd.print(peopleCount);         // Display people count
  
  // Clear extra characters if the people count has fewer digits
  if (peopleCount < 10) {
    lcd.print(" ");               // Print space to clear any leftover digit
  }

  // Display temperature value on the second line
  lcd.setCursor(0, 1);            // Move to the start of the second row
  lcd.print("Temp:");           
  lcd.print(temperature, 1);      // Display temperature value to 1 decimal place
  lcd.print(" C"); 


  // Define the timer interval in milliseconds (5000 ms = 5 seconds)
  const unsigned long timerInterval = 1000;

  // Entry detection logic
  if (sensor1Value == LOW && !sensor1Triggered && !sensor2Triggered) {
    sensor1Triggered = true;           // Mark sensor 1 as triggered
    entryStartTime = millis();         // Start the entry timer
  }
  
  if (sensor2Value == LOW && sensor1Triggered && !sensor2Triggered) {
    if (millis() - entryStartTime <= timerInterval) {  // Check if within 5-second window
      Serial.println("Someone has entered the room!");
      peopleCount++;  // Increment the people counter
      Serial.print("People count: ");
      Serial.println(peopleCount);
      // Send data to ESP32
      Serial.print("Data to ESP32: ");
      Serial.print("Temp: ");
      Serial.print(temperature);
      Serial.print(" C, Air: ");
      Serial.print(airQualityPercent);
      Serial.print("%, Ppl: ");
      Serial.println(peopleCount);

      // Reset both triggers for new detection
      sensor1Triggered = false;
      sensor2Triggered = false;
    }
  }

  // If 5 seconds have passed without the second sensor triggering, reset
  if (sensor1Triggered && millis() - entryStartTime > timerInterval) {
    sensor1Triggered = false;  // Reset entry trigger
  }

  // Exit detection logic
  if (sensor2Value == LOW && !sensor2Triggered && !sensor1Triggered) {
    sensor2Triggered = true;           // Mark sensor 2 as triggered
    exitStartTime = millis();          // Start the exit timer
  }
  
  if (sensor1Value == LOW && sensor2Triggered && !sensor1Triggered) {
    if (millis() - exitStartTime <= timerInterval) {  // Check if within 5-second window
      Serial.println("Someone has left the room!");
      if (peopleCount > 0) peopleCount--;  // Decrement the people counter, ensuring it doesn't go below zero
      Serial.print("People count: ");
      Serial.println(peopleCount);
      // Send data to ESP32
      Serial.print("Data to ESP32: ");
      Serial.print("Temp: ");
      Serial.print(temperature);
      Serial.print(" C, Air: ");
      Serial.print(airQualityPercent);
      Serial.print("%, Ppl: ");
      Serial.println(peopleCount);

      // Reset both triggers for new detection
      sensor1Triggered = false;
      sensor2Triggered = false;
    }
  }


  //here!!!!!!!!!!!!!!!!!!!


  if (temperature > 25) {
    digitalWrite(tempLEDPin, HIGH); // Turn on the LED
  } else {
    digitalWrite(tempLEDPin, LOW); // Turn off the LED
  }

 if (peopleCount > 10) {
    digitalWrite(peopleLEDPin, HIGH); // Turn on the LED
  } else {
    digitalWrite(peopleLEDPin, LOW); // Turn off the LED
  }
 
  
  if (airQualityPercent > 20) {
    digitalWrite(airQualityLEDPin, HIGH); // Turn on the LED
  } else {
    digitalWrite(airQualityLEDPin, LOW); // Turn off the LED
  }


  // If 5 seconds have passed without the first sensor triggering, reset
  if (sensor2Triggered && millis() - exitStartTime > timerInterval) {
    sensor2Triggered = false;  // Reset exit trigger
  }

  // Update temperature and send data to ESP32 every 2 seconds
  static unsigned long lastDataUpdate = 0;
  if (millis() - lastDataUpdate >= 2000) { // Update every 2 seconds
    lastDataUpdate = millis(); // Update timer

    // Read temperature and send it to Serial
    Serial.print("Temp: ");
    Serial.print(temperature); // Print the temperature value
    Serial.println(" C"); // Print the unit

    // Send air quality percentage and people count to Serial
    Serial.print("Air: ");
    Serial.print(airQualityPercent);
    Serial.print("%, Ppl: ");
    Serial.println(peopleCount); // Print people count

    // Send data to ESP32 via Serial (UART)
    Serial.print("Data to ESP32: ");
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" C, Air: ");
    Serial.print(airQualityPercent);
    Serial.print("%, Ppl: ");
    Serial.println(peopleCount);
  }

  // Small delay to stabilize readings
  delay(100);
}
