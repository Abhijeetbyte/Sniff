#include <WiFi.h>
#include <ESPmDNS.h>

// Pin Definitions for Sensors
const int mq2Pin = 34;   // Methane (Natural Gas) connected to GPIO 34
const int mq135Pin = 35; // Air Quality (CO2) connected to GPIO 35
const int mq136Pin = 32; // H2S (Hydrogen Sulfide) connected to GPIO 32
const int mq137Pin = 33; // NH3 (Ammonia) connected to GPIO 33

// Threshold values in PPM (example limits, adjust according to your need)
const float methaneThreshold = 1000;   // Methane, Natural Gas PPM limit
const float co2Threshold = 400;        // Air Quality (CO2)
const float h2sThreshold = 10;         // H2S (Hydrogen Sulfide)
const float ammoniaThreshold = 50;     // Ammonia (NH₃)

// SSID for Access Point
const char* ssid = "ESP32_Sensor_AP";
const char* mdnsName = "esp32";  // mDNS hostname

// HTML content for the webpage
String htmlPage = R"=====(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>Public Washroom Gas Monitor</title>
    <style>
      body { font-family: Arial, sans-serif; text-align: center; }
      h1 { color: #444; }
      table { width: 80%; margin: auto; border-collapse: collapse; }
      th, td { padding: 15px; text-align: left; }
      th { background-color: #f2f2f2; }
      .green { color: green; font-weight: bold; }
      .red { color: red; font-weight: bold; }
    </style>
  </head>
  <body>
    <h1>Public Washroom Gas Monitoring</h1>
    <table border="1">
      <tr>
        <th>Parameter</th>
        <th>PPM Value</th>
        <th>Status</th>
      </tr>
      <tr>
        <td>Methane (Natural Gas)</td>
        <td><span id="methaneValue">--</span> PPM</td>
        <td><span id="methaneStatus">--</span></td>
      </tr>
      <tr>
        <td>Air Quality (CO2)</td>
        <td><span id="co2Value">--</span> PPM</td>
        <td><span id="co2Status">--</span></td>
      </tr>
      <tr>
        <td>Hydrogen Sulfide (H2S)</td>
        <td><span id="h2sValue">--</span> PPM</td>
        <td><span id="h2sStatus">--</span></td>
      </tr>
      <tr>
        <td>Ammonia (NH3)</td>
        <td><span id="ammoniaValue">--</span> PPM</td>
        <td><span id="ammoniaStatus">--</span></td>
      </tr>
    </table>
    <script>
      setInterval(function() {
        fetch('/data')
          .then(response => response.json())
          .then(data => {
            document.getElementById('methaneValue').textContent = data.methane;
            document.getElementById('co2Value').textContent = data.co2;
            document.getElementById('h2sValue').textContent = data.h2s;
            document.getElementById('ammoniaValue').textContent = data.ammonia;

            document.getElementById('methaneStatus').textContent = (data.methane > 1000) ? 'Danger' : 'Safe';
            document.getElementById('methaneStatus').className = (data.methane > 1000) ? 'red' : 'green';

            document.getElementById('co2Status').textContent = (data.co2 > 400) ? 'Danger' : 'Safe';
            document.getElementById('co2Status').className = (data.co2 > 400) ? 'red' : 'green';

            document.getElementById('h2sStatus').textContent = (data.h2s > 10) ? 'Danger' : 'Safe';
            document.getElementById('h2sStatus').className = (data.h2s > 10) ? 'red' : 'green';

            document.getElementById('ammoniaStatus').textContent = (data.ammonia > 50) ? 'Danger' : 'Safe';
            document.getElementById('ammoniaStatus').className = (data.ammonia > 50) ? 'red' : 'green';
          });
      }, 1000);  // Refresh every second
    </script>
  </body>
  </html>
)=====";

WiFiServer server(80);

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Initialize sensor pins
  pinMode(mq2Pin, INPUT);
  pinMode(mq135Pin, INPUT);
  pinMode(mq136Pin, INPUT);
  pinMode(mq137Pin, INPUT);

  // Set up the ESP32 as an access point
  WiFi.softAP(ssid);
  
  // Set up mDNS
  if (!MDNS.begin(mdnsName)) {
    Serial.println("Error setting up MDNS responder!");
    return;
  }
  Serial.print("Access Point Started. IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("mDNS hostname: ");
  Serial.println(mdnsName);

  // Start the web server
  server.begin();
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  while (!client.available()) {
    delay(10);
  }

  // Read the client's request
  String request = client.readStringUntil('\r');
  Serial.println(request);  // Debugging: Print the HTTP request
  client.flush();

  // Read sensor values and convert to PPM
  float methanePPM = readMQ2Sensor();   // Methane PPM (MQ2)
  float co2PPM = readMQ135Sensor();     // CO2 PPM (MQ135)
  float h2sPPM = readMQ136Sensor();     // H2S PPM (MQ136)
  float ammoniaPPM = readMQ137Sensor(); // Ammonia PPM (MQ137)

  // Debugging: Print raw sensor values to the Serial Monitor
  Serial.print("Methane PPM: "); Serial.println(methanePPM);
  Serial.print("CO2 PPM: "); Serial.println(co2PPM);
  Serial.print("H2S PPM: "); Serial.println(h2sPPM);
  Serial.print("Ammonia PPM: "); Serial.println(ammoniaPPM);

  // Handle the request
  if (request.indexOf("/data") != -1) {
    // Send the sensor data in JSON format
    String jsonResponse = "{\"methane\":" + String(methanePPM) + ", \"co2\":" + String(co2PPM) + 
                          ", \"h2s\":" + String(h2sPPM) + ", \"ammonia\":" + String(ammoniaPPM) + "}";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(jsonResponse);
  } else {
    // Send the HTML page
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(htmlPage);
  }

  // Close the connection
  client.stop();
}

// Conversion functions for each sensor
float readMQ2Sensor() {
  int rawValue = analogRead(mq2Pin);
  // Logarithmic equation to convert MQ-2 raw value to PPM (Methane)
  float ppm = pow(10, ((log10(rawValue) - 0.3) / 0.4)); 
  return ppm;
}

float readMQ135Sensor() {
  int rawValue = analogRead(mq135Pin);
  // Logarithmic equation to convert MQ-135 raw value to PPM (CO2)
  float ppm = pow(10, ((log10(rawValue) - 0.3) / 0.35)); 
  return ppm;
}

float readMQ136Sensor() {
  int rawValue = analogRead(mq136Pin);
  // Logarithmic equation to convert MQ-136 raw value to PPM (H2S)
  float ppm = pow(10, ((log10(rawValue) - 0.25) / 0.35)); 
  return ppm;
}

float readMQ137Sensor() {
  int rawValue = analogRead(mq137Pin);
  // Logarithmic equation to convert MQ-137 raw value to PPM (NH3)
  float ppm = pow(10, ((log10(rawValue) - 0.2) / 0.45)); 
  return ppm;
}
