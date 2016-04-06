#define ADC_REF 3.3  // Reference voltage
#define ADC A0   // Analog input
#define LOGON D6  // take this pin high to turn on logging to the data stream
#define STATSON D7 // take this pin high to turn on stats 

// WiFi connection
#include <ESP8266WiFi.h>
const char* ssid = "ssid";
const char* password = "passwordf";

// Data stream
char* storeHost = "kitwallace.co.uk";
char* storePath = "/rt/home.xq";
char* streamId   = "id";
char* privateKey = "pk";

// Sensor 
String sensorName = "Angle";

// sampling
int samples = 25;
int delayms = 1000;

void setup() 
{
  Serial.begin(9600); 

  connectWifi();
  
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
      logData(String("average=")+mean+"&stddev="+stddev);

}

void getSample() {
      float voltage = getVoltage();
      float value =asValue(voltage);
      Serial.println(String("value = ") + value);
      if (digitalRead(LOGON) ==1 )
          logData(String("value=") + value);
}


float getVoltage()
{
  int sensor_value;
  sensor_value = analogRead(ADC);   
  float voltage = (float)sensor_value*ADC_REF/1024;
  return voltage;
}

float asValue(float voltage) {
    return voltage*270/ADC_REF;  // 270 degrees full turn
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

void httpPost(char* host, char* path, String data) {
   WiFiClient client;
   data.replace(" ", "+");
   int httpPort = 80;
   while (true) {
    if (client.connect(host, httpPort)) break;
    Serial.println("connection failed");
    delay(100);
  }
  
  Serial.println("connected ");
  Serial.println(String("Host=") + host + " path=" + path + " data=" + data);
// This will send the request to the server
  String httprequest = String("POST ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Content-Type: application/x-www-form-urlencoded\r\n"
               "Connection: close\r\n" +
               "Content-Length:"+ data.length() +"\r\n\r\n" +
               data;
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
   String data = "_action=store&_id=";
   data += streamId;
   data += String("&_pk=")+ privateKey + "&";
   data += params;
   httpPost(storeHost, storePath, data); 
}
