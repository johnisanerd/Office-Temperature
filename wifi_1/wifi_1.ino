// This project is a way to turn the A/C and heat on and off in my room.
// Using wyze plugs, and an Arduino MKR system.

//  https://docs.arduino.cc/hardware/mkr-env-shield
//  https://docs.arduino.cc/tutorials/mkr-env-shield/mkr-env-shield-basic
//  In-depth wifi documentation on webhooks:  https://help.ifttt.com/hc/en-us/articles/115010230347

#include <WiFiNINA.h>
#include <Arduino_MKRENV.h>
#include "arduino_secrets.h"  // We're keeping our SSID and other sensitive information in this file.

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiSSLClient client;



void setup() {
  // Initialize the sensors.
  ENV.begin();
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    Serial.println(pass);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  
  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");
}

void loop() {
  // check the network connection once every 10 seconds:

 delay(1000);
 printData();
 getSensors();
 turnOn();
 // turnOff();

 Serial.println("----------------------------------------");
}

void getSensors(){
  // read all the sensor values

  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();
  float illuminance = ENV.readIlluminance();
  float uva         = ENV.readUVA();
  float uvb         = ENV.readUVB();
  float uvIndex     = ENV.readUVIndex();  

  // print each of the sensor values
  
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");
 
  Serial.print("Pressure    = ");
  Serial.print(pressure);
  Serial.println(" kPa");


  Serial.print("Illuminance = ");
  Serial.print(illuminance);
  Serial.println(" lx");

  Serial.print("UVA         = ");
  Serial.println(uva);

  Serial.print("UVB         = ");
  Serial.println(uvb);

  Serial.print("UV Index    = ");
  Serial.println(uvIndex);

  // print an empty line

  Serial.println();

  
}

void printData() {
  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

// turn_off
// https://arduinogetstarted.com/tutorials/arduino-ifttt
// Send the trigger without data:  http://maker.ifttt.com/trigger/EVENT-NAME/with/key/YOUR-KEY
// KEY:  Your key is: xxxxxxx
// EVENT:  turn_off / turn_on

void turnOn() {

  char url[200];

  char host[] = "maker.ifttt.com";
  char server[] = "POST /trigger/turn_on/with/key/"; 
  char your_key[] = SECRET_KEY;
  strcpy(url, server);
  strcpy(url + strlen(server), your_key);

  Serial.print("URL: ");
  Serial.println(host);

  Serial.print("COMMAND: ");
  Serial.println(url);

  if (client.connect(host, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println(url);
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println();
    Serial.println("done");
  }
}

void turnOff() {

  char url[200];

  char host[] = "maker.ifttt.com";
  char server[] = "POST /trigger/turn_off/with/key/"; 
  char your_key[] = SECRET_KEY;
  strcpy(url, server);
  strcpy(url + strlen(server), your_key);

  Serial.print("URL: ");
  Serial.println(host);

  Serial.print("COMMAND: ");
  Serial.println(url);

  if (client.connect(host, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println(url);
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println();
    Serial.println("done");
  }


}
