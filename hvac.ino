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

void loop() {
    // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  /*if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }*/

  String url = "/testStage/hvac/testHvacID";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //String line = client.readString();//client.readStringUntil('\n');
  //Serial.println(line);
  delay(500);
  String json = "";
  String line = "";
  boolean httpBody = false;
  while (client.available()) {
    line = client.readStringUntil('\r');
    /*if (!httpBody && line.charAt(1) == '{') {
      httpBody = true;
    }
    if (httpBody) {
      json += line;
    }*/
  }
  StaticJsonBuffer<400> jsonBuffer;
  Serial.println("Got data:");
  Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  Serial.println("JSON PARSED");
  const char* heater = root["heater"];
  Serial.println(heater);
  
  if(!strcmp("on", heater)) {
    Serial.println("Heater is on");
    digitalWrite(D8, HIGH); // LED ON
  }
  else {
    Serial.println("Heater is off");
    digitalWrite(D8, LOW); // LED OFF
  }
}
