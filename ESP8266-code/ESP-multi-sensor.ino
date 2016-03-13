/*
Multi sensor setup on an ESP8266 D1 mini board

Author Chris Wallace
March 2016


*/

#include <ESP8266WiFi.h>
#include <SparkFunTSL2561.h>
#include <Wire.h>
#include "DHT.h"


const char* ssid = "ssid";
const char* password = "wifipw";
const char* host = "kitwallace.co.uk";
const char* pir_streamId   = "stream id";
const char* pir_privateKey = "stream pk";

// PIR 
const int pir_pin = 2;
boolean pir_active ;

// luminosity

SFE_TSL2561 light;
boolean gain = 0;     // Gain setting, 0 = X1, 1 = X16;
unsigned char itime = 2;
  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop to perform your own integration
unsigned int ms;  // Integration ("shutter") time in milliseconds 
 
// DHT22 temp/humidity
#define DHTPIN 14
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);
float humidity;
float temperature;

int seconds(long ms) {
   return round(ms/1000); 
}

// Return RSSI or 0 if target SSID not found
int32_t getRSSI(const char* target_ssid) {
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++) {
    if (WiFi.SSID(network)== String(target_ssid)) {
      return WiFi.RSSI(network);
    }
  }
  return 0;
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
   url += pir_streamId;
   url += String("&_pk=")+ pir_privateKey + "&";
   url += params;
   httpget(url); 
}

void setup () {
  Serial.begin(9600);
  delay(10);
  
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

// PIR
  
  pinMode(pir_pin,INPUT);
  Serial.println(String("PIR on pin ") + pir_pin);

// luminosity
  light.begin();
  light.setTiming(gain,itime,ms);
  light.setPowerUp();
  Serial.println(String("Luminosity gain=")+gain+" integration time= " + ms+"ms");

// DHT
  dht.begin();
  Serial.println("DHT started");
  
  delay(60*1000);  //PIR initialization
}

void loop() {

    long now = seconds(millis());

// read PIR
    pir_active= digitalRead(pir_pin);     
    Serial.println(String("sec=") + now + " PIR " + pir_active);

// read luminosity
   unsigned int data0, data1;
   double lux;    // Resulting lux value
   boolean good;  // True if neither sensor is saturated
   
  
   if (light.getData(data0,data1))
    {
    // getData() returned true, communication was successful
    
    Serial.println(String("data0= ")+data0 + " data1="+data1);
       
    // Perform lux calculation:

    good = light.getLux(gain,ms,data0,data1,lux);
    
    // Print out the results:
  
    Serial.print(" lux: ");
    Serial.print(lux);
    if (good) Serial.println(" (good)"); else Serial.println(" (BAD)");

    }
// RSSI for this AP  
 
    int32_t rssi = getRSSI(ssid);
 
// Read the DHT22 sensor

  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  int tries = 3;
  while((isnan(humidity) || isnan(temperature) && tries > 0) ) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    humidity = dht.readHumidity();
    temperature= dht.readTemperature();
    tries --;
  }
   Serial.println(String("temperature=") + temperature +" humidity="+humidity);

// log the data
   log_data(String("pir=") + pir_active+"&lux="+lux+"&rssi="+rssi+"&temperature="+temperature+"&humidity="+humidity);   

   delay(60*1000);
}    
