/*
 * Garduino
 * Environment surveillance for greenhouses
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
 * [x] Output to LCD
 * [ ] Include Wifi-Module
 * [ ] Submit sensor data to ThingSpeak
 */

#define GARDUINOVERSION 0.40

// Libraries
#include <Wire.h>            // I2C library
#include "RTClib.h"          // DS1307 RealTimeClock library
#include <DHT.h>             // DHT sensor library
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// Configure 20x4 LCD over I2C bus
LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified module

// Configure Real-Time-Clock
RTC_DS1307 RTC;

// Configure and initialize DHT-22 Sensor 1
#define DHT1TYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHT1PIN 10        // what digital pin we're connected to
DHT dht1(DHT1PIN, DHT1TYPE);
// Configure and initialize DHT-22 Sensor 2
#define DHT2TYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHT2PIN 11        // what digital pin we're connected to
DHT dht2(DHT2PIN, DHT2TYPE);

// 4-port relais on first I2C expansion board
#define I2C_RELAIS_ADDRESS 0x38
// Set the bitmasks for connected devices
const int relaisdefault = 255; // everything deactivated
const int light = 1;           // P0
const int fan1 = 2;            // P1
const int fan2 = 4;            // P2
const int exhaust = 8;         // P3

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

/* -------------------------------------------------------------- */
/*                User-defined defines/variables                  */
// Temperature control settings
// default temperature setting on boot
int settemperature = 25;
// refresh rate of the dht sensors in milliseconds
const int dhtsensordelay = 5000;

/* -------------------------------------------------------------- */

// Initialize variables
boolean debug = true;
float humidity1;
float temperature1;
float humidity2;
float temperature2;
float avgtemperature;
float avghumidity;
int lightontime;
int lightphase;
boolean tempPlus = false;
boolean tempMinus = false;

// Initial program
void setup() {

  // Light phases
  // 1: 12 on / 12 off
  // 2: 18 on /  6 off
  // 3: 17 on /  7 off
  switch (lightphase) {
    case 1:        // 12/12 for growing
      lightontime = 12;
    break;
    case 2:   // 18/6 for flowering
      lightontime = 18;
    break;
    case 3:   // 17/7 for enhanced flowering
      lightontime = 17;
    break;
  }

  // Initialize serial console
  Serial.begin(9600);

  // Initialize I2C
  Wire.begin();

  // Initialize LCD
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH); // NOTE: You can turn the backlight off by setting it to LOW instead of HIGH
  lcd.begin(20, 4);
  lcd.clear();  
  
  // Initialize RTC
  //RTC.begin();
  
  // Initialization procedure
  // Serial output
  Serial.print("Garduino v");
  Serial.println(GARDUINOVERSION);

  // LCD Output
  lcd.setCursor(3, 0);
  lcd.print("Garduino v");
  lcd.print(GARDUINOVERSION);
  lcd.setCursor(6,2);
  lcd.print("(c) 2016");
  lcd.setCursor(2,3);
  lcd.print("Sebastian Ickler");
  delay(3000); // wait 3 seconds
  lcd.clear();
  /* ------------------------------------------------------------------- */
  /* I2C initialization */

  // 4-port relais on 0x38
  // Deactivate all ports
  relaisWrite(relaisdefault);

 /*

    // following line sets the RTC to the date & time this sketch was compiled
    // RTC.adjust(DateTime(__DATE__, __TIME__));
    
  } else {
    // Serial output
    Serial.println("RTC    OK");
  }  
  
  delay(500);
  */
  
  /* ------------------------------------------------------------------- */
  /* Sensor initialization */
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensors:");
  
  // Initialize sensor 1 (DHT-22 Temp/Humidity sensor)
  delay(1000);
  dht1.begin();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity1 = dht1.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature1 = dht1.readTemperature();

  lcd.setCursor(0,1);
  lcd.print("DHT1:");
  lcd.setCursor(12,1);
  if (isnan(humidity1) || isnan(temperature1)) {
    // Serial output
    Serial.println("Sensor 1 (DHT-22)    FAIL");
    lcd.print("[ FAIL ]");
    // TODO better: wait for confirmation
  } else {
    // Serial output
    Serial.println("Sensor 1 (DHT-22)    OK");
    lcd.print("[  OK  ]");
  }
  
  // Initialize sensor 2 (DHT-22 Temp/Humidity sensor)
  delay(1000);
  dht2.begin();
  float humidity2 = dht2.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature2 = dht2.readTemperature();  
  
  lcd.setCursor(0,2);
  lcd.print("DHT2:");
  lcd.setCursor(12,2);
  if (isnan(humidity2) || isnan(temperature2)) {
    // Serial output
    Serial.println("Sensor 2 (DHT-22)    FAIL");
    lcd.print("[ FAIL ]");
    // TODO better: wait for confirmation
  } else {
    // Serial output
    Serial.println("Sensor 2 (DHT-22)    OK");
    lcd.print("[  OK  ]");
    // LCD output
  }
  delay(2000);
  lcd.clear();
}

