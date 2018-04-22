#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define HEATER_PIN D6
#define FAN_PIN D7
#define AC_PIN D8
#define MAX_ERROR_COUNT 1

struct HVACState {
  bool heater;
  bool ac;
  bool fan;
  unsigned int off_time;
  unsigned int min_off_time;
  unsigned int update_period;
};

WiFiClientSecure client;
HVACState state = {false, false, false, 0, 0, 10};
HVACState next_state = {false, false, false, 0, 0, 10};
unsigned long error_count = 0;

/*************** MODIFY THESE LINES ***************/

const char* ssid = "BYU-WiFi";//"zbrogz";
const char* password = "";//"7149925462";
const char* host = "api.zachbrogan.com"; // ex) "api.api.com"
const char* path = "/hvac/2f3f04053b824a5491b4aa09975277f9"; // ex) "/dev/hvac/"

/*************** Setup & Loop ***************/

void setup() {
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(AC_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, false);
  digitalWrite(AC_PIN, false);
  digitalWrite(FAN_PIN, false);
  Serial.begin(9600);
  connectToWifi();
}

void loop() {
  if(error_count < MAX_ERROR_COUNT) {
    if(getState() && verifyState()) {
      runState();  
    }
    //else error_count++;
  }
  else {
    //runOffline();
    //error_count = 0;
  }
  delay_seconds(state.update_period);
}

/*************** Helper Functions ***************/
void delay_seconds(unsigned int seconds) {
  unsigned long ms = 1000L * seconds;
  delay(ms);
}

void connectToWifi() {
  Serial.print("Connecting to: ");
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

bool getState() {
  // Connect with TLS
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, 443)) {
    Serial.println("connection failed");
    return false;
  }

  // Send Request
  Serial.print("requesting URL: ");
  Serial.println(path);
  client.print(String("GET ") + String(path) + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("request sent");

  // Get Response
  // Skip Headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  // Read body
  String json = "";
  while (client.available()) {
    json += client.readStringUntil('\r');
  }
  Serial.println("Got data:");
  Serial.println(json);

  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  if(!root.success()) {
    Serial.println("JSON PARSING FAILED");
    return false;
  }
  Serial.println("JSON PARSED");

  // Verify Json
  JsonVariant heater = root["heater"];
  if(!heater.success() || !heater.is<bool>()) return false;
  next_state.heater = heater.as<bool>();
  
  JsonVariant ac = root["ac"];
  if(!ac.success() || !ac.is<bool>()) return false;
  next_state.ac = ac.as<bool>();
  
  JsonVariant fan = root["fan"];
  if(!fan.success() || !fan.is<bool>()) return false;
  next_state.fan = fan.as<bool>();
  
  JsonVariant off_time = root["off_time"];
  if(!off_time.success() || !off_time.is<unsigned int>()) return false;
  next_state.off_time = off_time.as<unsigned int>();
  
  JsonVariant min_off_time = root["min_off_time"];
  if(!min_off_time.success() || !min_off_time.is<unsigned int>()) return false;
  next_state.min_off_time = min_off_time.as<unsigned int>();

  JsonVariant update_period = root["update_period"];
  if(!update_period.success() || !update_period.is<unsigned int>()) return false;
  next_state.update_period = update_period.as<unsigned int>();

  return true;
}

bool verifyState() {
  // 4 Valid States
  // HEATING (heaterand fan on)
  if(next_state.heater && !next_state.ac && next_state.fan) {
    return true;
  }
  // COOLING (ac and fan on)
  else if(!next_state.heater && next_state.ac && next_state.fan) {
    if(!state.ac && (next_state.off_time >= next_state.min_off_time)) {
      return true;
    }
    else return false;
  }
  // VENTILATING (just fan on)
  else if(!next_state.heater && !next_state.ac && next_state.fan) {
    return true;
  }
  // IDLE
  else if(!next_state.heater && !next_state.ac && !next_state.fan) {
    return true;
  }
  // ERROR (invalid state)
  else return false;
}

void runState() {
  state = next_state;
  digitalWrite(HEATER_PIN, state.heater);
  digitalWrite(AC_PIN, state.ac);
  digitalWrite(FAN_PIN, state.fan);
}
