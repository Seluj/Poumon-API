// Load Wi-Fi library
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <pins_arduino.h>

const char *ssid = "Lung-Network";
const char *password = "123456789";

// Set web server port number to 80
WebServer server(80);

StaticJsonDocument<1024> jsonDocument;
char buffer[1024];

const byte nbInput = 1;

byte outputPin[nbInput] = {2};
bool ledStatus[nbInput] = {false};

int getIndex(byte tab[], byte size, byte byteToFind)
{
  byte wantedpos = -1;
  for (byte i = 0; i < size; i++)
  {
    if (byteToFind = tab[i])
    {
      wantedpos = i;
      break;
    }
  }
  return wantedpos;
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

/**
 * API to changed the value of an output
 * JSON send look like :
 * {
 *    ledIndex = 2,
 *    ledValue = true/false,
 * }
 */
void changeLedValue()
{
  if (server.hasArg("plain") == false)
  {
    server.send(400, "text/plain", "You must specify a value Number" + server.args());
  }

  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  int led = jsonDocument["ledIndex"];
  byte index = getIndex(outputPin, nbInput, led);
  if (index == -1) {
    server.send(404, "text/plain", "Unknow output pin");
    return;
  }
  ledStatus[index] = jsonDocument["ledValue"];

  server.send(200);
}

void testConnection()
{
  server.send(200);
}

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < nbInput; i++)
  {
    pinMode(outputPin[i], OUTPUT);
    digitalWrite(outputPin[i], LOW);
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/getValue", HTTP_GET, getValue);
  server.on("/changeLedValue", HTTP_POST, changeLedValue);
  server.on("/testConnection", HTTP_GET, testConnection);
  server.begin();
}

void loop()
{
  server.handleClient(); // Listen for incoming clients

  for (int i = 0; i < nbInput; i++)
  {
    if (ledStatus[i])
    {
      digitalWrite(outputPin[i], HIGH);
    }
    else
    {
      digitalWrite(outputPin[i], LOW);
    }
  }
}