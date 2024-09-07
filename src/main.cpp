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

void getNumberOfMotorWithId()
{
  jsonDocument.clear();
  JsonObject newObject = jsonDocument.createNestedObject();
  int motor = 5;
  newObject["motorNumber"] = motor;
  randomSeed(analogRead(0));
  for (size_t i = 0; i < motor; i++)
  {
    newObject[String(i)] = random(10000, 99999);
  }

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void getNumberOfMotor()
{
  jsonDocument.clear();
  JsonObject newObject = jsonDocument.createNestedObject();
  newObject["motorNumber"] = 4;

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
  if (index == -1)
  {
    server.send(404, "text/plain", "Unknow output pin");
    return;
  }
  ledStatus[index] = jsonDocument["ledValue"];
  String sender = jsonDocument["sender"];

  // print the value to the serial port
  Serial.print("[");
  Serial.print(sender);
  Serial.print("] ");
  Serial.print("Led ");
  Serial.print(led);
  Serial.print(" set to ");
  Serial.println(ledStatus[index]);

  server.send(200);
}

/**
 * API to change the state of a motor
 * JSON send look like :
 * {
 *    motorIndex = 2,
 *    motorPosition = 112,
 *    motorSpeed = [0...1]
 * }
 */
void changeMotorPosition()
{
  if (server.hasArg("plain") == false)
  {
    server.send(400, "text/plain", "You must specify a value Number" + server.args());
  }

  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  int motor = jsonDocument["motorIndex"];
  int position = jsonDocument["motorPosition"];
  int speed = jsonDocument["motorSpeed"];
  String sender = jsonDocument["sender"];

  // print the value to the serial port
  Serial.print("[");
  Serial.print(sender);
  Serial.print("] ");
  Serial.print("Motor ");
  Serial.print(motor);
  Serial.print(" set to ");
  Serial.print(position);
  Serial.print(" with speed ");
  Serial.println(speed);

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
  server.on("/changeMotorPosition", HTTP_POST, changeMotorPosition);
  server.on("/getNumberOfMotorWithId", HTTP_GET, getNumberOfMotorWithId);
  server.on("/getNumberOfMotor", HTTP_GET, getNumberOfMotor);
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