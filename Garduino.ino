/*
 * Garduino
 * Environment surveillance for growing houses
 * 
 * (c) 2016 Sebastian Ickler
 * Licensed under BSD 3-Clause license. See LICENSE for more details.
 * 
 * [x] Read temperature and humidity from DHT-22 sesor every 10 seconds
 * [ ] Control fans to hold defined temperature
 * [ ] Control light cycles
 * 
 * [x] Output to TFT
 * [ ] Improve TFT output (if touched => screen on, after 30 seconds no touch => screen off)
 * [ ] Include Wifi-Module
 * [ ] Submit sensor data to ThingSpeak
 */

#define GARDUINOVERSION 0.20

// Libraries
#include <DHT.h>
//#include <Wire.h>
//#include <Ethernet.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

// Configure and initialize DHT-22 Sensor
#define DHTPIN 10        // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

//   D0 connects to digital pin 8
//   D1 connects to digital pin 9
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define BACKGROUNDCOLOR 0x001F

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Configure Ethernet
//byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 }; // Must be unique on local network

// Configure Thingspeak
//char thingSpeakAddress[] = "api.thingspeak.com";
//String writeAPIKey = "KVPK4FQUL5NQ0NYC";
//const unsigned long updateThingSpeakInterval = (unsigned long)120*1000; // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// Initialize variables
//unsigned long lastConnectionTime = 0; 
//boolean lastConnected = false;
//int failedCounter = 0;

float humidity;
float temperature;


// Initial program
void setup() {
  // Initialize serial console
  Serial.begin(9600);

  // Initialize 2.8" TFT
  /* 
   * Set LCD identifier first
   * Automatic recognition works on most TFT's.
   * Set following variable to enable automatic model recognition.
   *   uint16_t identifier = tft.readID();
   *   
   * but we have to hard code it *sigh*
   */
  uint16_t identifier = 0x9341;
  
  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(BACKGROUNDCOLOR);

  // Initialization procedure
  // Serial output
  Serial.print("Garduino v");
  Serial.print(GARDUINOVERSION);

  // TFT Output
  tft.setCursor(70, 0);
  tft.setTextColor(GREEN); tft.setTextSize(2);
  tft.print("Garduino v");
  tft.print(GARDUINOVERSION);
  delay(3000); // wait 3 seconds
  tft.setCursor(0, 30);
  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.print("Initializing sensors:");
  

  /* ------------------------------------------------------------------- */
  /* Sensor initialization */
  
  // Initialize DHT-22 Temp/Humidity sensor
  dht.begin();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();

  tft.setCursor(20,50);
  tft.print("DHT-22 Temp/Hum");
  delay(1000);
  tft.setCursor(20,180);
  if (isnan(humidity) || isnan(temperature)) {
    // Serial output
    Serial.println("DHT-22 Temp/Hum    FAIL");
  
    // TFT output
    tft.print("[");
    tft.setTextColor(RED); tft.setTextSize(2);
    tft.print(" FAIL ");
    tft.setTextColor(WHITE); tft.setTextSize(2);
    tft.print("]");
    return;
    // TODO better: wait for confirmation
  } else {
    // Serial output
    Serial.println("DHT-22 Temp/Hum    OK");

    // TFT output
    tft.print("[");
    tft.setTextColor(GREEN); tft.setTextSize(2);
    tft.print("  OK  ");
    tft.setTextColor(WHITE); tft.setTextSize(2);
    tft.print("]");
  }



}

// Repeated checks go here.
void loop() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    tft.fillScreen(BLACK);
    tft.setTextColor(RED); tft.setTextSize(2);
    tft.setCursor(70,105);
    tft.print("Failed to read");
    tft.setCursor(58,123);
    tft.print("from DHT sensor!");
   
    Serial.println("Failed to read from DHT sensor!");
   
    delay(5000);
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  // float hic = dht.computeHeatIndex(t, h, false);
  tft.fillScreen(BACKGROUNDCOLOR);
  // tft.fillRect(30, 180, 80, 80, BACKGROUNDCOLOR)
  tft.setCursor(70, 0);
  tft.setTextColor(GREEN); tft.setTextSize(2);
  tft.print("Garduino v");
  tft.println(GARDUINOVERSION);
  

  tft.setTextColor(WHITE); tft.setTextSize(2);
  tft.setCursor(25,30);
  tft.print("Temperature: ");
  tft.setCursor(180,30);
  tft.print(temperature);
  tft.print(" C");
  

  tft.setCursor(25,50);
  tft.print("Humidity: ");
  tft.setCursor(180,50);
  tft.print(humidity);
  tft.print("%");
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C\t");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("% ");
  Serial.print("\n");
  
  // Wait a few seconds between measurements.
  delay(5000);

}
