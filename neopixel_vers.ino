

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
//#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include "SdsDustSensor.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN D8
#define LED_COUNT 1

#define TEMPS_CYCLE 15000  // période d envoi des mesures en millisecondes
#define NB_MESURE   5      // nombre de mesure élémentaires dans le temps de cycle 
#define VALEUR_MIN_PM 0.0  // limite mini autorisee pour les PM
#define VALEUR_MAX_PM 10000.0   // limite maxi autorisee pour les PM

const int nbMes  = 2; // quantité de mesures à effectuer 

float temps_ref_envoi = 0;
float temps_ref_mesure = 0;
double pm[nbMes];

struct Mesure {
  String nom;
  int nombre;
  int nombreOk;
  double valeur;
  float ecartType;
  float date;
  float valeurMin;
  float valeurMax;
  float tauxErreur;
};
Mesure mes[nbMes];

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int rxPin = 14;
int txPin = 12;
SdsDustSensor sds(rxPin, txPin);
String readString;
/////////////////////////////////////////////////////////////
const char *ssid = "Philippe's iPhone";
const char *password = "thomyphi";
////////////////////////////////////////////////////////////
const char *ssid1 = "";
const char *password1 = "";
/////////////////////////////////////////////////////////////
const char *ssid2 = "";
const char *password2 = "";
////////////////////////////////////////////////////////////
const char *ssid3 = "";
const char *password3 = "";
///////////////////////////////////////////////////////////////////
const char *device_name = "sensor9"; // Set your device name !!!!!
///////////////////////////////////////////////////////////////////

const char *host = "http://jsonplaceholder.typicode.com/";
int sensorVal1;
int sensorVal2;
int sensorVal3;
int i;
WiFiServer server(80);
ESP8266WiFiMulti wifiMulti;

void setup() {              //SETUP Start
  Serial.begin(115200);     // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  CreerMesure();           // Création pm10 et pm2_5

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Serveur WiFi
  //WiFi.mode(WIFI_AP_STA);  // mode mixte server et client
  //WiFiManager wifiManager; // WiFi option non fixe
  ////  wifiManager.resetSettings(); //à décommenter pour ne pas gerder en mémoire les anciens identifiants
  //wifiManager.autoConnect("DIAMS plateforme R24");
  //Serial.println(""); Serial.print("IP address: "); Serial.println(WiFi.localIP());

  wifiMulti.addAP(ssid, password); // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid1, password1);
  wifiMulti.addAP(ssid2, password2);
  wifiMulti.addAP(ssid3, password3);
  Serial.println("Connecting ...");
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
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
  Serial.println(WiFi.SSID());     // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());  // Send the IP address of the ESP8266 to the computer

  if (!MDNS.begin("sensor")) {     // Start the mDNS responder for esp8266.local
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

void loop() {                     // Loop start
  WiFiClient client = server.available();

  // réalisation des mesures intermédiaires
  if (( (millis() - temps_ref_mesure) >= float(TEMPS_CYCLE)/float(NB_MESURE) ) and (NB_MESURE > 1)) {
      temps_ref_mesure = millis();
      LireCapteur();
      CalculMesure();
      //Serial.println("mesure");  
      //Serial.println(mes[0].valeur, mes[0].ecartType);
  }

  // boucle principale de mesure
  if ( (millis() - temps_ref_envoi) >= float(TEMPS_CYCLE) ){
      temps_ref_envoi = millis();
      if (NB_MESURE <= 1) {
          LireCapteur(); 
          CalculMesure();
      }
      GenereMesure();
      TraitementMesure(sensorVal1, sensorVal2, sensorVal3);  //envoi des données et affichage LED
      InitMesure();
  }

  if (!client) {
    return;
  } 
  else {
    
    // récupération sur le client du ressenti
    MDNS.update();
    delay(500);
    String req = client.readStringUntil('\r');  // Read the first line of HTTP request
    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    IPAddress ip = WiFi.localIP();
    if (addr_start == -1 || addr_end == -1) {
      Serial.print("Invalid request: ");
      Serial.println(req);
      return;
    }
    req = req.substring(addr_start + 1, addr_end);
    Serial.print("Request: ");
    Serial.println(req);
    TraitementRessenti(req);

    // rafraichissement des valeurs de PM sur le client
    String page= "";
    GenerationPage(page);
    client.print(page);
    //  delay(100);
    client.flush();
  }
}

