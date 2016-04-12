/*
 * on bootup attempt to read configuration from SPIFFS and connect
 * 
 * When config button is pressed start access point (4 flashes) and interact with user to provide the 
 * required access point  credentials and other parameters
 * save configuration to SPIFFS
 * Connect to the required access point (2 flashes)
 * 
 * In normal mode, collect data and send to the data store (1 flash)
 * 
 */
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <FS.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

WiFiServer server(80);
WiFiClient client;

//temporary access point credentials

const char * ap_ssid = "espap";
const char * ap_password = "1212121212";  // must be 8 characters or more

// application status
boolean ap_started = false;
boolean wifi_connected = false;

// configuration parameters - need to be in non-volatile memory
String ssid;
String password;
String stream_id;
String stream_pk;
char* host ="kitwallace.co.uk";
char* path ="/rt/home.xq?";

// interaction
const int CONFIG = D7;
const int LED = D4;

// current sensor
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

// calibration
// simple calibration with a 60w lightbulb  
float multiplier = 0.0106;   
int n_samples = 1024;  // could go down to 64 with 4% precision

void setup() {

  Serial.begin(9600);
    delay(1000);
  Serial.println();
  Serial.println(String("ap started: ") + ap_started + " WiFi connected: " + wifi_connected); 

  pinMode(CONFIG,INPUT);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);  // turn it off
  bool result = SPIFFS.begin();
  boolean loadOK;
  loadOK = getConfiguration();
  if (loadOK) {
     Serial.println("Configuration loaded");
     wifi_connected = connectWiFi();
  }
  else {
     Serial.println("No configuration found");
     wifi_connected = false;
  }

   ads.setGain(GAIN_ONE);       
   ads.begin();
}

void loop() {
  if (! ap_started && (digitalRead(CONFIG) || ! wifi_connected)) {
    startAP();
    startServer();
    ap_started = true;
    wifi_connected = false;
  }
  if (ap_started) {
    client = server.available(); 
    if (!client) 
      return;
    else {
        Serial.println(String("ap started: ") + ap_started + " WiFi connected: " + wifi_connected); 
        handleRequest();
        }
    }

  if (wifi_connected) {
    Serial.println(String("ap started: ") + ap_started + " WiFi connected: " + wifi_connected); 
    float current;
    int ms ;
    getIrms(n_samples, current, ms);
    String params = String("current=")+current+"&ms="+ms;
    logData(params);
    blink(1,100,500);
  }
}

void handleRequest() {
  Serial.println(String("request received ")+millis());
  String req = client.readStringUntil('\r');
  while(client.available()){
    req = client.readStringUntil('\r');
    Serial.print(req);
  }
// form data is the last line 
  client.flush();
  String message;
  String response;
  String reply = getParam(req,"ssid");
  if (reply != "") {
      ssid =getParam(req,"ssid");
      password = getParam(req,"password");
      stream_id = getParam(req,"stream_id");
      stream_pk = getParam(req,"stream_pk");
      String doConnect = getParam(req,"connect");
      
      if (doConnect =="Yes" && ssid != "")  {
         response= String("<div><h1>trying to connect</h1> If it fails, you will have to close the connection and reconnect</div>");
         httpReturn(response);
         delay(1000);
         wifi_connected = connectWiFi();
         ap_started= false;
         if (wifi_connected){
           putConfiguration();
         }
         return;
      }
     }
  }
// return form 
   response = "<html><head>";
   response += "<meta name='viewport' content='width=device-width, initial-scale=1'/>";
   response += "</head>";
   response += "<h1>Configuration Form</h1>";
   if (message != "") 
          response += "<h2>" + message + "</h2>";
   response += "<form action='?' method='post'><table>";
   response += "<tr><td>SSID</td><td><input type='text' name='ssid' value='" + ssid +"'/></td></tr>";
   response += "<tr><td>PW</td><td><input type='text' name='password' value='" + password + "'/></td></tr>";
   response += "<tr><td>stream id</td><td><input type='text' name='stream_id' value='" + stream_id + "'/></td></tr>";
   response += "<tr><td>stream pk</td><td><input type='text' name='stream_pk' value='" + stream_pk + "'/></td></tr>";
   response += "<tr><td>Connect</td><td><select name='connect'><option>No</option><option>Yes</option></select></td></tr>";
   response += "</table>";
   response += "<input type='submit' name='submit' />";
   response += "</form>";
   httpReturn(response);
}

String getParam(String request, String name) {
  int pstart = request.indexOf(name);
  if (pstart == -1) return "";
  pstart+= name.length() +1;
  int pend = request.indexOf("&",pstart);
  return request.substring(pstart,pend);
}

void httpReturn(String response) {
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += response;
  client.print(s);
}
void startAP() {
  Serial.println("Configuring access point...");
  Serial.println(String("ssid ")+ap_ssid+" password "+ ap_password);
  WiFi.disconnect();
  boolean result = WiFi.softAP(ap_ssid, ap_password);
  Serial.println(String("SoftAP return ") + result);
  delay(100);
  IPAddress ap_IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ap_IP);
  blink(4,100,500);
}

void startServer() {
  server.begin();
  Serial.println("HTTP server started");
  
}
boolean connectWiFi() {
// Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  char cssid[20];
  char cpw[30];
  ssid.toCharArray(cssid,ssid.length()+1);
  password.toCharArray(cpw,password.length()+1);
  WiFi.begin(cssid, cpw);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    blink(4,5,10);
    tries++;
    if (tries > 20 ) 
      return false;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  blink(2,100,500);
  return true;
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
   String p = "_action=store&_id=";
   p += stream_id;
   p += "&_pk=";
   p += stream_pk;
   p += "&";
   p += params;
   httpPost(host,path,p); 
}

void blink(int n, int onms,int offms) {
    for (int i =0;i <n; i++) {
      digitalWrite(LED,LOW);
      delay(onms);
      digitalWrite(LED,HIGH);
      delay(offms);
    }
}

boolean getConfiguration() {
     // open the file in write mode
    File f = SPIFFS.open("/f.txt", "r");
    if (!f) {
      Serial.println("file open failed");
      return false;
    }
    String line = f.readStringUntil('\n');
    Serial.println(line);
    ssid = getParam(line,"ssid");
    password = getParam(line,"password");
    stream_id = getParam(line,"stream_id");
    stream_pk = getParam(line,"stream_pk");
    return true;
};

void putConfiguration() {
    // open the file in write mode
    File f = SPIFFS.open("/f.txt", "w");
    if (!f) {
      Serial.println("file creation failed");
    }
    // write one URLstyle line
    f.println(String("ssid=")+ssid +"&password="+password+"&stream_id="+stream_id+"&stream_pk="+stream_pk+"&");
    f.close();
};

void getIrms(unsigned int Number_of_Samples,float &Irms,int &Ims)
{
  long startms = millis();
  long sum = 0;
  long sum2 = 0;
  for (int n = 0; n < Number_of_Samples; n++)
  {
    int sampleI = ads.readADC_Differential_0_1();
    sum += sampleI;
    sum2+= sampleI * sampleI;
  }  
  Ims = millis() - startms;
  Irms = sqrt( (sum2 - (sum * sum)/Number_of_Samples) / Number_of_Samples)* multiplier; 
  Serial.println(String("elapsed time ") + Ims +" sum " + sum + " sum2 " + sum2 + " current=" + Irms);
}
