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
 * [x] Output to TFT
 * [ ] Improve TFT output (if touched => screen on, after 30 seconds no touch => screen off)
 * [ ] Include Wifi-Module
 * [ ] Submit sensor data to ThingSpeak
 */

#define GARDUINOVERSION 0.30

// Libraries
#include <Wire.h>            // I2C library
#include "RTClib.h"          // DS1307 RealTimeClock library
#include <DHT.h>             // DHT sensor library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>     // Adafruit Touchscreen library

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

// Configuration for the TFT display
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

// This is calibration data for the raw touch data to the screen coordinates
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// Define min and max pressure for TouchScreen
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

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

/* -------------------------------------------------------------- */
/*                User-defined defines/variables                  */
// Temperature control settings
#define DEFAULTTEMPONBOOT 25

// Light control settings
// Set start of the lights on phase
#define LIGHTONHOUR 9
#define REFRESH 3000
/* -------------------------------------------------------------- */

// Initialize variables
boolean debug = true;
float humidity1;
float temperature1;
float humidity2;
float temperature2;
float avgtemperature;
float avghumidity;
int settemperature = DEFAULTTEMPONBOOT;
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
  pinMode(13, OUTPUT);

  // Initialize I2C
  Wire.begin();
  //relaisWrite(B11111111); // Power down all relais ports
  
  // Initialize RTC
  //RTC.begin();
  
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
  delay(1000); // wait 1 second

  /* ------------------------------------------------------------------- */
  /* I2C initialization */

  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.setCursor(0, 30);
  tft.print("Initializing I2C devices...");

  // 4-port relais on 0x38
  // Deactivate all ports
  relaisWrite(relaisdefault);
  /*
  tft.setCursor(180,40);
  if (! RTC.isrunning()) {
    // Serial output
    Serial.println("RTC    FAIL");
    tft.setCursor(0, 30);
    tft.print("Real Time Clock");
  
    // TFT output
    tft.print("[");
    tft.setTextColor(RED); tft.setTextSize(1);
    tft.print(" FAIL ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");

    // following line sets the RTC to the date & time this sketch was compiled
    // RTC.adjust(DateTime(__DATE__, __TIME__));
    
  } else {
    // Serial output
    Serial.println("RTC    OK");

    // TFT output
    tft.print("[");
    tft.setTextColor(GREEN); tft.setTextSize(1);
    tft.print("  OK  ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");
  }  
  
  delay(500);
  */

  /* ------------------------------------------------------------------- */
  /* Sensor initialization */
  
  delay(1000);
  tft.setCursor(0, 60);
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.print("Initializing sensors...");
  
  // Initialize sensor 1 (DHT-22 Temp/Humidity sensor)
  delay(1000);
  dht1.begin();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity1 = dht1.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature1 = dht1.readTemperature();

  tft.setCursor(20,70);
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.print("Sensor 1 (DHT-22)");
  delay(500);
  tft.setCursor(180,70);
  if (isnan(humidity1) || isnan(temperature1)) {
    // Serial output
    Serial.println("Sensor 1 (DHT-22)    FAIL");
  
    // TFT output
    tft.print("[");
    tft.setTextColor(RED); tft.setTextSize(1);
    tft.print(" FAIL ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");
    //return;
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

  // Initialize sensor 2 (DHT-22 Temp/Humidity sensor)
  delay(1000);
  dht2.begin();
  float humidity2 = dht2.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature2 = dht2.readTemperature();  
  
  tft.setCursor(20,80);
  tft.setTextColor(WHITE); tft.setTextSize(1);
  tft.print("Sensor 2 (DHT-22)");
  delay(500);
  tft.setCursor(180,80);
  if (isnan(humidity2) || isnan(temperature2)) {
    // Serial output
    Serial.println("Sensor 2 (DHT-22)    FAIL");
  
    // TFT output
    tft.print("[");
    tft.setTextColor(RED); tft.setTextSize(1);
    tft.print(" FAIL ");
    tft.setTextColor(WHITE); tft.setTextSize(1);
    tft.print("]");
    //return;
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
  
  /* TouchScreen */
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    
    if (debug) {
      Serial.print("X = "); Serial.print(p.x);
      Serial.print("\tY = "); Serial.print(p.y);
      Serial.print("\tPressure = "); Serial.println(p.z);
    }
        
    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
    if (debug) {
      Serial.print("("); Serial.print(p.x);
      Serial.print(", "); Serial.print(p.y);
      Serial.println(")");
    }
  }
  /* -------------------- BEGIN Temperature/Humidity ------------------- */
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity1 = dht1.readHumidity();
  float humidity2 = dht2.readHumidity();

  // Read temperature as Celsius (the default)
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
  tft.drawLine(1,9,319,9,WHITE);    // top outer line
  tft.drawLine(3,11,317,11,WHITE);  // top inner line
  tft.drawLine(3,74,317,74,WHITE);  // bottom inner line
  tft.drawLine(1,76,319,76,WHITE);  // bottom outer line
  tft.drawLine(1,9,1,76,WHITE);     // left outer line
  tft.drawLine(3,9,3,74,WHITE);     // left inner line
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

  /* --------------------- END Temperature/Humidity -------------------- */

  /* -------------------- BEGIN Temperature control -------------------- */

  tft.drawLine(1,78,319,78,WHITE);    // top outer line
  tft.drawLine(3,80,317,80,WHITE);    // top inner line
/*  tft.drawLine(3,74,317,74,WHITE);  // bottom inner line
  tft.drawLine(1,76,319,76,WHITE);  // bottom outer line
  tft.drawLine(1,9,1,76,WHITE);     // left outer line
  tft.drawLine(3,9,3,74,WHITE);     // left inner line
  tft.drawLine(319,9,319,76,WHITE); // right outer line
  tft.drawLine(317,9,317,74,WHITE); // right inner line
*/

  // Show set temperature and buttons
  int button_w = 40;
  int button_h = 40;
  int button_corner_radius = button_w / 8;  
  tft.setCursor(13,147);
  tft.setTextColor(WHITE); tft.setTextSize(3);
  tft.print(settemperature);
  // Increase temperature
  // Draw a "+" in a white circle
//  tft.drawCircle(30, 118, 20, WHITE);
  int button_plus_x = 10;
  int button_plus_y = 100;
  tft.drawRoundRect(button_plus_x, button_plus_y, button_w, button_h, button_corner_radius, WHITE);  
  tft.setCursor(20,105); tft.setTextSize(4); tft.print("+");
  // Decrease temperature
  // Draw a "-" in a white circle
  int button_minus_x = 10;
  int button_minus_y = 175;
//  tft.drawCircle(button_minux_x, button_minus_y, button_minus_r, WHITE);
  tft.drawRoundRect(button_minus_x, button_minus_y, button_w, button_h, button_corner_radius, WHITE);
  tft.setCursor(20,180); tft.setTextSize(4); tft.print("-");

  // See if there's any  touch data for us
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {   
    // Scale using the calibration #'s
    // and rotate coordinate system
    p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
    p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
    int y = tft.height() - p.x;
    int x = p.y;
    Serial.print(x);Serial.print(",");Serial.print(y);
    if (tempPlus)
    {
      if((x > button_plus_x) && (x < (button_plus_x + button_w))) {
        if ((y > button_plus_y) && (y <= (button_plus_y + button_h))) {
          Serial.println("Temperature + pressed"); 
          tft.fillRoundRect(button_minus_x, button_minus_y, button_w, button_h, button_corner_radius, WHITE);
          tft.setCursor(20,180); tft.setTextSize(4); tft.setTextColor(BACKGROUNDCOLOR); tft.print("-");
          //redBtn();
          settemperature += 1;
        }
      }
    }
/*
    else //Record is off (RecordOn == false)
    {
      if((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W))) {
        if ((y > GREENBUTTON_Y) && (y <= (GREENBUTTON_Y + GREENBUTTON_H))) {
          Serial.println("Green btn hit"); 
          greenBtn();
        }
      }
    }

    Serial.println(RecordOn);
*/
  }  
  /* --------------------- END Temperature Control ---------------------- */

  /* ----------------------- BEGIN Light control ----------------------- */
  // Light control goes here
  //DateTime lightofftime (now.unixtime() + lightontime * 3600L);
  /* ------------------------ END Light control ------------------------ */

  /* ------------------------ BEGIN Fan control ------------------------ */
  // Fan control goes here
  /* ------------------------- END Fan control ------------------------- */
  
  /* ---------------------- BEGIN Exhaust control ---------------------- */
  // Exhaust control goes here
  /* ----------------------- END Exhaust control ----------------------- */
  // repeat the loop every 30 seconds
  delay(REFRESH);

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

