#include <ESP8266WiFi.h>
#include <inttypes.h>
#include <Password.h>

#define READINGS       250
#define MS_PER_HOUR    3.6e6
#define WIFICONF_INSKETCH

int ledPin = 2; // GPIO2 of ESP8266

// Power measurement settings
unsigned short cycles_per_kwh = 400;
unsigned char  loThresholdP = 101;
unsigned char  hiThresholdP = 110;
unsigned long debounceTimeP = 600;
boolean markerState = LOW;
unsigned long cycles = 0;
unsigned long previous = 0; // timestamp
unsigned short readings[READINGS];
unsigned short cursor = 0;
boolean gotenough = false;
unsigned short hits = 0;
unsigned long prevMillisP = millis();    // Remember the start time of the Power logging cycle
boolean powerJustSwitchedOn = true;
double power = 0;   // Initial power is zero

// NETWORK: Server details...
IPAddress server(192, 168, 1, 2);

WiFiClient client;

void setup () {
  Serial.begin(115200);               // Start the serial connection for debugging

  // Switch off the LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  // Connect to wifi with DHCP
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
  
void loop () {
delay (20); // If the Arduino is too busy the Wifi will not function
  // Log the power values via wifi every minute
  if ((millis() - prevMillisP) >= 60000) {
    prevMillisP = millis();
      // if you get a connection, report back via serial:
      if (client.connect(server, 80)) {
        Serial.println("connected");
        // Make a HTTP request:
        client.print("GET /cgi-bin/meter?watt=");
        client.print(power);   
        client.println(" HTTP/1.1");
        client.println("Host: 192.168.1.2");
        client.println("Connection: close");
        client.println();
      } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    } 
  }
  //  Calulate the sum of 40 samples
  unsigned short sum = 0;
  for (byte i = 0; i < 40; i++) {
    sum += analogRead(A0);
  }

  // Calculate average over 250 sum samples 
  unsigned long bigsum = 0;
  for (unsigned short i = 0; i < READINGS; i++){
    bigsum += readings[i];
  }
  unsigned short average = bigsum / READINGS;

//  Calculate the ratio of the sum samples and the 250 sum samples multipied by 100
  unsigned short ratio = (double) sum / (average+1) * 100;
  
//   Keep 250 sum samples in the reading array

   readings[cursor++] = sum;
   if (cursor >= READINGS) {
    cursor = 0;
   }

// If markerState has not changed, make newmarkerState high if ration > lo. If the markerState HAS changed make newledstae low if ratio >= hi
  boolean newmarkerState = markerState 
    ? (ratio >  loThresholdP)
    : (ratio >= hiThresholdP);

  if (newmarkerState) hits++;

// Return if the markerstate has not changed
  if (newmarkerState == markerState) return;

  markerState = newmarkerState;

//  LED is ON (low) if the marker is detected
  digitalWrite(ledPin, !markerState);

  if (!markerState) {
    Serial.print("Marker: ");
    Serial.print(millis() - previous);
    Serial.print(" ms (");
    Serial.print(hits, DEC);
    Serial.println(" readings)");
    hits = 0;
    return;
  }
  
  unsigned long now = millis();
  unsigned long time = now - previous;

  Serial.println(time);

  if (time < debounceTimeP) return;

  previous = now;  

// If power has just been switched on, discard the cycle.
  if (powerJustSwitchedOn) {
    Serial.println("Discarding incomplete cycle.");
    powerJustSwitchedOn = false;
    return;
  }
  cycles++;
  power = 1000 * ((double) MS_PER_HOUR / time) / cycles_per_kwh;
  Serial.print("Cycle ");
  Serial.print(cycles, DEC);
  Serial.print(": ");
  Serial.print(time, DEC);
  Serial.print(" ms, ");
  Serial.print(power, 2);
  Serial.println(" W");
 
}

