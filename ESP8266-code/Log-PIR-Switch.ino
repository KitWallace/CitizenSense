#include <ESP8266WiFi.h>
const char* ssid = "ssid";
const char* password = "wifipw";

const char* host = "kitwallace.co.uk";
const char* streamId   = "stream";
const char* privateKey = "pk";

// sms
char* myphone ="447421119309";
char* sms_pin = "1564";


// door switch is debounced 
const int door_pin = 14;
volatile boolean door_isClosed;  
volatile boolean door_changed = false;
volatile unsigned long start_ms;
unsigned int door_ms;
const long debounce_interval = 3 * 1000;  //3 seconds is OK for a door opening
boolean warning_sent = false;
const long door_wait = 30 * 1000;  // longer than 30 seconds in practice

//pir  transition to no movement only after a period of inactivity
const int pir_pin = 15;
volatile boolean pir_active ;
volatile boolean pir_changed = false;
unsigned long pir_ms;
boolean pir_movement = false;
unsigned long movement_ms;
const long inactive_interval = 60 * 1000;  // wait 60 seconds after pir false before signaling no activity

int seconds(long ms) {
   return round(ms/1000); 
}

void door_change() {
// ignore changes within debounce_interval
    unsigned long now = millis();
    if (now - start_ms > debounce_interval) {
        start_ms = now;
        door_isClosed = digitalRead(door_pin);
        door_changed = true;
    }  
}

void pir_change() {
   pir_active= digitalRead(pir_pin);
   pir_changed= true;
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

void send_sms(String text,char* destination) {
  String url = String("/sms/xquery/sendsms.xq?pin=")+sms_pin+"&text="+text+"&destination="+destination;
  httpget(url);
}
void setup () {
  Serial.begin(9600);
  delay(10);

 // set switch pin to be interrupt 
  pinMode(door_pin,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(door_pin),door_change,CHANGE);
  Serial.println(String("Door switch on pin ") + door_pin);
  
 // set PIR pin to be interrupt 
  pinMode(pir_pin,INPUT);
  attachInterrupt(digitalPinToInterrupt(pir_pin),pir_change,CHANGE);
  Serial.println(String("PIR on pin ") + pir_pin);
  
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
  
  // initialize states and time marks
  unsigned long now = millis();
  pir_ms=now;
  pir_active = digitalRead(pir_pin);
  door_ms=now;
  door_isClosed = digitalRead(door_pin);
  movement_ms=now;
  start_ms=now;
}

void loop() {
    long now = millis();
    
    if (door_changed) {
      Serial.print(now/1000);
      Serial.print(" Door ");
      Serial.print(door_isClosed);
      log_data(String("door=") + door_isClosed + "&interval=" + seconds(now - door_ms) );
      door_ms= now;
      door_changed=false;
      warning_sent=false;
    }
    
// if door has been left open for longer than door_wait then send SMS

    if ( ! door_isClosed  && now - door_ms > door_wait && ! warning_sent) {
          send_sms(String("door has been open for ")+seconds(now - door_ms)+ " seconds. Please shut it.",myphone);
          Serial.println(" SMS warning sent");
          warning_sent=true;
    }
     
    if (pir_changed) {     
      Serial.print(now/1000);
      Serial.print(" PIR ");
      Serial.println(pir_active);
      if (pir_active && ! pir_movement) {
              pir_movement = true;
              Serial.println(" movement");
              log_data(String("empty=") + seconds(now - movement_ms));
              movement_ms = now;
           }
      pir_changed = false;
      pir_ms = now;
    }
    
    if ( ! pir_active && pir_movement && now - pir_ms > inactive_interval) {
          pir_movement=false;
          Serial.println(" no movement");
          log_data(String("occupied=") + seconds(now - movement_ms));;
          movement_ms = now;
    }
          
//
    delay(10);
}
