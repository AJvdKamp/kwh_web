#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Password.h>
#include <LM75.h>
#include <inttypes.h>

#define READINGS       250
#define MS_PER_HOUR    3.6e6

int ledPin = 2; // GPIO2 of ESP8266

WiFiServer server(80);//Service Port

// NETWORK: Static IP details...
IPAddress ip(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

struct SettingsStruct {
  unsigned short cycles_per_kwh = 400;
  unsigned char  lower_threshold = 101;
  unsigned char  upper_threshold = 110;
} settings;

boolean ledstate = LOW;
unsigned long debounce_time = 600;
double watt;

void setup () {
  
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(SSID, PASSWORD);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
  }
  Serial.println("");
  Serial.println("WiFi connected");
   
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  Wire.begin();  
}


unsigned long cycle = 0;
unsigned long previous = 0; // timestamp

unsigned short readings[READINGS];
unsigned short cursor = 0;
boolean gotenough = false;

unsigned short hits = 0;
  
void loop () {

  writeWebData();

  //  Calulate the sum of 40 samples
  unsigned short sum = 0;
  for (byte i = 0; i < 40; i++) {
//    sum += analogRead(A0);
  }
   sum += analogRead(A0);
   Serial.println(analogRead(A0));

//  // Calculate average over 250 sum samples 
//  unsigned long bigsum = 0;
//  for (unsigned short i = 0; i < READINGS; i++){
//    bigsum += readings[i];
//  }
//  unsigned short average = bigsum / READINGS;
//  
////  Calculate the ratio of the sum samples and the 250 sum samples multipied by 100
//  unsigned short ratio = (double) sum / (average+1) * 100;
//  
////   Read 250 sum samples in the reading array
//   readings[cursor++] = sum;
//   if (cursor >= READINGS) {
//    cursor = 0;
//   }
//
//
//  unsigned short lo = settings.lower_threshold;
//  unsigned short hi = settings.upper_threshold;
//
//// If ledstate has not changed, make newledstate high if ration > lo. If the ledstate HAS changed make newledstae low if ratio >= hi
//  boolean newledstate = ledstate 
//    ? (ratio >  lo)
//    : (ratio >= hi);
//
//  if (newledstate) hits++;
//
//    writeWebData();
// 
//  if (newledstate == ledstate) return;
//
//  ledstate = newledstate;
//
////  LED is ON (low) if the marker is detected
//  digitalWrite(ledPin, !ledstate);
//
//  if (!ledstate) {
//    Serial.print("Marker: ");
//    Serial.print(millis() - previous);
//    Serial.print(" ms (");
//    Serial.print(hits, DEC);
//    Serial.println(" readings)");
//    hits = 0;
//    return;
//  }
//  
//  unsigned long now = millis();
//  unsigned long time = now - previous;
//
//  Serial.println(time);
//
//  if (time < debounce_time) return;
//
//  previous = now;  
// 
//  if (!cycle++) {
//    Serial.println("Discarding incomplete cycle.");
//    return;
//  }
//  watt = 1000 * ((double) MS_PER_HOUR / time) / settings.cycles_per_kwh;
//  Serial.print("Cycle ");
//  Serial.print(cycle, DEC);
//  Serial.print(": ");
//  Serial.print(time, DEC);
//  Serial.print(" ms, ");
//  Serial.print(watt, 2);
//  Serial.println(" W");
}

void writeWebData(){

// Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
   
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
   
  // Match the request
 
  char* sensor = "";
  if (request.indexOf("/watt") != -1) {
    sensor = "watt";
  } 
  else if (request.indexOf("/light") != -1){
    sensor = "light";
  }
   
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
   
  if(sensor == "watt") {
    client.println(watt);  
  } 
  else if(sensor == "light") {
//    client.println(getLuminance());
  }
  else {
  client.println("<br>");
  client.println("Click <a href=\"/watt\">here</a> to read the power<br>");
  client.println("Click <a href=\"/light\">here</a> to read the light<br>");
  client.println("</html>");
  }
  
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
  Serial.println("Client disconnected");
  Serial.println("");
}

