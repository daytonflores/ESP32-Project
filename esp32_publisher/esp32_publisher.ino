#include <BlynkSimpleEsp32.h>
#include "ThingSpeak.h"
#include <WiFi.h>
#include <Wire.h>

#define WIFI_SSID "Dayton's iPhone"                     // WiFi network to connect to
#define WIFI_PASSWORD "daytonflores"                    // WiFi network's password to connect to

#define TEMPERATURE_ADDR 0x48                           // I2C address of LM75B
#define TEMPERATURE_REG 0x00                            // LM75B register which holds temperature data
#define TEMPERATURE_CONFIG_REG 0x01                     // LM75B register for configuration & status

#define HUMIDITY_ADDR 0x40                              // I2C address of HDC1080
#define HUMIDITY_REG 0x01                               // HDC1080 register which holds humidity data
#define HUMIDITY_CONFIG_REG 0x02                        // HDC1080 register for configuration & status

#define BRIGHTNESS_ADDR 0x29                            // I2C address of TSL2561
#define BRIGHTNESS_REG_L 0x8C                           // TSL2561 register which holds low byte of brightness data from channel 0 (command byte)
#define BRIGHTNESS_REG_U 0x8D                           // TSL2561 register which holds high byte of brightness data from channel 0 (command byte)

int readingNumber = 0;                                  // used to count reading number

uint8_t temperatureByte[2];                             // 2 unsigned bytes to store temperature data
uint16_t rawTemperature;                                // unsigned short to store the 2 concatenated unsigned bytes of humidity data
int temperatureC;                                       // int value of temperature (celsius)
int temperatureF;                                       // int value of temperature (fahrenheit)

uint8_t humidityByte[2];                                // 2 unsigned bytes to store humidity data
uint16_t rawHumidity;                                   // unsigned short to store the 2 concatenated unsigned bytes of humidity data
int humidity;                                           // int value of humidity

uint8_t brightnessByte[2];                              // 2 unsigned bytes to store brightness data
uint16_t rawBrightness;                                 // unsigned short to store the 2 concatenated unsigned bytes of brightness data
int brightness;                                         // int value of brightness

const int temperatureField = 1;                         // ThingSpeak field 1
const int humidityField = 2;                            // ThingSpeak field 2
const int brightnessField = 3;                          // ThingSpeak field 3
const char * myWriteAPIKey = "GG6DMFE7TYHS6V9J";        // ThingSpeak write API key
unsigned long myChannelID = 931142;                     // ThingSpeak channel ID
WiFiClient client;

const char auth[] = "kWZtsxsj4i7LGldSPnVVVZF4C1XnCe5H"; // authorization token from Blynk App
BlynkTimer timer;

void setup(){
  Serial.begin(9600);                         // initiate serial with baud 9600
  Wire.begin();                               // initiate Wire library & join the I2C bus

  Wire.beginTransmission(TEMPERATURE_ADDR);   // begin a transmission with LM75B
  Wire.write(TEMPERATURE_CONFIG_REG);         // access the configuration register in the LM75B
  Wire.write(0x00);                           // normal operation mode
  Wire.endTransmission();                     // transmit configuration bytes to LM75B
  
  delay(10);                                  // allow 10 ms for LM75B configuration

  Wire.beginTransmission(HUMIDITY_ADDR);      // begin a transmission with HDC1080
  Wire.write(HUMIDITY_CONFIG_REG);            // access the configuration register in the HDC1080
  Wire.write(0x82);                           // software reset, temp or humidity acquired, 8-bit resolution
  Wire.write(0x00);                           // reserved, must be 0
  Wire.endTransmission();                     // transmit configuration bytes to HDC1080
  
  delay(10);                                  // allow 10 ms for HDC1080 configuration

  WiFi.mode(WIFI_STA);                        // WiFi mode set to Station
  connectToWiFi(WIFI_SSID, WIFI_PASSWORD);    // connect to WiFi with credentials

  ThingSpeak.begin(client);                   // initialize ThingSpeak connection with WiFi client
  Blynk.config(auth);                         // initialize Blynk connection with authentication token
  timer.setInterval(15000L, getSensorData);   // run getSensorData() every 15 s
}

void loop(){
  Blynk.run();                                        // maintain connection with Blynk
  timer.run();                                        // run BlynkTimer every 15 s
}

