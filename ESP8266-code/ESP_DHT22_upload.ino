#include <ESP8266WiFi.h>
#include "DHT.h"

#define DHTPIN 5
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "my ssid";
const char* password = "my password";
float delayMinutes= 1;
float humidity;
float temperature;
const char* host = "kitwallace.co.uk";
const char* streamId   = "streamid";
const char* privateKey = "stream pk";

void setup() {
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

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  dht.begin();
}

void loop() {
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  while (true) {
    if (client.connect(host, httpPort)) break;
    Serial.println("connection failed");
    delay(100);
  }
  Serial.println("connected");

// Read the DHT22 sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  while(isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
  }
  
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

   // DISPLAY DATA
  Serial.print("DHT22, \t");
  Serial.print(h);
  Serial.print(",\t");
  Serial.println(t);

// Create URL to send the data to the datastore in a GET

    String url = "/rt/home.xq?_action=store&_id=";
    url += streamId;
    url += "&_pk=";
    url += privateKey;
    url += "&temp=";
    url += t;
    url += "&humidity=";
    url += h;
    url += "&heatindex=";
    url += hic;
  
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

  delay(delayMinutes*60*1000);
}
