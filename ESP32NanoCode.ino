#include "arduino_secrets.h"
//required library for the functionality
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <HardwareSerial.h>

//define RX for reading data pins for UART
#define RXD2 2

//port 81 for WebSocket
WebSocketsServer webSocket = WebSocketsServer(81); 
//this is the network credentials
const char* ssid     = "Nano Aquaguard";
const char* password = "bmm22kfu";

//set web server port number to 80
WiFiServer server(80);

//buffer to store incoming UART data
String uartData = "";

//define strings to be display to the web server
String line = "";
String topIntake = "0";
String bottomIntake = "0";
String topOutlet = "0";
String bottomOutlet = "0";
String top_valve = "";
String bottom_valve = "";

//web server html stored as a variable to be displayed later
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body { font-family: sans-serif; padding: 20px; }
    .card { border: 1px solid #ccc; padding: 15px; margin: 10px; border-radius: 10px; }
  </style>
  <script>
    var socket = new WebSocket('ws://' + location.hostname + ':81/');
    socket.onmessage = function(event) {
      var data = JSON.parse(event.data);
      document.getElementById('topOutlet').innerText = data.topOutlet + ' L/hour';
      document.getElementById('bottomOutlet').innerText = data.bottomOutlet + ' L/hour';
      document.getElementById('topIntake').innerText = data.topIntake + ' L/hour';
      document.getElementById('bottomIntake').innerText = data.bottomIntake + ' L/hour';
      document.getElementById('topValve').innerText = data.topValve;
      document.getElementById('bottomValve').innerText = data.bottomValve;
    };
  </script>
</head>
<body>
  <h2>Live Sensor Readings</h2>
  <div class='card'><strong>Top Outlet:</strong> <span id='topOutlet'>--</span></div>
  <div class='card'><strong>Bottom Outlet:</strong> <span id='bottomOutlet'>--</span></div>
  <div class='card'><strong>Top Intake:</strong> <span id='topIntake'>--</span></div>
  <div class='card'><strong>Bottom Intake:</strong> <span id='bottomIntake'>--</span></div>
  <div class='card'><strong>Top Valve:</strong> <span id='topValve'>--</span></div>
  <div class='card'><strong>Bottom Valve:</strong> <span id='bottomValve'>--</span></div>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2);  // UART1
  
  //start access point with an address 192.168.4.1
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.begin();
  
  //then start the websocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  server.begin();
}

//debug console log for different events
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  //switch case for each state of clients connected the ESP32 Nano
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("Client %u Connected\n", num);
      break;
    case WStype_DISCONNECTED:
      Serial.printf("Client %u Disconnected\n", num);
      break;
    case WStype_TEXT:
      Serial.printf("Received from %u: %s\n", num, payload);
      break;
    default:
      break;
  }
}

void loop() {
  webSocket.loop();

  //read UART lines and extract values
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      line.trim();
      //if statement to scan for each relevant prefixes and prase those variables appropriately
      if (line.startsWith("Top Intake:")) {
        topIntake = line.substring(12, line.indexOf("L/hour"));
        topIntake.trim();
      } else if (line.startsWith("Bottom Intake:")) {
        bottomIntake = line.substring(15, line.indexOf("L/hour"));
        bottomIntake.trim();
      } else if (line.startsWith("Top Outlet:")) {
        topOutlet = line.substring(12, line.indexOf("L/hour"));
        topOutlet.trim();
      } else if (line.startsWith("Bottom Outlet:")) {
        bottomOutlet = line.substring(15, line.indexOf("L/hour"));
        bottomOutlet.trim();
      } else if (line.startsWith("Top valve:")) {
        top_valve = line.substring(10);
        top_valve.trim();
        } else if (line.startsWith("Bottom valve:")) {
        bottom_valve = line.substring(13);
        bottom_valve.trim();
} 

      //send combined JSON string to webSocket clients so that the client can display the data
      String json = "{\"topIntake\":\"" + topIntake + 
              "\",\"bottomIntake\":\"" + bottomIntake + 
              "\",\"topOutlet\":\"" + topOutlet + 
              "\",\"bottomOutlet\":\"" + bottomOutlet + 
              "\",\"topValve\":\"" + top_valve + 
              "\",\"bottomValve\":\"" + bottom_valve + "\"}";
      webSocket.broadcastTXT(json);
      line = "";
    } else {
      line += c;
    }
  }

  //serve HTML page
  WiFiClient client = server.available();
  if (client) {
    String request = "";
    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 2000) {
      if (client.available()) {
        char c = client.read();
        request += c;
        timeout = millis();

        if (request.endsWith("\r\n\r\n")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          client.println(webpage);
          break;
        }
      }
    }
    client.stop();
  }
}