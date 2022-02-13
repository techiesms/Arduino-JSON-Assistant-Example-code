#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

const char* ssid     = "SSID";
const char* password = "PASS";


const char * Hostname = "worldtimeapi.org";
String URI = "/api/timezone/asia/kolkata";
const uint16_t port = 80;

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  while (!Serial) continue;

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
    
  // Clear the buffer
  display.clearDisplay();


  Serial.print("Connecting to ");
  Serial.println(ssid);
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Conneting...");
  display.display();      // Show initial text


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
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connected");
  display.display();



}

void loop() {

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


  String datetime = doc["datetime"]; // 2022-02-11T15:21:21.732473+05:30

  int Day = doc["day_of_week"];

  Serial.println(datetime);
  Serial.println(Day);

  // Disconnect
  client.stop();



  /***************************************************************************
      FOR TIME
  ***************************************************************************/

  String Time = datetime.substring(11, 16);
  String Hour = datetime.substring(11, 13);

  //24 hour to 12 hour conversion
  int h = Hour.toInt();
  if (h > 12)
    h = h - 12;
  else
    h = h;
  Serial.println(h);
  Hour = (String)h;
  Time = Time.substring(2);
  if (h > 10)
    Time = Hour + Time;
  else
    Time = "0" + Hour + Time;
  Serial.print("time - "); Serial.println(Time);

  /***************************************************************************
    FOR DATE
  ***************************************************************************/

  String Date = datetime.substring(8, 10);
  String    Day_str = "";

  if (Day == 0)
    Day_str = "Sun";
  if (Day == 1)
    Day_str = "Mon";
  if (Day == 2)
    Day_str = "Tue";
  if (Day == 3)
    Day_str = "Wed";
  if (Day == 4)
    Day_str = "Thu";
  if (Day == 5)
    Day_str = "Fri";
  if (Day == 6)
    Day_str = "Sat";

  Serial.print("date - "); Serial.println(Date);
  Serial.print("day - "); Serial.println(Day_str);


  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(40, 0);
  display.println(Time);
  display.setCursor(30, 15); 
  display.print(Date);
  display.print(",");
  display.println(Day_str);
  display.display();


  delay(60000);
  // not used in this example
}
