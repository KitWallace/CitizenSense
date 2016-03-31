/*
 * When config button is pressed start accesspoint and interact with user to provide the 
 * required access point  credentials and other parameters
 * Connect to the required access point and stop the server
 * 
 * In normal mode, collect data and send to the data store
 * 
 * Todo
 *   configuration needs to set the configuration in non-volatile store
 *   wifi connection failure not quite handled correctly yet
 *   need to flash the led when connected and data flowing
 * 
 */
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 

//temporary access point credentials

const char * ap_ssid = "tempap";
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

WiFiServer server(80);
WiFiClient client;

void setup() {
	delay(1000);
	Serial.begin(9600);
  Serial.println(String("ap started: ") + ap_started + " WiFi connected: " + wifi_connected); 
}

void loop() {
  if (! ap_started && digitalRead(D1)) {
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
    String params = String("digital=")+analogRead(A0);
    logData(params);
    delay(5000);
  }
}

void handleRequest() {
  Serial.println(String("request received ")+millis());
 
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  String response;
  String message = "";
  ssid = getParam(req,"ssid");
  if (ssid != "") {
      password = getParam(req,"password");
      stream_id = getParam(req,"stream-id");
      stream_pk = getParam(req,"stream-pk");
      String doConnect = getParam(req,"connect");
      
      if (doConnect =="Yes" && ssid != "")  {
      wifi_connected = connectWifi();
      if (wifi_connected) {
          response = String("<h1>Connecting to access point Bye Bye</h1>");
  
          httpReturn(response);
          // stop the ap - how ?
          ap_started = false;
          // stop the server - how ?
          return;
      }
      message = "Wifi not connected";
     }
  }
// return form 
   response = "<html><head>";
   response += "<meta name='viewport' content='width=device-width, initial-scale=1'/>";
   response += "</head>";
   response += "<h1>Configuration Form</h1>";
   if (message != "") 
          response += "<h2>" + message + "</h2>";
   response += "<form action='?'><table>";
   response += "<tr><td>SSID</td><td><input type='text' name='ssid' value='" + ssid +"'/></td></tr>";
   response += "<tr><td>PW</td><td><input type='text' name='password' value='" + password + "'/></td></tr>";
   response += "<tr><td>stream id</td><td><input type='text' name='stream-id' value='" + stream_id + "'/></td></tr>";
   response += "<tr><td>stream pk</td><td><input type='text' name='stream-pk' value='" + stream_pk + "'/></td></tr>";
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
  boolean result = WiFi.softAP(ap_ssid, ap_password);
  Serial.println(String("SoftAP return ") + result);
  
  IPAddress ap_IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ap_IP);
}

void startServer() {
  server.begin();
  Serial.println("HTTP server started");
  
}
boolean connectWifi() {
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
    if (tries > 10) return false;
    tries ++;
  }
  Serial.println("");
  Serial.println("WiFi connected");
  return true;
}

void httpGet(char * host,String url) {
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

void logData(String params) {
   String url = "/rt/home.xq?_action=store&_id=";
   url += stream_id;
   url += "&_pk=";
   url += stream_pk;
   url += "&";
   url += params;
   httpGet(host,url); 
}
