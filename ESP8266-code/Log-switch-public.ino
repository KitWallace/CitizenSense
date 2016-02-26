#include <ESP8266WiFi.h>
const char* ssid = "myssid";
const char* password = "wifipw";

const char* host = "kitwallace.co.uk";
const char* streamId   = "streamid1";
const char* privateKey = "pk";

const int pin = 4;

boolean isOpen = false;
boolean stateChanged = false;
unsigned long startMillis = 0;
const long debounce_interval = 3000;  //3 seconds is OK for a door opening

void switchChange() {
// ignore changes within debounce_interval
    unsigned long now = millis();

    if (now - startMillis > debounce_interval) {
        startMillis = now;
        isOpen = ! isOpen;
        stateChanged = true;
    }  
}

void httpget(String url) {
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
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println("closing connection");
}

void rtlog (boolean state) {
   String url = "/rt/home.xq?_action=store&_id=";
   url += streamId;
   url += "&_pk=";
   url += privateKey;
   url += "&state=";
   url += state;
   httpget(url);
}

void setup () {
 // put your setup code here, to run once:
  Serial.begin(9600);
  delay(10);

 // set pin to be innterrupt 
  pinMode(pin,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pin),switchChange,CHANGE);
  
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

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

void loop() {
    if (stateChanged) {
      Serial.println(isOpen);
      stateChanged=false;
      rtlog(isOpen);
    }
    delay(10);
}
