// This project is a way to turn the A/C and heat on and off in my room.
// Using wyze plugs, and an Arduino MKR system.

//  https://docs.arduino.cc/hardware/mkr-env-shield
//  https://docs.arduino.cc/tutorials/mkr-env-shield/mkr-env-shield-basic
//  In-depth wifi documentation on webhooks:  https://help.ifttt.com/hc/en-us/articles/115010230347

#include <WiFiNINA.h>
#include <Arduino_MKRENV.h>
#include "arduino_secrets.h"  // We're keeping our SSID and other sensitive information in this file.
#include <NTPClient.h>
#include <WiFiUdp.h>

int work_strt = 6;  // Hour that work is starting. 6 PM to get ready for me coming in.
int work_end  = 20;  // Hour that work is ending.  8 PM to get 

int temp_high = 24; // Turn on the cool.
int temp_low = 23;  // Turn off the cool.

WiFiUDP ntpUDP;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "time-a-g.nist.gov", -14400, 60000);

bool debug_serial_monitor_on = true;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiSSLClient client;

void setup() {

  // Setup the time client.
  timeClient.begin();

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
  // once every 60 seconds:

  delay(6000);
  getSensors();  // Print the sensor readings for debugging.
  manage_temperature();

 Serial.println("----------------------------------------");
}

// Simply read the temperature and return it.  
int getTemperature(){
  float temperature = ENV.readTemperature();
  return int(temperature);
}

// Returns time in hours.

int getTime(){
  timeClient.update();

  // Serial.println(timeClient.getFormattedTime());
  Serial.print("Time in hours: ");
  Serial.println(timeClient.getHours());
  // We really only need the hour, so we can roughly know when we
  // are turning the temperature up and down.  

  return timeClient.getHours();
}

// Function will check the time, temperature, and turn the A/C on and off.
void manage_temperature(){

  // Get the time, in hour.
  int current_hour = getTime();
  // Get the temp.
  int current_temp = getTemperature();
  Serial.print("Current temperature is: ");
  Serial.println(current_temp);

  // Are we between the time to start and time to end?
  if((current_hour > work_strt) && (current_hour < work_end)){
    // Are we greater than the temperature?
    if(current_temp > temp_high){
      // Turn the AC on.
      Serial.println("Higher than threshold, turn the AC on.");
      turnOn();
    }
    else{
      // Do nothing.  
      Serial.println("In time range.  Temp is not higher than threshold temp.  Doing nothin.");
    };
    if(current_temp <= temp_low){
      // Turn the AC off.  
      Serial.println("Current temp lower than threshold, turn the AC off.");
      turnOff();
    }
  }
  else{
    // Outside the working hours.  
    // Turn the AC off.
    Serial.println("Turning the AC off.  Outisde work time.  Turn AC off.");
    turnOff();
  }

  // If the temperature is above threshold, turn the A/C on.
  // If the temperature is below threshold, turn the A/C off.  
  // If the tempearture is in the range, leave the AC be.
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
    // client.println(url);

    // HTTP/1.1
    client.println("POST /trigger/turn_on/with/key/L1A_QUONyD5v7I84-Hjef HTTP/1.1");
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
    // client.println(url);

    // HTTP/1.1
    client.println("POST /trigger/turn_off/with/key/L1A_QUONyD5v7I84-Hjef HTTP/1.1");
    
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println();
    Serial.println("done");
  }


}
