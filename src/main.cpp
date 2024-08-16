// Load Wi-Fi library
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <pins_arduino.h>

// Replace with your network credentials
const char *ssid = "ESP32-Access-Point";
const char *password = "123456789";

// Set web server port number to 80
WebServer server(80);

StaticJsonDocument<1024> jsonDocument;
char buffer[1024];

// Variable to store the HTTP request
String header;

bool ledStatus;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output = 2;

int getPinMode(int p)
{

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  const int MAX_DIGITAL_PIN_NUMBER = 69;
#else
  const int MAX_DIGITAL_PIN_NUMBER = 19;
#endif

  // Check valid pin number
  if (p > MAX_DIGITAL_PIN_NUMBER)
  {
    return -1;
  }

  // Convert designated Arduino pin to ATMega port and pin
  uint8_t pbit = digitalPinToBitMask(p);
  uint8_t pport = digitalPinToPort(p);

  // Read the ATmega DDR for this port
  volatile uint32_t *pmodereg = portModeRegister(pport);

  // Test the value of the bit for this pin and return
  // 0 if it is reset and 1 if it is set
  return ((*pmodereg & pbit) != 0);
}

void getValue()
{
  jsonDocument.clear();
  JsonObject newObject = jsonDocument.createNestedObject();
  newObject["ledValue"] = ledStatus;
  newObject["ledValue2"] = !ledStatus;
  JsonObject newObject2 = jsonDocument.createNestedObject();
  newObject2["ledValue2"] = !ledStatus;

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void setValue()
{
  if (server.hasArg("plain") == false)
  {
    server.send(300, "text/plain", "You must specify a value Number" + server.args());
  }

  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  ledStatus = jsonDocument["ledValue"];

  // Respond to the client
  server.send(200, "application/json", body);
}

void changeLedValue()
{
  if (server.hasArg("plain") == false)
  {
    server.send(400, "text/plain", "You must specify a value Number" + server.args());
  }

  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  int led = jsonDocument["ledIndex"];
  int mode = getPinMode(led);
  ledStatus = jsonDocument["ledValue"];
  if (mode < 0)
  {
    server.send(409, "text/plain", "Pin number is invalid : " + led);
  }
  else
  {
    if (mode == INPUT)
    {
      server.send(410, "text/plain", "Pin led is not in output : " + led);
    }
    else
    {
      digitalWrite(led, ledStatus);
      server.send(200);
    }
  }
}

void testConnection()
{
  server.send(200);
}

void setup()
{
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/getValue", HTTP_GET, getValue);
  server.on("/setValue", HTTP_POST, setValue);
  server.on("/changeLedValue", HTTP_POST, changeLedValue);
  server.on("/testConnection", HTTP_GET, testConnection);
  server.begin();
}

void loop()
{
  server.handleClient(); // Listen for incoming clients

  if (ledStatus)
  {
    digitalWrite(output, HIGH);
  }
  else
  {
    digitalWrite(output, LOW);
  }
}