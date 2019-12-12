#include "ThingSpeak.h"
#include <WiFi.h>

#define WIFI_SSID "Dayton's iPhone"             // WiFi network to connect
#define WIFI_PASSWORD "daytonflores"            // WiFi network's password to connect to

/**********************************************/
//             EDIT THESE VALUES              //
/**********************************************/
const int temperatureMin = 65;                  // user-defined low threshold for temperature reading from cloud
const int temperatureMax = 75;                  // user-defined high threshold for temperature reading from cloud
const int humidityMin = 20;                     // user-defined low threshold for humidity reading from cloud
const int humidityMax = 30;                     // user-defined high threshold for humidity reading from cloud
const int brightnessMin = 20;                   // user-defined low threshold for brightness reading from cloud
const int brightnessMax = 25;                   // user-defined high threshold for brightness reading from cloud
/**********************************************/
const int ledGreen = 0;                         // green LED is connected to Pin 0
const int ledRed = 32;                          // red LED is connected to Pin 32

int temperatureFromCloud;                       // used to store most recent temperature reading from cloud
int humidityFromCloud;                          // used to store most recent humidity reading from cloud
int brightnessFromCloud;                        // used to store most recent brightness reading from cloud

int readingNumber = 0;                          // used to count reading number

const int temperatureField = 1;                 // ThingSpeak field 1
const int humidityField = 2;                    // ThingSpeak field 2
const int brightnessField = 3;                  // ThingSpeak field 3
const char * myReadAPIKey = "CZ6BWKWYLUO28Y3C"; // ThingSpeak read API key
unsigned long myChannelID = 931142;             // ThingSpeak channel ID

WiFiClient client;                              // initialize ThingSpeak connection with WiFi client

void setup(){
  pinMode(ledGreen, OUTPUT);                // set Pin 0 as an output
  pinMode(ledRed, OUTPUT);                  // set Pin 32 as an output
    
  Serial.begin(9600);                       // initialize serial with baud 9600
  
  WiFi.mode(WIFI_STA);                      // WiFi mode set to Station
  
  digitalWrite(ledGreen, LOW);              // keep both LEDs off until connection to WiFi has been made
  digitalWrite(ledRed, LOW);                // keep both LEDs off until connection to WiFi has been made
    
  connectToWiFi(WIFI_SSID, WIFI_PASSWORD);  // connect to WiFi with credentials

  ThingSpeak.begin(client);                 // initialize ThingSpeak connection with WiFi client
}

void loop(){
  Serial.println("Reading from cloud...");
  
  temperatureFromCloud = ThingSpeak.readIntField(myChannelID, temperatureField, myReadAPIKey);  // store most recent temperature from ThingSpeak
  humidityFromCloud = ThingSpeak.readIntField(myChannelID, humidityField, myReadAPIKey);        // store most recent humidity from ThingSpeak 
  //brightnessFromCloud = ThingSpeak.read(myChannelID, brightnessField, myReadAPIKey);            // store most recent brightness from ThingSpeak

  Serial.print("Reading no: ");
  Serial.print(readingNumber);
  Serial.print(", Temperature: ");
  Serial.print(temperatureFromCloud);
  Serial.print(". Humidity: ");
  Serial.println(humidityFromCloud);
  //Serial.print("%. Brightness: ");
  //Serial.println(brightnessFromCloud);
  Serial.println("\n");

  if( (temperatureMin < temperatureFromCloud) &&    // check if temperature reading from cloud is over the low threshold
      (temperatureFromCloud < temperatureMax) &&    // check if temperature reading from cloud is under the high threshold
      (humidityMin < humidityFromCloud) &&          // check if humidity reading from cloud is over the low threshold
      (humidityFromCloud < humidityMax)){ //&&      // check if humidity reading from cloud is under the high threshold
      //(brightnessMin < brightnessFromCloud) &&      // check if brightness reading from cloud is over the low threshold
      //(brightnessFromCloud < brightnessMax)){       // check if brightness reading from cloud is under the high threshold
    digitalWrite(ledGreen, HIGH);                   // turn on green LED to let user know all parameters have been met
    digitalWrite(ledRed, LOW);                      // turn off red LED to let user know all parameters have been met
  }
  else{                                             // otherwise
    digitalWrite(ledGreen, LOW);                    // turn off green LED to let user know all parameters have been met
    digitalWrite(ledRed, HIGH);                     // turn on red LED to let user know all parameters have been met
  }

  readingNumber++;                                  // increment reading number

  delay(15000);                                     // wait for 15 s (ThingSpeak cannot accept data more than once every 10 s)
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

  for(int i = 0; i < 20; i++){                  // once connected to WiFi, enter loop
    digitalWrite(ledGreen, LOW);                // blink both LEDs off
    digitalWrite(ledRed, LOW);                  // blink both LEDs off
    delay(50);                                  // wait 50 ms
    digitalWrite(ledGreen, HIGH);               // blink both LEDs on
    digitalWrite(ledRed, HIGH);                 // blink both LEDs on
    delay(50);                                  // wait 50 ms
  }

  digitalWrite(ledGreen, HIGH);                 // turn on both LEDs
  digitalWrite(ledRed, HIGH);                   // turn on both LEDs
  delay(1000);                                  // for 1 s
  digitalWrite(ledGreen, LOW);                  // initialize both LEDs off after alerting user connection to WiFi has been made
  digitalWrite(ledRed, LOW);                    // initialize both LEDs off after alerting user connection to WiFi has been made
  
  Serial.println("Connected!");
  Serial.println();
}
