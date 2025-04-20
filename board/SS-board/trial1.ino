#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <math.h>

const char* ssid = "ESP32_Sensor_AP";
const char* mdnsName = "esp32";
WebServer server(80);

const int mq2Pin = 34;
const int mq135Pin = 36;
const int mq136Pin = 33;
const int mq137Pin = 32;
//resistance b/w A0 and GND
const float RL_MQ2   = 4.3;
const float RL_MQ135 = 0.7;
const float RL_MQ136 = 0.8;
const float RL_MQ137 = 0.7;
//As per caliberation
const float Ro_MQ2   = 9.83;
const float Ro_MQ135 = 32.26;
const float Ro_MQ136 = 0.39;
const float Ro_MQ137 = 1.31;

const float VCC = 5.0;
const int numSamples = 10;

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
          document.getElementById('ammoniaValue').textContent = data.nh3;

          document.getElementById('methaneStatus').textContent = (data.methane > 1000) ? 'Danger' : 'Safe';
          document.getElementById('methaneStatus').className = (data.methane > 1000) ? 'red' : 'green';

          document.getElementById('co2Status').textContent = (data.co2 > 400) ? 'Danger' : 'Safe';
          document.getElementById('co2Status').className = (data.co2 > 400) ? 'red' : 'green';

          document.getElementById('h2sStatus').textContent = (data.h2s > 10) ? 'Danger' : 'Safe';
          document.getElementById('h2sStatus').className = (data.h2s > 10) ? 'red' : 'green';

          document.getElementById('ammoniaStatus').textContent = (data.nh3 > 50) ? 'Danger' : 'Safe';
          document.getElementById('ammoniaStatus').className = (data.nh3 > 50) ? 'red' : 'green';
        });
    }, 1000);
  </script>
</body>
</html>
)=====";

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid);
  
  if (MDNS.begin(mdnsName)) {
    Serial.print("mDNS hostname: ");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/data", []() {
    float ppmMQ2   = readMQ2();
    float ppmMQ135 = readMQ135();
    float ppmMQ136 = readMQ136();
    float ppmMQ137 = readMQ137();

    String json = "{";
    json += "\"methane\":" + String(ppmMQ2) + ",";
    json += "\"co2\":"     + String(ppmMQ135) + ",";
    json += "\"h2s\":"     + String(ppmMQ136) + ",";
    json += "\"nh3\":"     + String(ppmMQ137) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("ESP32 Server Started!");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}

float getAverageADC(int pin) {
  float adcSum = 0;
  for (int i = 0; i < numSamples; i++) {
    adcSum += analogRead(pin);
    delay(10);
  }
  return adcSum / numSamples;
}

float calculateRs(float adc, float RL) {
  float V_RL = adc * 3.3 / 4095.0;
  return ((VCC * RL) / V_RL) - RL;
}

float calculatePPM(float Rs, float Ro, float m, float b) {
  float ratio = Rs / Ro;
  return pow(10, ((log10(ratio) - b) / m));
}

float readMQ135() {
  float adc = getAverageADC(mq135Pin);
  float V_esp32 = adc * 3.3 / 4095.0;
  float V_RL_actual = V_esp32 * ( 260 / 150 ); //voltage divider
  float Rs = ((VCC * RL_MQ135) / V_RL_actual) - RL_MQ135;

  float ppm = calculatePPM(Rs, Ro_MQ135, -0.42, 1.92);
  return ppm;
}

float readMQ2() {
  float adc = getAverageADC(mq2Pin);
  float Rs = calculateRs(adc, RL_MQ2);
  return calculatePPM(Rs, Ro_MQ2, -0.38, 1.46);
}

float readMQ136() {
  float adc = getAverageADC(mq136Pin);
  float Rs = calculateRs(adc, RL_MQ136);
  return calculatePPM(Rs, Ro_MQ136, -1.53, 0.93);
}

float readMQ137() {
  float adc = getAverageADC(mq137Pin);
  float Rs = calculateRs(adc, RL_MQ137);
  return calculatePPM(Rs, Ro_MQ137, -1.41, 0.87);
}
