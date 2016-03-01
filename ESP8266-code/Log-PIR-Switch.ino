#include <ESP8266WiFi.h>
const char* ssid = "ssid";
const char* password = "wifipw";

const char* host = "kitwallace.co.uk";
const char* streamId   = "stream";
const char* privateKey = "pk";

//switch is debounced 
const int switch_pin = 4;
volatile boolean switch_isClosed = true;  
volatile boolean switch_changed = false;
volatile unsigned long startMillis = 0;
const long debounce_interval = 3 * 1000;  //3 seconds is OK for a door opening

//pir  transition to no movement only after a period of inactivity
const int pir_pin = 5;
volatile boolean pir_on = false;
volatile boolean pir_changed = false;
unsigned long pir_millis = 0;
boolean pir_movement = false;

const long inactive_interval = 60 * 1000;  // wait 60 seconds after pir false before signaling no activity

void switch_change() {
// ignore changes within debounce_interval
    unsigned long now = millis();
    if (now - startMillis > debounce_interval) {
        startMillis = now;
        switch_isClosed = digitalRead(switch_pin);
        switch_changed = true;
    }  
}

void pir_change() {
   pir_on= digitalRead(pir_pin);
   pir_changed= true;
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
  
  Serial.println();
  Serial.println("closing connection");
}

void rtlog (char* field, boolean state) {
   String url = "/rt/home.xq?_action=store&_id=";
   url += streamId;
   url += "&_pk=";
   url += privateKey;
   url += String("&")+field+"=";
   url += state;
   httpget(url);
}

void setup () {
  Serial.begin(9600);
  delay(10);

 // set switch pin to be interrupt 
  pinMode(switch_pin,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(switch_pin),switch_change,CHANGE);
  Serial.println("using Reed switch in pin 4 (d1)");
  
 // set PIR pin to be interrupt 
  pinMode(pir_pin,INPUT);
  attachInterrupt(digitalPinToInterrupt(pir_pin),pir_change,CHANGE);
  Serial.println("using PIR in pin 5 (d2)");
  
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
    unsigned long now = millis();
    if (switch_changed) {
      Serial.print(now/1000);
      Serial.print(" Switch ");
      Serial.print(switch_isClosed);
      if (switch_isClosed) 
            Serial.println(" door closed");
      else 
            Serial.println(" door open");
            rtlog("door",switch_isClosed);
      switch_changed=false;
    }
    if (pir_changed){     
      Serial.print(now/1000);
      Serial.print(" PIR ");
      Serial.println(pir_on);
      pir_millis = now;
      if (pir_on && ! pir_movement) {
              pir_movement = true;
              Serial.println(" movement");
              rtlog("movement",pir_movement);
          }
      pir_changed = false;
    }
    if ( ! pir_on && pir_movement && now - pir_millis > inactive_interval) {
          pir_movement=false;
          Serial.println(" no movement");
          rtlog("movement",pir_movement);
    }
          
//
    delay(10);
}
