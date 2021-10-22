/***************************************************
  LED Website Indicator by the Circus Scientist - a visual indicator of visitors to your WordPress webssites
  This ketch adapted from original code by Adafruit
  Visit http://ledindicator.devsoft.co.za for more details on how this works
  This code was tested on the D1 Mini only
  Original Adafruit notice continues below: 

  **************************************************

  
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
 
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#define led D4 //built in LED on my D1 mini
ESP8266WiFiMulti WiFiMulti;
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Your-SSID"             // replace this with your own WiFi details
#define WLAN_PASS       "password"              // replace this with your own WiFi details

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "ledmqtt.devsoft.co.za"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL - not set up on the server as yet though
#define AIO_USERNAME    "asdfasdfas"           // replace this with your own from ledindicator.devsoft.co.za (username)
#define AIO_KEY         "ghjkghjkgh"           // replace this with your own from ledindicator.devsoft.co.za (password)
//define subs - username/LED for free, the others are for paid version: 
#define LED_SUB         "asdfasdfas/LED"


/******************************************************* you don't need to change anything below this *************************************************/


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/


// Setup a feed called 'LEDsignal' for subscribing to changes.
Adafruit_MQTT_Subscribe LEDsignal = Adafruit_MQTT_Subscribe(&mqtt, LED_SUB);

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);
  //initialise LED:
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH); //HIGH is off for D1 mini

  Serial.println(F("Welcome to LED Website Indicator"));
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(100);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WLAN_SSID, WLAN_PASS);
}

uint32_t x = 0;

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    WiFiClient client;
    // Setup MQTT subscription for onoff feed.
    mqtt.subscribe(&LEDsignal);

    // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.
    MQTT_connect();

    // this is our 'wait for incoming subscription packets' busy subloop
    // try to spend your time here

    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) {
      if (subscription == &LEDsignal) {
        Serial.print(F("Got: "));
        Serial.println((char *)LEDsignal.lastread);
        digitalWrite(led, LOW); //LOW is on for LED_BUILTIN
        delay(1000); //todo: delay time is a variable, per subscription
        digitalWrite(led, HIGH);
      }

      

      // ping the server to keep the mqtt connection alive
      
      if (! mqtt.ping()) {
        mqtt.disconnect();
      } else {
//        Serial.println("ping"); //for debugging purposes
      } 

    }
  }
}
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
//    Serial.println("tried connecting, but already connected");
    if (! mqtt.ping()) {
        mqtt.disconnect();
      } else {
//        Serial.println("ping");
      }
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 8;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");

} 
