  /* Option de compilation et téléversement :
      ESP     -> Type de carte : Node MCU 1.0(ESP-12E Module) / Options : 80 MHz, Flash, 4M (3M SPIFFS), v2 Lower Memory, Disabled, None, Only Sketch / programmateur : Arduino as ISP / Outils : ESP8266 Sketch Data Upload (envoi des fichiers Data)
      MKR1200 -> Type de carte : ARDUINO MKR FOX 1200 / programmateur : ATMEL-ICE
*/

//#define BLYNK_PRINT Serial
//#include <BlynkSimpleEsp8266.h>

//---------------------------------------------include  --------------------------------------------------------------------------------
#include "parametre.h"                                            // Déclaration de constantes, librairies et de structures de données
//---------------------------------------------variables--------------------------------------------------------------------------------
  // variables générales
  String  modeFonctionnement= "normal";                           // 3 modes : "normal", "veille", "economie"
  String  theme             = "Global  ";
  // variables affichage LED
  int     niveauAffichage   = NIVEAU_MOYEN;                       // voir les niveaux dans parametre.h
  float   mesValeurLED      = 0;                                  // valeur issue des mesures à afficher sur la LED
  boolean niveauBatterieBas = false;                              // utilisé pour l'affichage LED
  // variables périodicité
  float   temps_ref_mesure  = 0;                                  // compteur de temps intermédiaire de 0 à TEMPS_CYCLE/NB_MESURE
  unsigned long timerVeille = 0;                                  // compteur de temps de clignotement en veille
  int     nbMesureGroupe    = 0;                                  // compteur de mesures pour envoi groupé Sigfox de 0 à TAILLE_ECH
  int     nbMesureElem      = 0;                                  // compteur de mesures élémentaires dans le temps de cycle pour calcul du niveau de qualité
  DateRef dateRef           = {0, 0, 0};
  // variables mesures et capteurs
  //struct WorkingStateResult etatSDS;                                     // état de fonctionnement du capteur SDS
  Mesure  mes[NB_MES];                                            // tableau des informations liées à une mesure
  double  pm [NB_MES];                                            // mesure de PM2.5 et PM10
  String  ressenti          = "normal";                           // 3 états : "bien", "normal", "pasbien"
#ifdef RESEAUWIFI
  // variables envois wifi
  File    ficMes;                                                 // fichier de stockage temporaire des mesures non envoyées
  String  JSONmessage;                                            // message JSON envoyé au serveur
  String  payload;                                                // message JSON retourné par le serveur 
  StaticJsonDocument<TAILLE_MAX_JSON> root;                       // taille à ajuster en fonction du nombre de caractères à envoyer
  String  token;
  boolean token_presence = false;
#endif
#ifdef COMPRESSION
  // variables series de mesures
  Mesure   mesEnvoi[NB_MES][TAILLE_ECH];                          // tableau des mesures à envoyer
  CoefReg  coef     = {0.0, 0.0, 0.0};                            // paramètre de chaque régression unitaire
  CoefComp coefc    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefc : paramètres issue de la compression ou de l'optimisation
  CoefComp coefp    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefp : paramètres issus du decodage
  CoefCode coefi    = {0, 0, 0.0, {}, {}, 0.0};                   // paramètre après codage
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
#ifdef RESEAUWIFI
  ADC_MODE(ADC_VCC);                                                      // autorise la lecture de tension (non compatible avec l'utilisation de A0 qui doit rester en l'air) 
  ESP8266WebServer server(80);                                            // ecoute sur le port 80
#endif
#ifdef BOARDSIGFOX
  SdsDustSensor sds(Serial1);                                             // capteur utilisé avec liaison série intégrée
#else
  SdsDustSensor sds(RXPIN, TXPIN);                                        // capteur utilisé avec liaison SPI
