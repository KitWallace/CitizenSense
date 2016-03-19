/*
  ESP8266 script to control a 433Mhz power socket from a web-based switch

  Chris Wallace
  March 2016
  
*/
#include <ESP8266WiFi.h>
#include <RCSwitch.h>

#define ON 1
#define OFF 0
#define UNKNOWN -1
// WiFi connection 
const char* ssid = "ssid";
const char* password = "wifipw";

// Data base connection
const char* host = "kitwallace.co.uk";
const char* data_streamId   = "streamid";
const char* data_privateKey = "streampk";

// Pins
int TRANSMIT = 5;


RCSwitch mySwitch = RCSwitch();
//  remote socket setting
int sw_group = 2;
int sw_switch = 1;

char* field = "sw";
String url =  constructURL(field);

// switch status is unknown until we initialise

String lastTimestamp = "";  
int sw_state = UNKNOWN; 

int poll_seconds = 20;

void httpGetLines(String url, String* lines) {
// call the url and fill the array lines  with terminated lines 

   WiFiClient client;
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

   // skip headers - terminated by blank line 
   // should really check response code
   while (client.connected()) {
     String line = client.readStringUntil('\n');
     if (line == "\r") {
       Serial.println("headers received");
       break;
     }
   }
  // read n lines - first will be a character count 
  Serial.println("Reading data");
  int i = 0;
  while (i < sizeof(lines)-1 && client.connected()) {
      String line = client.readStringUntil('\n');
      lines[i] = line;
      Serial.println(line);
      i++;
  }
}

String constructURL(char* field) {
   String url = "/rt/home.xq?_action=value&_format=text&_id=";
   url += data_streamId;
   url += String("&_pk=")+ data_privateKey;
   url += String("&_field=") + field;
   return url;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Switch controller started");
  mySwitch.enableTransmit(TRANSMIT);  // send on pwm pin
}

void loop() {
   Serial.println(String("Current State ") + sw_state);

// poll the URL to get the timestamp and value
   String data[3];
   httpGetLines(url,data);
   String timestamp = data[1];
   String request = data[2];

   int sw_request = request.toInt();     // this defaults to 0 if not an integer
   Serial.println(String("requested ") + sw_request);
  
   if (sw_state == UNKNOWN ||  (timestamp > lastTimestamp)) { // there has been a new command
      if (sw_request == ON)  
          mySwitch.switchOn(sw_group,sw_switch); 
      else if (sw_request == OFF)
          mySwitch.switchOff(sw_group,sw_switch);
      else {
         Serial.println(String("unknown request ") + sw_request);
         return;
      }
      sw_state = sw_request; 
      lastTimestamp = timestamp;
      Serial.println(String("switched to ") + sw_state );
   }
   
   delay(poll_seconds*1000);

}
