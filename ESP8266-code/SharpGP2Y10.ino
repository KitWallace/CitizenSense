/*
   See http://kitwallace.tumblr.com/post/141890535929/dust-sensor for details

   Kit Wallace March 2016

*/

#include <RunningMedian.h>
#include <ESP8266WiFi.h>

const int dustPin= A0;
const float ADC_REF = 3.3;
const int ADC_MAX = 1024; 
const int ledPower =D1;

// WiFi connection

const char* ssid = "ssid";
const char* password = "wifi";

// Data stream
const char* host = "kitwallace.co.uk";
const char* streamId   = "stream";
const char* privateKey = "pk";

// Sharp timing
int waitToRead = 280;
int waitToTurnOff = 40;
int waitForNext= 9680;

// sampling
int sampleSize = 15;
RunningMedian samples = RunningMedian(sampleSize);

int interval = 3000; 
float lastReading ;
void setup(){
   Serial.begin(9600);
   connectWifi();
   pinMode(ledPower,OUTPUT);
   lastReading = millis();
}
 
void loop(){
   int baseDigital = analogRead(dustPin);
   digitalWrite(ledPower,LOW); // power on the LED
   delayMicroseconds(waitToRead);
   int readDigital =analogRead(dustPin); 
   delayMicroseconds(waitToTurnOff);
   digitalWrite(ledPower,HIGH); // turn the LED off
   int dustDigital = readDigital - baseDigital;
   samples.add(dustDigital);
   if (millis() > lastReading + interval) {
        float dustVoltage = samples.getAverage() * ADC_REF / ADC_MAX;
        String report = String("dustVoltage=")+dustVoltage;
        Serial.println(report);
        logData(report);
        lastReading = millis();
   }
   delayMicroseconds(waitForNext);
}

void connectWifi() {
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

void httpGet(String url) {
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

void logData(String params) {
   String url = "/rt/home.xq?_action=store&_id=";
   url += streamId;
   url += String("&_pk=")+ privateKey + "&";
   url += params;
   httpGet(url); 
}