void getSensorData(){
  Wire.beginTransmission(TEMPERATURE_ADDR);           // begin a transmission with LM75B
  Wire.write(TEMPERATURE_REG);                        // access temperature register in LM75B
  Wire.endTransmission();                             // transmit temperature data request to LM75B

  delay(10);                                          // allow 10 ms for temperature data request to be sent

  Wire.requestFrom(TEMPERATURE_ADDR, 2);              // request 2 bytes of data from LM75B
  
  temperatureByte[1] = Wire.read();                   // read MSB of temperature data
  temperatureByte[0] = Wire.read();                   // read LSB of temperature data

  rawTemperature = ((uint16_t)temperatureByte[1] << 8) | (temperatureByte[0] & 0x80); // concatenate [15:8] and [7] of temperature data
  temperatureC = ((float)rawTemperature) * 0.5 / 128;                                 // convert temperature data to celsius
  temperatureF = (temperatureC * 1.8) + 32;
  
  Wire.beginTransmission(HUMIDITY_ADDR);              // begin a transmission with HDC1080
  Wire.write(HUMIDITY_REG);                           // access humidity register in HDC1080
  Wire.endTransmission();                             // transmit humidity data request to HDC1080
  
  delay(10);                                          // allow 10 ms for humidity data request to be sent
  
  Wire.requestFrom(HUMIDITY_ADDR, 2);                 // request 2 bytes of data from HDC1080
   
  humidityByte[1] = Wire.read();                      // read MSB of humidity data
  humidityByte[0] = Wire.read();                      // read LSB of humidity data

  rawHumidity = ((uint16_t)humidityByte[1] << 8) | (humidityByte[0] & 0xFC);          // concatenate [15:8] and [7:2] of humidity data
  humidity = ((float)rawHumidity / 65536) * 100;                                      // convert humidity to relative humidity
  /*
  Wire.beginTransmission(BRIGHTNESS_ADDR);            // begin a transmission with TSL2561
  Wire.write(BRIGHTNESS_REG_L);                       // access lower byte of brightness register in TSL2561
  Wire.endTransmission();                             // transmit brightness data request to TSL2561

  delay(500);                                          // allow 10 ms for brightness data request to be sent

  Wire.requestFrom(BRIGHTNESS_ADDR, 1);               // request 1 byte of data from TSL2561

  brightnessByte[0] = Wire.read();                    // read LSB of brightness data
  
  Wire.beginTransmission(BRIGHTNESS_ADDR);            // begin a transmission with TSL2561
  Wire.write(BRIGHTNESS_REG_U);                       // access upper byte of brightness register in TSL2561
  Wire.endTransmission();                             // transmit brightness data request to TSL2561

  delay(10);                                          // allow 10 ms for brightness data request to be sent

  Wire.requestFrom(BRIGHTNESS_ADDR, 1);               // request 1 byte of data from TSL2561

  brightnessByte[1] = Wire.read();                    // read MSB of brightness data
  
  rawBrightness = brightnessByte[0];//((uint16_t)brightnessByte[1] << 8) | (brightnessByte[0]); // concatenate [15:8] and [7:0] of brightness data
  brightness = (float)rawBrightness;                                        // convert brightness into lux
  */
  Serial.print("Reading no: ");
  Serial.print(readingNumber);
  Serial.print(", Temperature: ");
  Serial.print(temperatureF);
  Serial.print(". Humidity: ");
  Serial.println(humidity);
  //Serial.print("%. Brightness: ");
  //Serial.println(brightness);

  Serial.print("...");
  ThingSpeak.setField(1, temperatureF);               // prepare temperature reading to be sent to ThingSpeak
  ThingSpeak.setField(2, humidity);                   // prepare humidity reading to be sent to ThingSpeak
  //ThingSpeak.setField(3, brightness);                 // prepare brightness reading to be sent to ThingSpeak

  ThingSpeak.writeFields(myChannelID, myWriteAPIKey); // transmit readings to ThingSpeak

  Serial.println(" readings sent to cloud!");
  Serial.println();

  Blynk.virtualWrite(V0, temperatureF);               // send temperature value to Blynk app
  Blynk.virtualWrite(V1, humidity);                   // send humidity value to Blynk app
  //Blynk.virtualWrite(V2, brightness);                 // send brightness value to Blynk app

  readingNumber++;                                    // increment reading number
}

void connectToWiFi(const char * ssid, const char * pwd){
  int ellipsesCounter = 0;                      // used to make output format nice while connecting
 
  Serial.print("Connecting to WiFi network: ");
  Serial.println(WIFI_SSID);                   
  
  WiFi.begin(ssid, pwd);                        // connect to WiFi using given credentials
  
  while(WiFi.status() != WL_CONNECTED){         // while ESP32 remains not connected from WiFi
    if(ellipsesCounter % 10 == 0){              // check if counter is multiple of 10
      Serial.println();                         // Serial println for nice format while connecting
    }
    Serial.print("... ");                       
    ellipsesCounter ++;                         // increment ellipsesCounter
    delay(2000);                                // wait for 2 s
  }
  
  Serial.println("Connected!");
  Serial.println();
}
