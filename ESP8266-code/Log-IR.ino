#define ADC_REF 3.3  // Reference voltage
#define ADC A0   // Analog input
#define LOGON 5  // take this pin high to turn on logging to the data stream
#define STATSON 4 // take this pin high to turn on stats 

// WiFi connection
#include <ESP8266WiFi.h>
const char* ssid = "ssid";
const char* password = "pw";

// Data stream
const char* host = "kitwallace.co.uk";
const char* streamId   = "streamid";
const char* privateKey = "pk";

// Sensor 
String sensorName = "IR distance sensor";

// sampling
int samples = 25;
int delayms = 1000;

void setup() 
{
  Serial.begin(9600); 

  connect_wifi();
  
  delay(100);  // sensor startup delay
  
  Serial.println(); 
  Serial.println(sensorName);
}


void loop() {
  if (digitalRead(STATSON))
       gatherStats();
  else getSample();
  delay(delayms);
}

void gatherStats() {
  float sum=0;
  float sum1=0;  
  float sum2=0;
  float value, k;
  k = asValue(getVoltage()); 
  Serial.println(String("k = ") + k);
  delay(delayms); 
  for(int i =0; i<samples; i++) {
      float voltage = getVoltage();
      value =asValue(voltage);
      Serial.println(String("value = ") + value);
      sum += value;
      sum1 += (value - k);
      sum2 += (value - k)* (value - k);
      delay(delayms); 
  }
  float mean = sum / samples ;
  float stddev = sqrt((sum2 - (sum1 * sum1) / samples) / (samples - 1));
  Serial.println(String("Statistics for ") + samples + " samples");
  Serial.println(String("mean =")+ mean + " StdDev= " + stddev + " 95% interval = " + (mean - 2 * stddev) + "," + (mean+ 2 * stddev));
  if (digitalRead(LOGON) ==1 )
      log_data(String("average=")+mean+"&stddev="+stddev);

}

void getSample() {
      float voltage = getVoltage();
      float value =asValue(voltage);
      Serial.println(String("value = ") + value);
      if (digitalRead(LOGON) ==1 )
          log_data(String("value=") + value);
}


float getVoltage()
{
  int sensor_value;
  sensor_value = analogRead(ADC);   
  float voltage = (float)sensor_value*ADC_REF/1024;
  return voltage;
}

/*
float asValue(float voltage) {
    return voltage;  // initially no conversion
}
*/
float asValue(float voltage) {
// very approximate
   return 27.0 / voltage;
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
