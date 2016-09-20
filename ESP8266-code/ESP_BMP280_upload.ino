/***************************************************************************
 uses the Adafruit BMP280 library together with my owncode for uploading data to the eXist database.
 
 something odd about the I2C address - here it is set to 0x76 overridding the adafruit default and this is the value echoed on setup but it
 only works if pin 6 is high??
 

 ***************************************************************************/
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#undef BMP280_ADDRESS         // Undef BMP280_ADDRESS from the BMP280 library to easily override I2C address
#define BMP280_ADDRESS (0x76) // Low = 0x76 , High = 0x77 (default on adafruit and sparkfun BME280 modules, default for library)

const char* ssid = "ssid";
const char* password = "password";

const char* host = "kitwallace.co.uk";
const char* streamId   = "stream";
const char* privateKey = "privatekey";

Adafruit_BMP280 bmp; // I2C

WiFiClient client;
int delayMinutes = 1;
int elevation = 60;  // height of station in m

void setup() {
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));
  Serial.println(BMP280_ADDRESS);
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
   connect_wifi();

}

void loop() {
    Serial.print(F("Temperature = "));
    float temp = bmp.readTemperature();
    Serial.print(temp);
    Serial.println(" *C");
    
    Serial.print(F("Raw Pressure = "));
    float pressure= bmp.readPressure()/ 100.0F; 
    Serial.print(pressure);
    Serial.println(" HPa");
    
    float slpressure = pressure + elevation/9.2F;
    Serial.print(F("Sea Level Pressure = "));
    Serial.print(slpressure); 
    Serial.println(" m");
    
    Serial.println();
    String report  = String("pressure=") + slpressure + "&temp="+ temp ;
    Serial.println(report);

    log_data(report);
    delay(delayMinutes * 60 * 1000);
}


void connect_wifi() {
// Connect to WiFi network
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
  
}

void httpget(String url) {
   WiFiClient client;
   url.replace(" ", "+");
   int httpPort = 80;
   while (true) {
    if (client.connect(host, httpPort)) break;
    Serial.println("connection failed");
    delay(100);
  }
  
  Serial.println("connected ");
  Serial.print("requesting URL: ");
  Serial.println(url);
// This will send the request to the server
  String httprequest = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n";
// Serial.println(httprequest);
  client.print(httprequest);
  delay(100);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

void log_data(String params) {
   String url = "/rt/home.xq?_action=store&_id=";
   url += streamId;
   url += String("&_pk=")+ privateKey + "&";
   url += params;
   httpget(url); 
}
