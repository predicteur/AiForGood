#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include "SdsDustSensor.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN D8
#define LED_COUNT 1
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int rxPin = 14;
int txPin = 12;
SdsDustSensor sds(rxPin, txPin);
String readString;
/////////////////////////////////////////////////////////////
const char *ssid = "alabintheair";
const char *password = "aiforgood";
////////////////////////////////////////////////////////////
const char *ssid1 = "SFR-07f0";
const char *password1 = "Q5DYH61QXS2M";
/////////////////////////////////////////////////////////////
const char *ssid2 = "";
const char *password2 = "";
////////////////////////////////////////////////////////////
const char *ssid3 = "";
const char *password3 = "";
///////////////////////////////////////////////////////////////////
const char *device_name = "testbatteryremy"; // Set your device name !!!!!
///////////////////////////////////////////////////////////////////

const char *host = "http://jsonplaceholder.typicode.com/";
double pm2_5;
double pm10;
int sensorVal1;
int sensorVal2;
int sensorVal3;
float temps_ref = 0;
int i;
WiFiServer server(80);
ESP8266WiFiMulti wifiMulti;

void setup() //SETUP Start*

{
  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  wifiMulti.addAP(ssid, password); // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid1, password1);
  wifiMulti.addAP(ssid2, password2);
  wifiMulti.addAP(ssid3, password3);
  Serial.println("Connecting ...");

  while (wifiMulti.run() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above

    Serial.print(++i);
    Serial.print(' ');
    delay(500);
    strip.setPixelColor(0, strip.Color(255, 165, 0));
    strip.show();
    delay(500);
    strip.setPixelColor(0, strip.Color(30, 144, 255));
    strip.show();
  }

  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID()); // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer

  if (!MDNS.begin("sensor"))
  { // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  Serial.println(sds.queryFirmwareVersion().toString());       // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString());     // ensures sensor is in 'active' reporting mode
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended

  MDNS.addService("http", "tcp", 80);
  server.begin();
  sds.begin();
}

void loop() // Loop start
{
  WiFiClient client = server.available();

  if ((millis() - temps_ref) >= 15000)
  {
    pmResult(sensorVal1, sensorVal2, sensorVal3);
  }

  if (!client)
  {
    return;
  }
  else
  {

    MDNS.update();
    delay(500);
    // Read the first line of HTTP request
    String req = client.readStringUntil('\r');

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    IPAddress ip = WiFi.localIP();

    if (addr_start == -1 || addr_end == -1)
    {
      Serial.print("Invalid request: ");
      Serial.println(req);
      return;
    }
    req = req.substring(addr_start + 1, addr_end);
    Serial.print("Request: ");
    Serial.println(req);

    if (req == "/BIEN")
    {
      sensorVal1 = 1;
      sensorVal2 = 0;
      sensorVal3 = 0;
    }
    else if (req == "/NORMAL")
    {
      sensorVal1 = 0;
      sensorVal2 = 1;
      sensorVal3 = 0;
    }
    else if (req == "/PASBIEN")
    {
      sensorVal1 = 0;
      sensorVal2 = 0;
      sensorVal3 = 1;
    }

    String s = "<html lang='fr'><head><meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1'>";
    s += "<meta http-equiv='refresh' name='viewport' content='width=device-width, user-scalable=no'/>";
    s += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'>";
    s += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>";
    s += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";
    s += "</head>";
    s += "<body style='background:url('https://zupimages.net/up/19/29/jblq.jpg') no-repeat center/100% 100%;'>";
    s += "<div class='container container-fluid px-5' style='min-height: 100%;'>";
    s += "<div class='row'>";
    s += "<div class='col-md-12'>";
    s += "<h3 class='text-center'>";
    s += "AlabintheAIR";
    s += "</h3>";
    s += "<br /><br />";
    s += "<hr />";
    s += "<h3 class='text-center'>My FEELING</h3>";
    s += "<br /><br />";
    s += "<div class='row'>";
    s += "<div class='col-sm-12'>";
    s += "<a class='btn btn-success btn-block' style='padding-top:3%;padding-bottom:3%;' href=\"BIEN\">FRESH AIR</a>";
    s += "<br /><br />";
    s += "<a class='btn btn-primary btn-lg btn-block' style='padding-top:3%;padding-bottom:3%;' href=\"NORMAL\">COMMON AIR</a>";
    s += "<br /><br />";
    s += "<a class='btn btn-danger btn-lg btn-block' style='padding-top:3%;padding-bottom:3%;' href=\"PASBIEN\">STALE AIR</a>";
    s += "<br />";
    s += "</div>";
    s += "</div>";
    s += "</div>";
    s += "</div>";
    s += "</div>";
    s += "</body> </html> \n";

    client.print(s);
    //  delay(100);
    client.flush();
  }
}

void sendData(double z, double k, int a, int b, int c)
{

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  String string_a = String(a, 2);
  String string_b = String(b, 2);
  String string_c = String(c, 2);
  String string_z = String(z, 2);
  String string_k = String(k, 2);
  root["device"] = device_name;
  root["PM2_5"] = string_z;
  root["PM10"] = string_k;
  root["mixed_feeling"] = string_b;
  root["negative_feeling"] = string_c;
  root["positive_feeling"] = string_a;

  root.printTo(Serial);
  char JSONmessageBuffer[300];
  root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println(JSONmessageBuffer);

  if (WiFi.status() == WL_CONNECTED)
  { //Check WiFi connection status

    HTTPClient http;                                                    //Declare object of class HTTPClient
    http.begin("http://simple-ai4good-sensors-api.herokuapp.com/data"); //Specify request destination
    http.addHeader("Content-Type", "application/json");                 //Specify content-type header
    int httpCode = http.POST(JSONmessageBuffer);                        //Send the request
    String payload = http.getString();                                  //Get the response payload
    Serial.println(httpCode);                                           //Print HTTP return code
    Serial.println(payload);                                            //Print request response payload
    http.end();                                                         //Close connection
  }
  else
  {
    delay(500);
    strip.setPixelColor(0, strip.Color(255, 165, 0));
    strip.show();
    delay(500);
    strip.setPixelColor(0, strip.Color(30, 144, 255));
    strip.show();
  }
}

void updateLed(double a, double b) //Function UpdateOled Pm value
{
  int myInt = (int)a;
  switch (myInt)
  {
  case 0 ... 10:
    strip.setPixelColor(0, strip.Color(0, 128, 0));
    break;
  case 11 ... 20:
    strip.setPixelColor(0, strip.Color(255, 165, 0));
    break;
  case 21 ... 1000:
    strip.setPixelColor(0, strip.Color(127, 0, 0));
    break;
  }
  strip.show();
}

void pmResult(int a, int b, int c)
{
  PmResult pm = sds.queryPm();
  if (pm.isOk())
  {
    pm2_5 = pm.pm25;
    pm10 = pm.pm10;
    sendData(pm2_5, pm10, a, b, c);
    updateLed(pm2_5, pm2_5);
    temps_ref = millis();
  }
}
