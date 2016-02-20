/*
 * Garduino
 * Environment surveillance for growing houses
 * 
 * GitHub: https://github.org/icklers/arduino-garduino.git
 * 
 * (c) 2016 Sebastian Ickler
 * Licensed under BSD 3-Clause license. See LICENSE for more details.
 * 
 * [x] Read temperature and humidity from two DHT-22 sensors every 10 seconds
 * [ ] Control fans to hold defined temperature
 * [ ] Control light cycles
 * 
 * [x] Output to TFT
 * [ ] Improve TFT output (if touched => screen on, after 30 seconds no touch => screen off)
 * [ ] Include Wifi-Module
 * [ ] Submit sensor data to ThingSpeak
 */

#define GARDUINOVERSION 0.30

// Libraries
#include <DHT.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

// Configure and initialize DHT-22 Sensor 1
#define DHT1TYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHT1PIN 10        // what digital pin we're connected to
DHT dht1(DHT1PIN, DHT1TYPE);
// Configure and initialize DHT-22 Sensor 2
#define DHT2TYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHT2PIN 11        // what digital pin we're connected to
DHT dht2(DHT2PIN, DHT2TYPE);


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
#define BACKGROUNDCOLOR 0x0014

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

boolean debug = false;
float humidity1;
float temperature1;
float humidity2;
float temperature2;


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
  Serial.println(GARDUINOVERSION);

  // TFT Output
  tft.setCursor(70, 2);
  tft.setTextColor(GREEN); tft.setTextSize(2);
  tft.print("Garduino v");
  tft.print(GARDUINOVERSION);
  delay(3000); // wait 3 seconds
  tft.setCursor(0, 30);
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.print("Initializing sensors...");
  delay(2000);
  

  /* ------------------------------------------------------------------- */
  /* Sensor initialization */
  
  // Initialize sensor 1 (DHT-22 Temp/Humidity sensor)
  dht1.begin();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity1 = dht1.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature1 = dht1.readTemperature();

  tft.setCursor(20,50);
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.print("Sensor 1 (DHT-22)");
  delay(500);
  tft.setCursor(180,50);
  if (isnan(humidity1) || isnan(temperature1)) {
    // Serial output
    Serial.println("Sensor 1 (DHT-22)    FAIL");
  
    // TFT output
    tft.print("[");
    tft.setTextColor(RED); tft.setTextSize(1);
    tft.print(" FAIL ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");
    return;
    // TODO better: wait for confirmation
  } else {
    // Serial output
    Serial.println("Sensor 1 (DHT-22)    OK");

    // TFT output
    tft.print("[");
    tft.setTextColor(GREEN); tft.setTextSize(1);
    tft.print("  OK  ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");
  }
  delay(2000);
  // Initialize sensor 2 (DHT-22 Temp/Humidity sensor)
  dht2.begin();
  float humidity2 = dht2.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature2 = dht2.readTemperature();  
  
  tft.setCursor(20,60);
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.print("Sensor 2 (DHT-22)");
  delay(500);
  tft.setCursor(180,60);
  if (isnan(humidity2) || isnan(temperature2)) {
    // Serial output
    Serial.println("Sensor 2 (DHT-22)    FAIL");
  
    // TFT output
    tft.print("[");
    tft.setTextColor(RED); tft.setTextSize(1);
    tft.print(" FAIL ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");
    return;
    // TODO better: wait for confirmation
  } else {
    // Serial output
    Serial.println("Sensor 2 (DHT-22)    OK");

    // TFT output
    tft.print("[");
    tft.setTextColor(GREEN); tft.setTextSize(1);
    tft.print("  OK  ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");
  }
  delay(2000);
  tft.fillScreen(BACKGROUNDCOLOR);
}

// Repeated checks go here.
void loop() {
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity1 = dht1.readHumidity();
  float humidity2 = dht2.readHumidity();

  // Read temperature as Celsius (the default)
  float temperature1 = dht1.readTemperature();
  float temperature2 = dht2.readTemperature();

  // Check if any reads failed and exit early (to try again).
  // Sensor 1
  if (isnan(humidity1) || isnan(temperature1)) {
    tft.fillScreen(BLACK);
    tft.setTextColor(RED); tft.setTextSize(2);
    tft.setCursor(70,105);
    tft.print("Failed to read");
    tft.setCursor(30,125);
    tft.print("from sensor 1 (DHT-22)");

    if (debug) {
      Serial.println("Failed to read from sensor 1 (DHT-22)");
    }
     
    delay(5000);
    return;
  }
  // Sensor 2
  if (isnan(humidity2) || isnan(temperature2)) {
    tft.fillScreen(BLACK);
    tft.setTextColor(RED); tft.setTextSize(2);
    tft.setCursor(70,105);
    tft.print("Failed to read");
    tft.setCursor(30,125);
    tft.print("from sensor 2 (DHT-22)");
    
    if (debug) {
      Serial.println("Failed to read from sensor 2 (DHT-22)");
    }
    
    delay(5000);
    return;
  }

  // Print version and copyright
  tft.setCursor(5, 230);
  tft.setTextColor(GREEN); tft.setTextSize(1);
  tft.print("Garduino v");
  tft.println(GARDUINOVERSION);
  tft.setCursor(160, 230);
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.print("(c) 2016 Sebastian Ickler");

  // Temperature and Humidity
  // draw a neat double frame around Temp and Hum.
  tft.drawLine(1,9,319,9,WHITE);   // top outer line
  tft.drawLine(3,11,317,11,WHITE); // top inner line
  tft.drawLine(3,74,317,74,WHITE); // bottom inner line
  tft.drawLine(1,76,319,76,WHITE); // bottom outer line
  tft.drawLine(1,9,1,76,WHITE); // left outer line
  tft.drawLine(3,9,3,74,WHITE); // left inner line
  tft.drawLine(319,9,319,76,WHITE); // right outer line
  tft.drawLine(317,9,317,74,WHITE); // right inner line

  // Clear old values
  tft.fillRect(220, 30, 29, 40, BACKGROUNDCOLOR);
    
  tft.setCursor(8,15);
  tft.print("Temperature and Humidity");  
  tft.drawLine(8,25,150,25,WHITE);
  // Sensor 1
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.setCursor(8,30);
  tft.print("Sensor 1:");
  tft.setCursor(120,30);
  tft.print("Temperature: ");
  tft.setCursor(220,30);
  tft.print(temperature1);
  tft.print("C");

  tft.setCursor(120,40);
  tft.print("Humidity: ");
  tft.setCursor(220,40);
  tft.print(humidity1);
  tft.print("%");

  // Sensor 2
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.setCursor(8,50);
  tft.print("Sensor 2:");
  tft.setCursor(120,50);
  tft.print("Temperature: ");
  tft.setCursor(220,50);
  tft.print(temperature2);
  tft.print("C");

  tft.setCursor(120,60);
  tft.print("Humidity: ");
  tft.setCursor(220,60);
  tft.print(humidity2);
  tft.print("%");

  if (debug){
    Serial.print("Temp[1]: ");
    Serial.print(temperature1);
    Serial.print(" C\t");
    Serial.print("Humi[1]: ");
    Serial.print(humidity1);
    Serial.print("% ");
    Serial.print("\n");
    Serial.print("Temp[2]: ");
    Serial.print(temperature2);
    Serial.print(" C\t");
    Serial.print("Humi[2]: ");
    Serial.print(humidity2);
    Serial.print("% ");
    Serial.print("\n");
  }

  
  // Wait a few seconds between measurements.
  delay(5000);

}
