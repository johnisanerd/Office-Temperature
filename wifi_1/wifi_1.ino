// This project is a way to turn the A/C and heat on and off in my room.
// The A/C is controlled with a wyze plugs, 
// The hardware is a Arduino MKR system.
// This will check the time, and then the temperature, and decide whether to turn the A/C on and off.
// There's some tuning: finding the temperature levels to turn it on and off.

//  https://docs.arduino.cc/hardware/mkr-env-shield
//  https://docs.arduino.cc/tutorials/mkr-env-shield/mkr-env-shield-basic
//  In-depth wifi documentation on webhooks:  https://help.ifttt.com/hc/en-us/articles/115010230347

//  Hardware isn't ideal:  the env shield can heat up because it's overtop the wifi device. 
//  The wifi chip really heats it up and makes it higher temp than it is.  So mount it pointed vertically.
//  In my case, I'm pointing it upwards, so the sensor isn't on top of the chip, and air flows upward.

#include <WiFiNINA.h>
#include <Arduino_MKRENV.h>
#include "arduino_secrets.h"  // We're keeping our SSID and other sensitive information in this file.
#include <NTPClient.h>
#include <WiFiUdp.h>

// Turning debug on and off.  If it's attached to a serial terminal, ie a computer,
// you can leave debug on (1), if it's not it's important to turn the debug off (ie a standalone device)
// Otherwise the serial gets hung up, and the machine waits for a serial line. 

int debug_on = 0;      // Turns serial debugging on and off.
                       // O is off.  1 is debug on.

// Hours of operation.  This is so it doesn't work overnight and waste my money.
int work_strt = 6;  // Hour that work is starting. 6 PM to get ready for me coming in.
int work_end  = 20;  // Hour that work is ending.  8 PM to get 

// Temperature range.  This is so it doesn't run too hot or cold. 
int temp_high = 26; // Turn on the cool.
int temp_low = 25;  // Turn off the cool.

// NTP server.  We're running the server to get the time, date, etc.
WiFiUDP ntpUDP;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "time-a-g.nist.gov", -14400, 60000);

// Sensitive data in the Secret tab/arduino_secrets.h .  IE the SSID and PASSWORD
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;     // the Wifi radio's status

// Initialize the Ethernet client library.
WiFiSSLClient client;

void setup() {

  // Setup the time client.
  timeClient.begin();

  // Initialize the sensors.
  ENV.begin();
  
  //Initialize serial and wait for port to open.
  // It's important this is optional and not run when run as standalone.
  // Or it's going to sit and wait for the serial to connect.
  if(debug_on){
    Serial.begin(9600);
    while (!Serial);
  };

  // Connect to Wifi network
  while (status != WL_CONNECTED) {
    if(debug_on){
      Serial.print("Attempting to connect to network: ");
      Serial.println(ssid);
      Serial.println(pass);
      // Connect to WPA/WPA2 network:
    };
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data:
  if(debug_on){
    Serial.println("You're connected to the network");
    Serial.println("----------------------------------------");
    printData();
    Serial.println("----------------------------------------");
  };
}

void loop() {
  // Check the temp and time once every 60 seconds.
  // Then manage the temp every minute.

  delay(6000);
  getSensors();  // Print the sensor readings for debugging.
  manage_temperature();
  // day_of_week();
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

  // Is it a weekday?
  // IE is it NOT Sunday (0) or Saturday (6)
  int weekday = day_of_week();
  // Run the logic first.
  if(weekday != 0 && weekday != 6){
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

  // All the other readings are just bullshit, you don't need them for this project.
  /*
  Serial.print("UVA         = ");
  Serial.println(uva);

  Serial.print("UVB         = ");
  Serial.println(uvb);

  Serial.print("UV Index    = ");
  Serial.println(uvIndex);
  */
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

// Turn on the Wyze plug.
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
    client.println("POST /trigger/turn_off/with/key/L1A_QUONyD5v7I84-Hjef HTTP/1.1");
    
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println();
    Serial.println("done");
  }

}

int day_of_week(){
  // NTP has a day of the week function.
  // 0 = Sunday.  6 = Saturday.
  timeClient.update();
  timeClient.getDay();
  Serial.print("Day of the week: ");
  Serial.println(timeClient.getDay());
  return timeClient.getDay();
  
}