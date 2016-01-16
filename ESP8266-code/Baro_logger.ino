/*
 *  This sketch sends data via HTTP GET request to my server
 *  Chris Wallace 
*/

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

int delayMinutes = 1;
int elevation = 50;  // height of Morely Sq in m

const char* ssid = "ssid";
const char* password = "password";

const char* host = "kitwallace.co.uk";
const char* streamId   = "streamid";
const char* privateKey = "pk";

float pressure = 0;
float temp = 0;

void setup() {
 /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  Serial.begin(9600);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  while (true) {
    if (client.connect(host, httpPort)) break;
    Serial.println("connection failed");
    delay(100);
  }
  Serial.println("connected");
  // get data

  bmp.getPressure(&pressure);
  bmp.getTemperature(&temp);
  float pressure_mb = pressure/100;
  float SL_pressure = pressure_mb + elevation/9.2;
  
// We now create a URI for the request
   String url = "/rt/home.xq?_action=store&_id=";
   url += streamId;
   url += "&_pk=";
   url += privateKey;
   url += "&temp=";
   url += temp;
   url += "&pressure=";
   url += SL_pressure;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(100);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  ESP.deepSleep(delayMinutes*60*1000000);
}

void loop() {}