// Repeated checks go here.
void loop() {
  // Time and Date...
  /*
  DateTime now = RTC.now();
  int hour = now.hour();
  Serial.println(now.minute(), DEC);
  if (debug) {
    Serial.print(now.year(), DEC); Serial.print('/');
    Serial.print(now.month(), DEC); Serial.print('/');
    Serial.print(now.day(), DEC); Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]); Serial.print(") ");
    Serial.print(now.hour(), DEC); Serial.print(':');
    Serial.print(now.minute(), DEC); Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
  */
  
  /* -------------------- BEGIN Temperature/Humidity ------------------- */
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  if ((millis() % dhtsensordelay) == 0) {
    float humidity1 = dht1.readHumidity();
    float humidity2 = dht2.readHumidity();
  
    float temperature1 = dht1.readTemperature();
    float temperature2 = dht2.readTemperature();
  
    // Calculate average temperature and humidity
    float avgtemperature = (temperature1 + temperature2) / 2;
    float avghumidity = (humidity1 + humidity2) / 2;
    
    // Check if any reads failed and exit early (to try again).
    // Sensor 1
    if (isnan(humidity1) || isnan(temperature1)) {
      temperature1 = 0000;
      humidity1    = 0000;
    }
    // Sensor 2
    if (isnan(humidity2) || isnan(temperature2)) {
      temperature2 = 0000;
      humidity2    = 0000;
    }
    // LCD Output
    lcd.clear();
    // Head row
    lcd.setCursor(6,0);
    lcd.print("Temp");
    lcd.setCursor(15,0);
    lcd.print("Luft");
  
    //First sensor
    lcd.setCursor(0,1);
    lcd.print("S1");
    lcd.setCursor(5,1);
    lcd.print(temperature1);
    lcd.print("C");
    lcd.setCursor(14,1);
    lcd.print(humidity1);
    lcd.print("%");
  
  
    lcd.setCursor(0,2);
    lcd.print("S2");
    lcd.setCursor(5,2);
    lcd.print(temperature2);
    lcd.print("C");
    lcd.setCursor(14,2);
    lcd.print(humidity2);
    lcd.print("%");
    
    lcd.setCursor(0,3);
    lcd.print("Avg");
    lcd.setCursor(5,3);
    lcd.print(avgtemperature);
    lcd.print("C");
    lcd.setCursor(14,3);
    lcd.print(avghumidity);
    lcd.print("%");  
    
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
  }
  /* --------------------- END Temperature/Humidity -------------------- */

  /* -------------------- BEGIN Temperature control -------------------- */

    if (tempPlus) {
          if (debug) {
            Serial.println("Temperature + pressed"); 
          }
          settemperature += 1;
    }  

    if (tempMinus) {
          if (debug) {
            Serial.println("Temperature - pressed"); 
          }
          settemperature -= 1;
    }      
  /* --------------------- END Temperature Control ---------------------- */

  /* ----------------------- BEGIN Light control ----------------------- */
  // Light control goes here
  //DateTime lightofftime (now.unixtime() + lightontime * 3600L);
  /* ------------------------ END Light control ------------------------ */

  /* ------------------------ BEGIN Fan control ------------------------ */
  // Fan control goes here
  /* Example
  int relaisStatus = relaisRead();
  //Serial.println(relaisStatus);
  if (temperature1 >= 23.00) {
     activateRelaisPort(fan1);
  }
  if (temperature1 >= 25.00){
    activateRelaisPort(fan2);
  } else {
    deactivateRelaisPort(fan2);
  }
   */
  /* ------------------------- END Fan control ------------------------- */
  
  /* ---------------------- BEGIN Exhaust control ---------------------- */
  // Exhaust control goes here
  /* ----------------------- END Exhaust control ----------------------- */

}

void relaisWrite(byte txData) {
  Wire.beginTransmission(I2C_RELAIS_ADDRESS);
  Wire.write(txData);
  Wire.endTransmission();
}

int relaisRead() {
  Wire.requestFrom(I2C_RELAIS_ADDRESS, 1);
  while(Wire.available())    // slave may send less than requested
  { 
    int i = Wire.read();    // receive a byte as integer
    return i; 
  }
}

void activateRelaisPort(int portbit) {
  int relaisStatus = relaisRead();
  if ((relaisStatus & portbit) == portbit) {
    if (debug) {
      Serial.print("Activating relais portbit ");
      Serial.println(portbit);
    }
    relaisWrite(relaisStatus - portbit);  // "0" on relais means "on"
  }
}

void deactivateRelaisPort(int portbit) {
  int relaisStatus = relaisRead();
  if ((relaisStatus & portbit) != portbit) {
    if (debug) {
      Serial.print("Deactivating relais portbit ");
      Serial.println(portbit);
    }
    relaisWrite(relaisStatus + portbit);  // "1" on relais means "off"
  }
}

