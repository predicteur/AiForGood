/* Option de compilation et téléversement :
      ESP     -> Type de carte : Node MCU 1.0(ESP-12E Module) / Options : 80 MHz, Flash, 4M (3M SPIFFS), v2 Lower Memory, Disabled, None, Only Sketch / programmateur : Arduino as ISP / Outils : ESP8266 Sketch Data Upload (envoi des fichiers Data)
      MKR1200 -> Type de carte : ARDUINO MKR FOX 1200 / programmateur : ATMEL-ICE
*/
//---------------------------------------------include  --------------------------------------------------------------------------------
#include "parametre.h"                                            // Déclaration de constantes, librairies et de structures de données
//---------------------------------------------variables--------------------------------------------------------------------------------
  // variables périodicité
  float   temps_ref_envoi   = 0;                                  // compteur de temps principal de 0 à TEMPS_CYCLE
  float   temps_ref_mesure  = 0;                                  // compteur de temps intermédiaire de 0 à TEMPS_CYCLE/NB_MESURE
  // variables mesures
  Mesure mes[NB_MES];                                             // tableau des informations liées à une mesure
  double  pm[NB_MES];                                             // mesure de PM2.5 et PM10
  // variables ressenti
  int     sensorVal1;                                             // 0 : rien, 1 : ok
  int     sensorVal2;                                             // 0 : rien, 1 : moyen
  int     sensorVal3;                                             // 0 : rien, 1 : pas bon
#ifdef COMPRESSION
  // variables series de mesures
  Mesure mesEnvoi[NB_MES][TAILLE_ECH];                            // tableau des mesures à envoyer
  CoefReg coef      = {0.0, 0.0, 0.0};                            // paramètre de chaque régression unitaire
  CoefComp coefc    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefc : paramètres issue de la compression ou de l'optimisation
  CoefComp coefp    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefp : paramètres issus du decodage
  CoefCode coefi    = {0, 0, 0.0, {}, {}, 0.0};                   // paramètre de chaque régression unitaire
  int nombre_mesure = 0;                                          // compteur de mesures de 0 à TAILLE_ECH
  float y0init[TAILLE_ECH], y0n[TAILLE_ECH], y0fon[TAILLE_ECH];   // y0init : mesures à compresser, Y0n : mesures normalisées (entre 0 et 1), Y0fon : mesures issues de la compressipon
#endif
#ifdef BOARDSIGFOX
  // variables sigfox
  SigfoxMessage payload;                                          // message envoyé par sigfox (12 octets découpés en trois variables de 4 octets)
  int lastMessageStatus;                                          // message de retour d'un envoi 
#endif
//---------------------------------------------initialisation--------------------------------------------------------------------------------
  // initialisation des objets
  Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);      // afficheur LED associé au capteur
#ifdef BOARDSIGFOX
  SdsDustSensor sds(Serial1);                                             // capteur utilisé avec liaison série intégrée
#else
  SdsDustSensor sds(RXPIN, TXPIN);                                        // capteur utilisé avec liaison SPI
#endif
#ifdef RESEAUWIFI
  ESP8266WebServer server(80);                                            // ecoute sur le port 80
#endif
//---------------------------------------------setup--------------------------------------------------------------------------------
  void setup() {
  
    // initialisation liaison série
    Serial.begin(115200);                   
    delay(10); Serial.println('\n');
      
    // initialisation données et affichage
    CreerMesure();                                              // Création pm10 et pm2_5
    strip.begin();                                              // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();                                               // Turn OFF all pixels
    StripAffiche("démarrage");
  
    // initialisation capteur PM
    Serial.println(sds.queryFirmwareVersion().toString());       // prints firmware version
    Serial.println(sds.setActiveReportingMode().toString());     // ensures sensor is in 'active' reporting mode
    Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended
    sds.begin();
#ifdef BOARDSIGFOX
    // initialisation Sigfox
    if (!SigFox.begin()) {
      reboot();
    }
    SigFox.end();
    if (oneshot) {
      SigFox.debug();
    }
#endif
#ifdef RESEAUWIFI
    // initialisation serveur de fichier
    if (!SPIFFS.begin()){
      Serial.println("SPIFFS Mount failed");
    } else {
      Serial.println("SPIFFS Mount succesfull");
    }
  
    // initialisation Serveur WiFi
    WiFi.mode(WIFI_AP_STA);                                       // mode mixte server et client
    WiFiManager wifiManager;                                      // WiFi option non fixe
    if (MEMIDENTIFIANT == 0){wifiManager.resetSettings();}        //garder en mémoire ou non les anciens identifiants
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
#endif    
    StripAffiche("démarré");
  }
//---------------------------------------------boucle--------------------------------------------------------------------------------
  void loop() {
    // boucle de réalisation des mesures intermédiaires
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
#ifdef COMPRESSION
    // boucle d'envoi des données
    if ( nombre_mesure >= TAILLE_ECH ){
        nombre_mesure = 0;      
        genereSerie();
        prSerie(y0init, TAILLE_ECH, "y0init");
        compress();
        prSerie(y0fon, TAILLE_ECH, "y0fon apres compress");
        envoiSigfox();
    }
#endif
#ifdef RESEAUWIFI
    // traitement des requetes des pages web
    WiFiClient client;
    server.handleClient();
#endif
  }
