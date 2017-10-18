#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <stdint.h>
#include <stdlib.h>
#include <ArduinoJson.h>

#define MIN_OFF_TIME_S 300 // AC must be off 300 seconds before turning back on
#define PERIOD_S 10 // How frequently the state is retrieved

#define MIN_OFF_TIME (MIN_OFF_TIME_S / PERIOD_S)
#define PERIOD_MS (PERIOD_S * 1000)
#define HEAT_PIN D6
#define FAN_PIN D7
#define AC_PIN D8

struct HVACState {
  bool heat;
  bool ac;
  bool fan;
  uint32_t off_time;
};

const char* ssid = "INSERT SSID";
const char* password = "INSERT PASSWORD";
const char* host = "INSERT URL";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";
bool offline = false;
HVACState state = {false, false, false, 0};
HVACState nextState = {false, false, false, 0};

void setup() {
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(AC_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println();
  connectToWifiAP();
}

void loop() {
  if(offline) {
    //runOfflineMode();
  }
  else {
    getState();
    if(verifyState()) {
      runState();  
    }
    //else offline = true;
  }
  delay(PERIOD_MS);
}

bool verifyState() {
  // 4 Valid States
  // HEATING (heat and fan on)
  if(nextState.heat && !nextState.ac && nextState.fan) {
    return true;
  }
  // COOLING (ac and fan on)
  else if(!nextState.heat && nextState.ac && nextState.fan) {
    if(!state.ac && nextState.off_time >= MIN_OFF_TIME) {
      return true;
    }
    else return false;
  }
  // VENTILATING (just fan on)
  else if(!nextState.heat && !nextState.ac && nextState.fan) {
    return true;
  }
  // IDLE
  else if(!nextState.heat && !nextState.ac && !nextState.fan) {
    return true;
  }
  // ERROR (invalid state)
  else return false;
}

void runState() {
  state = nextState;
  digitalWrite(HEAT_PIN, state.heat);
  digitalWrite(AC_PIN, state.ac);
  digitalWrite(FAN_PIN, state.fan);
}

void getState() {
  static WiFiClientSecure client; //make static?
  connectToAPI(&client);
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
  //Add better checks here
  nextState.heat = root["heat"];
  nextState.ac = root["ac"];
  nextState.fan = root["fan"];
  nextState.off_time = root["off_time"];
}

