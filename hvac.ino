#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>


const char* ssid = "ADD SSID HERE";
const char* password = "ADD PSWD HERE";

const char* host = "ADD URL HERE";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";

void setup() {
  pinMode(D8, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  connectToWifiAP();
}

void loop() {
  WiFiClientSecure client;
  sendRequestToAPI(&client);
  getResponseFromAPI(&client);
}

void connectToWifiAP() {
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToAPI(WiFiClientSecure* client) {
  // Use WiFiClientSecure class to create TLS connection
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client->connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  /*if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }*/
  
}

void sendRequestToAPI(WiFiClientSecure* client) {
  String url = "/testStage/hvac/testHvacID";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client->print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  
  Serial.println("request sent");
  delay(500);
}

void getResponseFromAPI(WiFiClientSecure* client) {
  String json = "";
  // Read https response until json data
  while (client->available()) {
    json = client->readStringUntil('\r');
  }
  StaticJsonBuffer<400> jsonBuffer;
  Serial.println("Got data:");
  Serial.println(json);
  JsonObject& root = jsonBuffer.parseObject(json);
  if(!root.success()) {
    Serial.println("JSON PARSING FAILED");
    return;
  }
  Serial.println("JSON PARSED");
  const char* heater = root["heater"];
  
  if(!strcmp("on", heater)) {
    Serial.println("Heater is on");
    digitalWrite(D8, HIGH); // LED ON
  }
  else {
    Serial.println("Heater is off");
    digitalWrite(D8, LOW); // LED OFF
  }
}

