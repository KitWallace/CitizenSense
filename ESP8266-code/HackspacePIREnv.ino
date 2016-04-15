#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_HTU21DF.h>

// Wifi
const char* ssid = "ssid";
const char* password = "pw";


// datastream
char* pir_host = "kitwallace.co.uk";
const char* pir_streamId   = "stream";
const char* pir_privateKey = "private stream pk";

// anyonein

char* anyonein_host = "bristol.hackspace.org.uk";
String anyonein_url =  "/anyonein/?sensor=movement"; 

// PIR 
const int pir_pin = D6;
boolean pir_active ;

// HTU21D
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

void setup () {
  Serial.begin(9600);
  delay(10);

// Temp - humidity 
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

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

// PIR
  
  pinMode(pir_pin,INPUT);
  Serial.println(String("PIR on pin ") + pir_pin);

// Start HTU
   htu.begin();

  delay(60*1000);  //PIR initialization
}

void loop() {

    long now = seconds(millis());

// read PIR
    pir_active= digitalRead(pir_pin);     
    Serial.println(String("sec=") + now + " PIR " + pir_active);


// RSSI for this AP  
    int rssi = getRSSI(ssid);

// temp/humidity
   float temp = htu.readTemperature();
   float humidity =htu.readHumidity();

// log the data
   String data =String("occupied=") + pir_active+"&rssi="+rssi+"&temperatureC"+temp +"&humidity" + humidity;  
   Serial.println(data);
   log_data(data);
   if (pir_active = 1)
       log_anyonein();
   
   delay(60*1000);
}    

int seconds(long ms) {
   return round(ms/1000); 
}

// Return RSSI or 0 if target SSID not found
int  getRSSI(const char* target_ssid) {
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++) {
    if (WiFi.SSID(network)== String(target_ssid)) {
      return WiFi.RSSI(network);
    }
  }
  return 0;
}

void httpget(char * host, String url) {
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

void log_anyonein() {
   httpget(anyonein_host,anyonein_url);  
}

void log_data(String params) {
   String url = "/rt/home.xq?_action=store&_id=";
   url += pir_streamId;
   url += String("&_pk=")+ pir_privateKey + "&";
   url += params;
   httpget(pir_host,url); 
}
