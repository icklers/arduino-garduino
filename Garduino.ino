/*
 * Garduino
 * Environment surveillance for growing houses
 * 
 * Reads temperature and humidity from DHT-22 sesor every 10 seconds
 * Controls fans to hold defined temperature
 * Controls light cycles
 * 
 * Status output to 16x2 LCD
 */

#define GARDUINOVERSION 0.10

// Configure and initialize DHT-22 Sensor
#include "DHT.h"
#define DHTPIN 2        // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

// Initial program
void setup() {
  Serial.begin(9600);
  Serial.print("Booting Garduino version ");
  Serial.print(GARDUINOVERSION);
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.println(".");
  delay(1000);

  // Initialize DHT-22 Temp/Humidity sensor
  dht.begin();

}

// Repeated checks go here.
void loop() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  // float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" C\t");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("% ");
  Serial.print("\n");
  
  // Wait a few seconds between measurements.
  delay(10000);

}
