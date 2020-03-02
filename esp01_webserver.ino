/*********
  Adail Horst
  Inspiration Original code from http://randomnerdtutorials.com 
*********/

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager

#define DEBUG

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String newStatus = "on";

int switchStatus[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int switchId[] = {16, 5, 4, 0, 0, 0, 0, 0, 0};

String statusLed = "on";
String statusBlink = "on";

const int led = 2;

void setup() {
  Serial.begin(115200);

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  int c;
  for( c = 0; c < 8; c = c + 1 ){
    pinMode(switchId[c], OUTPUT);
    digitalWrite(switchId[c], HIGH);
  }
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);

  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  //wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  
  server.begin();
}

void loop(){
  if (statusBlink == "on") {
    blinkLed ();
  }
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            int b;
            char urlOn[12];
            char urlOff[12];
            for( b = 0; b < 8; b = b + 1 ){
              snprintf(urlOn, 12, "GET /%d/on", b);
              snprintf(urlOff, 12, "GET /%d/off", b);
              if (header.indexOf(urlOn) >= 0) {
                Serial.println("GPIO on");
                switchStatus[b] = 1;
                digitalWrite(switchId[b], HIGH);
              }
              if (header.indexOf(urlOff) >= 0) {
                Serial.println("GPIO off");
                switchStatus[b] = 0;
                digitalWrite(switchId[b], LOW);
              }
              Serial.println(b);
            }

            if (header.indexOf("GET /led/on") >= 0) {
              Serial.println("Board LED on");
              statusLed = "on";
              digitalWrite(led, HIGH);
            } else if (header.indexOf("GET /led/off") >= 0) {
              Serial.println("Board LED off");
              statusLed = "off";
              digitalWrite(led, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            // Enable BootSrap
            client.println("<link href=\"//netdna.bootstrapcdn.com/bootstrap/3.0.0/css/bootstrap.min.css\" rel=\"stylesheet\" id=\"bootstrap-css\">");
            client.println("<script src=\"//netdna.bootstrapcdn.com/bootstrap/3.0.0/js/bootstrap.min.js\"></script>");
            client.println("<script src=\"//code.jquery.com/jquery-1.11.1.min.js\"></script>");
            
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            //client.println(".button { background-color: #31d100; border: none; color: white; padding: 16px 40px;");
            //#195B6A / 
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            //client.println(".buttonOff {background-color: #77878A;}");
            client.println("</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>"+WiFi.hostname()+" Web Server</h1>");
            // Buttons
            int a;
            for( a = 0; a < 8; a = a + 1 ){
              char title[30];
              char id[2];
              snprintf(title, 30, "Switch %d", a);
              snprintf(id, 2, "%d", a);
              if (switchStatus[a] == 1) {
                newStatus = "on";
              } else {
                newStatus = "off";
              }
              client.println(gpioButton (title, id, newStatus));
            }
            //client.println(gpioButton ("Switch 1", "1", statusG1));
            //client.println(gpioButton ("Switch 2", "2", statusG2));
            //client.println(gpioButton ("Switch 3", "3", statusG3));
            client.println(gpioButton ("Board Led", "led", statusLed));
            client.println(gpioButton ("Blink Board Led", "blink", statusBlink));
            client.println(gpioButton ("Reset WiFi", "reset", ""));

            if (header.indexOf("GET /blink/on") >= 0) {
              Serial.println("Enable Board LED Blink");
              statusLed = "on";
              digitalWrite(led, HIGH);
            }            

            if (header.indexOf("GET /blink/on") >= 0) {
              statusBlink = "on";
              blinkLed ();
            } else if (header.indexOf("GET /blink/off") >= 0) {
              statusBlink = "off";
            }
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
void blinkLed () {
  digitalWrite(led, LOW);   //Liga LED
  delay(1000);              // aguarda 1 segundo
  digitalWrite(led, HIGH);  // apaga LED
  delay(1000);              // Aguarda 1 segundo
}

String button (String title, String URL, String State) {
  return "<p><a href=\""+URL+"\"><button class=\"button btn "+(State == "on" ? "btn-danger" : "btn-success")+"\">"+title+"</button></a></p>";
}

String gpioButton (String title, String GPIO, String State) {
  String result = "";
  result = "<p>"+title+" - State " + (State == "on" ? "OFF" : "ON") + "</p>";
  String URL = "/"+GPIO+"/"+(State == "on" ? "off" : "on");
  result = result + button (title, URL,State);
  return result;
}
