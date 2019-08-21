/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
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
String statusG0 = "off";
String statusLed = "off";

// Assign output variables to GPIO pins
const int g0 = 0;
const int led = 2;

void setup() {
  Serial.begin(115200);

  pinMode(g0, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(g0, LOW);
  digitalWrite(led, LOW);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  
  server.begin();
}

void loop(){
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
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /0/on") >= 0) {
              Serial.println("GPIO 0 on");
              statusG0 = "on";
              digitalWrite(g0, HIGH);
            } else if (header.indexOf("GET /0/off") >= 0) {
              Serial.println("GPIO 0 off");
              statusG0 = "off";
              digitalWrite(g0, LOW);
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
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266/ESP01 Web Server</h1>");
/*            
            client.println("<p>GPIO 0 - State " + statusG0 + "</p>");
            if (statusG0=="off") {
              client.println("<p><a href=\"/0/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/0/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("<p>Board LED - State " + statusLed + "</p>");
            if (statusLed=="off") {
              client.println("<p><a href=\"/led/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/led/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
/*
            client.println(gpioButton ("GPIO 0", 0, (output0State == "off" ? 0 : 1)));
            client.println(gpioButton ("GPIO 1", 1, (output1State == "off" ? 0 : 1)));
            client.println(gpioButton ("GPIO 2", 2, (output2State == "off" ? 0 : 1)));
            client.println(gpioButton ("GPIO 3", 3, (output3State == "off" ? 0 : 1)));
            client.println("</body></html>");
*/            
            client.println(gpioButton ("GPIO 0", "0", statusG0));
            client.println(gpioButton ("Board Led", "led", statusLed));
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

String button (String title, String URL, String State) {
  return "<p><a href=\""+URL+"\"><button class=\"button "+(State == "on" ? "button2" : "")+"\">"+title+"</button></a></p>";
}

String gpioButton (String title, String GPIO, String State) {
  String result = "";
  result = "<p>"+title+" - State " + (State == "on" ? "OFF" : "ON") + "</p>";
  String URL = "/"+GPIO+"/"+(State == "on" ? "off" : "on");
  result = result + button (title, URL,State);
/*  if (State == 0) {
    result = result + button (title, "/"+GPIO+"\/"+(State != 0 ? "off" : "on"),State);
    //"<p><a href=\"/"+GPIO+"\/on\"><button class=\"button\">ON</button></a></p>";
  } else {
    result = result + 
    //"<p><a href=\"/"+GPIO+"/off\"><button class=\"button button2\">OFF</button></a></p>";
  } */
  return result;
}
