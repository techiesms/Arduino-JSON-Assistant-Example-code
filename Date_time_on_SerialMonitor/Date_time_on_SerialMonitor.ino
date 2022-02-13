#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid     = "SSID";
const char* password = "password";


const char * Hostname = "worldtimeapi.org";
String URI = "/api/timezone/asia/kolkata";
const uint16_t port = 80;


void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  while (!Serial) continue;
  wifi_connect();
  
  // Connect to HTTP server
  WiFiClient client;
  client.setTimeout(10000);
  if (!client.connect(Hostname, port)) {
    Serial.println(F("Connection failed"));
    return;
  }

  Serial.println(F("Connected!"));

  // Send HTTP request
  client.println("GET " + URI + " HTTP/1.0");
  client.println("Host: " + (String)Hostname);
  client.println("Connection: close");
  if (client.println() == 0) {
    Serial.println("Failed to send request");
    client.stop();
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.0 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    client.stop();
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    client.stop();
    return;
  }

  // Allocate the JSON document
  StaticJsonDocument<768> doc;

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    client.stop();
    return;
  }

  const char* datetime = doc["datetime"];

  Serial.println(datetime);

  // Disconnect
  client.stop();
}

void loop() {
  // not used in this example
}

void wifi_connect()
{


  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
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
