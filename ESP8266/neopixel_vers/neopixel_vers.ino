// Node MCU 1.0(ESP-12E Module), 80 MHz, Flash, 4M (3M SPIFFS), v2 Lower Memory, Disabled, None, Only Sketch, 115200 sur COM4, programmateur : Arduino as ISP

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>
#include "SdsDustSensor.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <FS.h>

#define LED_PIN     D8         // affichage de l'état des mesures
#define RXPIN       14         // laison capteur
#define TXPIN       12         // laison capteur

#define LED_COUNT       1         // nombre de LED dans le ruban
#define TEMPS_CYCLE     15000     // période d envoi des mesures en millisecondes
#define NB_MESURE       5         // nombre de mesure élémentaires dans le temps de cycle 
#define VALEUR_MIN_PM   0.0       // limite mini autorisee pour les PM
#define VALEUR_MAX_PM   1000.0    // limite maxi autorisee pour les PM
#define SEUIL_BON_PM    10.0      // seuil affichage pour les PM
#define SEUIL_MOYEN_PM  20.0      // seuil affichage pour les PM

const int     NB_MES  = 2;               // quantité de mesures à effectuer (ici 2 :PM25 et PM10)
const char    *DEVICE_NAME = "sensor9";  // nom du device a documenter
const String  SERVEUR_AI4GOOD = "http://simple-ai4good-sensors-api.herokuapp.com/data";
const int     ROUGE[3]  = {127, 0, 0};
const int     ORANGE[3] = {255, 165, 0};
const int     BLEU[3]   = {30, 144, 255};
const int     VERT[3]   = {0, 128, 0};
const int     LUMINOSITE_FORTE  = 250;     // maxi 255
const int     LUMINOSITE_FAIBLE = 50;
const char    *AUTO_CONNECT = "AI for GOOD";

float   temps_ref_envoi   = 0;
float   temps_ref_mesure  = 0;
double  pm[NB_MES];
int     sensorVal1;
int     sensorVal2;
int     sensorVal3;

struct Mesure {
  String  nom;        // nom de la mesure ex. PM10
  int     nombre;     // nombre de mesures effectuées dans le temps de cycle
  int     nombreOk;   // nombre de mesures correctes dans le temps de cycle
  double  valeur;     // valeur de la mesure calculée dans le temps de cycle
  float   ecartType;  // écart-type des valeurs bonnes
  float   date;       // date de la mesure
  float   valeurMin;  // valeur min autorisée pour la mesure
  float   valeurMax;  // valeur max autorisée pour la mesure
  float   tauxErreur; // ratio nombre de mesures correctes / nombre de mesures effectuées
};
// initialisation des objets
Mesure mes[NB_MES];
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
SdsDustSensor sds(RXPIN, TXPIN);
ESP8266WebServer server(80);         // ecoute sur le port 80

void setup() {         

  // liaison série
  Serial.begin(115200);     // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  // initialisation données et affichage
  CreerMesure();              // Création pm10 et pm2_5
  strip.begin();              // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();               // Turn OFF all pixels
  StripAffiche("démarrage");

  // Serveur de fichier
  if (!SPIFFS.begin()){
    Serial.println("SPIFFS Mount failed");
  } else {
    Serial.println("SPIFFS Mount succesfull");
  }

  // Serveur WiFi
  WiFi.mode(WIFI_AP_STA);  // mode mixte server et client
  WiFiManager wifiManager; // WiFi option non fixe
  //  wifiManager.resetSettings(); //à décommenter pour ne pas gerder en mémoire les anciens identifiants
  wifiManager.autoConnect(AUTO_CONNECT);
  Serial.println(""); Serial.print("IP address: "); Serial.println(WiFi.localIP());
  Serial.print("Connected to ");  Serial.println(WiFi.SSID()); 
  Serial.print("IP address:\t");  Serial.println(WiFi.localIP());

  // serveur web
  server.on("/mesures.json", SendMesure);
  server.on("/BIEN",    []() {sensorVal1 = 1; sensorVal2 = 0; sensorVal3 = 0; Serial.println("BIEN")   ; server.send(200, "text/plain", "/index.html");});
  server.on("/NORMAL",  []() {sensorVal1 = 0; sensorVal2 = 1; sensorVal3 = 0; Serial.println("NORMAL") ; server.send(200, "text/plain", "/index.html");});
  server.on("/PASBIEN", []() {sensorVal1 = 0; sensorVal2 = 0; sensorVal3 = 1; Serial.println("PASBIEN"); server.send(200, "text/plain", "/index.html");});
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/index.html", SPIFFS, "/index.html");
  server.begin();

  // capteur PM
  Serial.println(sds.queryFirmwareVersion().toString());       // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString());     // ensures sensor is in 'active' reporting mode
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended
  sds.begin();
  
  StripAffiche("démarré");
}

void loop() {
  // réalisation des mesures intermédiaires
  if (( (millis() - temps_ref_mesure) >= float(TEMPS_CYCLE)/float(NB_MESURE) ) and (NB_MESURE > 1)) {
      temps_ref_mesure = millis();
      StripAffiche("début mesure");
      LireCapteur();
      CalculMesure();
      StripAffiche("fin mesure");
  }
  // boucle principale de mesure
  if ( (millis() - temps_ref_envoi) >= float(TEMPS_CYCLE) ){
      temps_ref_envoi = millis();
      if (NB_MESURE <= 1) {
        StripAffiche("début mesure");
        LireCapteur();
        CalculMesure();
        StripAffiche("fin mesure");
      }
      GenereMesure();
      TraitementMesure();  //envoi des données et affichage LED
      InitMesure();
      InitRessenti();
  }
  // traitement des requetes des pages web
  WiFiClient client;
  server.handleClient();
}

