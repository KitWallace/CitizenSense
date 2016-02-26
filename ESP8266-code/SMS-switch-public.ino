#include <ESP8266WiFi.h>
const char* ssid = "ssid";
const char* password = "wifipw";

const char* smslogin = "greentextlogin"; 
const char* smspassword= "greentextpw";
const char* smsref = "espreed";

const char* host = "www.textapp.net";
const char* replyurl = "http://kitwallace.co.uk/sms/xquery/service.xq";
const char* myPhone = "smsdestination";

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
  
  Serial.println();
  Serial.println("closing connection");
}

void sendsms(String text, const char* destination) {
// send via Greentext service
    String url = String("/webservice/httpservice.aspx") + "?method=sendsms&csvstring=false" + 
                 "&externallogin="+ smslogin +"&password=" + smspassword + 
                 "&clientbillingreference="+smsref+"&clientmessagereference=" + smsref + 
                 "&originator=&destinations=" + destination + 
                 "&charactersetid=2&validity=1&replymethodid=5&replydata=" + replyurl+"&statusnotificationurl=" + 
                 "&body=" + text;
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
      if (isOpen) {
         sendsms("door+opened",myPhone);
         Serial.println("text sent");
      }
    }
    delay(10);
}