#endif
//---------------------------------------------setup--------------------------------------------------------------------------------
  void setup() {
  
    theme = "setup   ";
    // initialisation liaison série
    Serial.begin(115200);                   
    delay(10); Serial.println('\n');
      
    // initialisation données et affichage
    CreerMesure();                                              // Création pm10 et pm2_5
    strip.begin();                                              // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();                                               // Turn OFF all pixels
    StripAffiche("démarrage");
  
    // initialisation capteur PM
    theme = "capteur ";
    sds.begin();
    Log(3, sds.queryFirmwareVersion().toString(), "");          // prints firmware version
    Log(3, sds.setActiveReportingMode().toString(), "");        // ensures sensor is in 'active' reporting mode
    Log(3, sds.setContinuousWorkingPeriod().toString(), "");    // ensures sensor has continuous working period - default but not recommended
    //WorkingStateResult etatSDS = sds.wakeup();                   // initialisation de etat SDS
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
    theme = "ESP     ";
    // initialisation serveur de fichier
    if (!SPIFFS.begin()){
      Log(2, "SPIFFS Mount failed", "");
    } else {
      Log(3, "SPIFFS Mount succesfull", "");
    }
    ficMes = SPIFFS.open(FIC_BUF, "w");                           // pour vider le fichier des mesures en attente
    ficMes.println("vide");                                       // pour vider le fichier des mesures en attente
    ficMes.close();                                               // pour vider le fichier des mesures en attente

    // initialisation Serveur WiFi
    WiFi.mode(WIFI_AP_STA);                                       // mode mixte server et client
    WiFiManager wifiManager;                                      // WiFi option non fixe
    if (MEM_IDENTIFIANT == 0){wifiManager.resetSettings();}        // garder en mémoire ou non les anciens identifiants
    wifiManager.autoConnect(AUTO_CONNECT);                        // connexion automatique au réseau précédent si MEMIDENTIFIANT = 1
    wifiManager.setDebugOutput(String(MODE_LOG) == String("debug"));
    //wifiManager.autoConnect("Freebox-Lilith", "youwontforgetmyname");      
    theme = "wifi    ";
    Log(3, "Connected to : " + WiFi.SSID(), WiFi.psk());
    Log(0, "IP address : ", WiFi.localIP().toString());
  
    // serveur web
    server.on("/mesures.json", HTTP_GET, SendMesureWeb);
    server.serveStatic("/", SPIFFS, "/index.html");
    server.serveStatic("/index.html", SPIFFS, "/index.html");
    server.begin();
#endif    
    StripAffiche("controleur démarré");
    Log(0, "controleur demarre en mode : MODE_LOG = ", String(MODE_LOG));
    
    //Blynk.begin("YF4nOYISynxxazzjW8aXMS1CrB3-H_B5", "Freebox-Lilith", "youwontforgetmyname");
  }
//---------------------------------------------boucle--------------------------------------------------------------------------------
  void loop() {
    
#ifdef RESEAUWIFI
    theme = "wifi    ";
    // mise à jour des paramètres(traitement des données du serveur)
    AjustementParametres();
    // mise à jour des paramètres(traitement des requetes des pages web)
    WiFiClient client;
    server.handleClient();
#endif    
    if (modeFonctionnement == "veille") {
      theme = "capteur ";
      StripAffiche("mode veille SDS");
      /*WorkingStateResult etatSDS = sds.sleep();
      if (etatSDS.isWorking()) {
        Log(1, "Probleme de mise en sommeil SDS", "");
      }*/
    } else {
      /*WorkingStateResult etatSDS = sds.wakeup();
      if (!etatSDS.isWorking()) {
        Log(1, "Probleme de reveil SDS", "");
      }*/
      theme = "global  ";
      if (modeFonctionnement == "economie") {
          niveauAffichage = NIVEAU_FAIBLE;
      } else if (modeFonctionnement == "renforce"){
          niveauAffichage = NIVEAU_FORT;        
      } else {
          niveauAffichage = NIVEAU_MOYEN;        
      }
      // boucle de réalisation des mesures intermédiaires
      if ( (millis() - temps_ref_mesure) >= float(TEMPS_CYCLE)/float(NB_MESURE) ) {
          theme = "mesure  ";
          temps_ref_mesure = millis();
          StripAffiche("début mesure");
          LireCapteur();                                      // lecture des données dans pm[]
          CalculMesure();                                     // ajout des valeurs instantanées dans mes.xxx
          StripAffiche("fin mesure");
          nbMesureElem ++;
      }
      // boucle principale de mesure
      if ( nbMesureElem >= NB_MESURE ){
          theme = "mesure  ";
          nbMesureElem = 0;
#ifdef RESEAUWIFI
          // envoi des données stockées à renvoyer
          RepriseEnvoiWifi();                                 // essai de renvoi des mesures du fichier
#endif
          GenereMesure();                                     // calcul de la mesure moyenne dans mes.xxx
          if (MesureOk()) {
              PrMesure(3);                                     
#ifdef RESEAUWIFI
              EnvoiWifi();                                    // envoi sur le serveur (et stockage fichier si KO)
#else
              GroupeMesure();                                 // ajout de la mesure au groupe de mesure à envoyer par Sigfox   
#endif
          } 
          else {
              StripAffiche("mesure non envoyée"); delay(2000);
          }
          niveauBatterieBas = TestBatterieBasse();            // test à mettre en place (mesure)
#ifdef COMPRESSION
          nbMesureGroupe ++;
#endif
          mesValeurLED = mes[M_LED].valeur;
          UpdateLed();                                        // affichge du niveau sur les LED
          InitMesureRessenti();
      }
#ifdef COMPRESSION
      // boucle d'envoi des données groupées
      if ( nbMesureGroupe >= TAILLE_ECH ){
          nbMesureGroupe = 0;      
          GenereGroupe();                                      // préparation des données à compresser avant envoi
          PrSerie(y0init, TAILLE_ECH, "y0init");
          compress();                                          // compression des données
          PrSerie(y0fon, TAILLE_ECH, "y0fon apres compress");
          EnvoiSigfox();                                       // envoi sigfox des données compressées
      }
#endif
    }
    //Blynk.run();
  }
