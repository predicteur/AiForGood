/* 
  Option de compilation et téléversement :
      ESP     -> Type de carte : Node MCU 1.0(ESP-12E Module) / Options : 80 MHz, Flash, 4M (3M SPIFFS), v2 Lower Memory, Disabled, None, Only Sketch / programmateur : Arduino as ISP / Outils : ESP8266 Sketch Data Upload (envoi des fichiers Data)
      MKR1200 -> Type de carte : ARDUINO MKR FOX 1200 / programmateur : ATMEL-ICE
*/
//#define BLYNK_PRINT Serial
//#include <BlynkSimpleEsp8266.h>

//---------------------------------------------include  --------------------------------------------------------------------------------
#include "parametre.h"                                            // Déclaration de constantes, librairies et de structures de données
//---------------------------------------------variables--------------------------------------------------------------------------------
  // variables générales
  String  theme             = "Global  ";                         // précision utilisée pour les logs
  String  modeFonctionnement= MODE_NORMAL;                        // voir valeur initiale dans parametre.h
  String  modeLog           = MODE_LOG;                           // voir valeur initiale dans parametre.h
  int     tempsCycle        = TEMPS_CYCLE;                        // voir valeur initiale dans parametre.h
  int     bleu  [3]         = {BLEU[0],    BLEU[1],  BLEU[2]};
  int     rouge [3]         = {ROUGE[0],  ROUGE[1], ROUGE[2]};
  int     vert  [3]         = {VERT[0],    VERT[1],  VERT[2]};
  int     orange[3]         = {ORANGE[0],ORANGE[1],ORANGE[2]};
  int     violet[3]         = {VIOLET[0],VIOLET[1],VIOLET[2]};

  // variables affichage LED
  int     mesureLED         = M_LED;                              // voir valeur initiale dans parametre.h
  int     niveauAffichage   = NIVEAU_MOYEN;                       // voir les niveaux dans parametre.h
  int     niveauFort        = NIVEAU_FORT;                        // voir valeur initiale dans parametre.h
  int     niveauMoyen       = NIVEAU_MOYEN;                       // voir valeur initiale dans parametre.h
  int     niveauFaible      = NIVEAU_FAIBLE;                      // voir valeur initiale dans parametre.h
  float   mesValeurLED      = 0;                                  // valeur issue des mesures à afficher sur la LED
  int     niveauBatterie    = 100;                                // 
  boolean niveauBatterieBas = false;                              // 
  // variables périodicité
  float   temps_ref_mesure  = 0;                                  // compteur de temps intermédiaire de 0 à tempsCycle/NB_MESURE pour les mesures
  float   temps_ref_parametre = 0;                                // compteur de temps intermédiaire de 0 à tempsCycle/NB_MESURE pour les paramètres
  unsigned long timerVeille = 0;                                  // compteur de temps de clignotement en veille
  int     nbMesureGroupe    = 0;                                  // compteur de mesures pour envoi groupé Sigfox de 0 à TAILLE_ECH
  int     nbMesureElem      = 0;                                  // compteur de mesures élémentaires dans le temps de cycle pour calcul du niveau de qualité
  DateRef dateRef           = {0, 0, 0};
  // variables mesures et capteurs
  //struct WorkingStateResult etatSDS;                                     // état de fonctionnement du capteur SDS
  Mesure  mes[NB_MES];                                            // tableau des informations liées à une mesure
  double  pm [NB_MES];                                            // mesure de PM2.5 et PM10
  String  ressenti          = "normal";                           // 3 états : "bien", "normal", "pasbien"
#if GPS
  String  longitude         = "";
  String  latitude          = "";
#endif
#ifdef RESEAUWIFI
  File    ficMes;                                                 // fichier de stockage temporaire des mesures non envoyées
  String        tokenValeur;
  boolean       tokenPresence = false;
  unsigned long tokenExpire = 0;
#endif
#ifdef COMPRESSION                                                // variables series de mesures
  Mesure   mesEnvoi[NB_MES][TAILLE_ECH];                          // tableau des mesures à envoyer
  CoefReg  coef     = {0.0, 0.0, 0.0};                            // paramètre de chaque régression unitaire
  CoefComp coefc    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefc : paramètres issue de la compression ou de l'optimisation
  CoefComp coefp    = {0.0, 0.0, 0.0, {}, {}, 0.0};               // coefp : paramètres issus du decodage
  CoefCode coefi    = {0, 0, 0.0, {}, {}, 0.0};                   // paramètre après codage
  float y0init[TAILLE_ECH], y0n[TAILLE_ECH], y0fon[TAILLE_ECH];   // y0init : mesures à compresser, Y0n : mesures normalisées (entre 0 et 1), Y0fon : mesures issues de la compressipon
#endif
#ifdef BOARDSIGFOX
  SigfoxMessage payload;                                          // message envoyé par sigfox (12 octets découpés en trois variables de 4 octets)
  int lastMessageStatus;                                          // message de retour d'un envoi 
#endif
#if Z14A
  byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};  // donnees pour le Z14A
  char reponse[9];
#endif
//--------------------------------------------- initialisation des objets --------------------------------------------------------------------------------
  Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);      // afficheur LED associé au capteur
#ifdef RESEAUWIFI
  ADC_MODE(ADC_VCC);                                                      // autorise la lecture de tension (non compatible avec l'utilisation de A0 qui doit rester en l'air) 
  ESP8266WebServer server(80);                                            // ecoute sur le port 80
#endif
#if SDS
 #ifdef BOARDSIGFOX
  SdsDustSensor sds(Serial1);                                             // capteur utilisé avec liaison série intégrée (à revoir sur ESP)
 #else
  SdsDustSensor sds(RXPIN, TXPIN);                                        // capteur utilisé avec liaison SPI
 #endif
#endif
#if GPS
  TinyGPSPlus gps;
  SoftwareSerial GpsSerial(RXGPS, SW_SERIAL_UNUSED_PIN);                  // (RX, TX)
#endif
#if NEXTPM
  PMS pms(Serial1);
#endif
#if CCS811
  Adafruit_CCS811 ccs;
#endif
/*
 ****************************************************************************************************************
             Setup 
 ****************************************************************************************************************
*/
  void setup() {
  
    theme = "setup   ";
    Serial.begin(115200);                                       // initialisation liaison série
    delay(10); Serial.println('\n');
                                                                // initialisation données et affichage
    CreerMesure();                                              // Création pm10 et pm2_5
    strip.begin();                                              // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();                                               // Turn OFF all pixels
    StripAffiche("démarrage");
  
    theme = "capteur ";
#if SDS                                                         // initialisation capteur PM
    sds.begin();
    Log(3, sds.queryFirmwareVersion().toString(), "");          // prints firmware version
    Log(3, sds.setActiveReportingMode().toString(), "");        // ensures sensor is in 'active' reporting mode
    Log(3, sds.setContinuousWorkingPeriod().toString(), "");    // ensures sensor has continuous working period - default but not recommended
    //WorkingStateResult etatSDS = sds.wakeup();                // initialisation de etat SDS
#endif
#if CCS811                                                      // initialisation eCO2/COV     -- à revoir 
    if(!ccs.begin()){
      Serial.println("Failed to start sensor! Please check your wiring.");
      while(1);}
    while(!ccs.available());                                    //calibrate temperature sensor
    float temp = ccs.calculateTemperature();
    ccs.setTempOffset(temp - 25.0);
    Serial.println("calibration effectuée");
#endif
#if GPS                                                         // initialisation GPS  
    GpsSerial.begin(9600);
#endif
#if NEXTPM                                                      // initialisation NextPM  
    Serial1.begin(9600);                                        // UART hardware
#endif
#if Z14A                                                        // initialisation Z14A  
    Serial1.begin(9600);                                        // UART hardware
#endif
#ifdef BOARDSIGFOX                                              // initialisation Sigfox
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
    if (!SPIFFS.begin()){                                         // initialisation serveur de fichier
      Log(2, "SPIFFS Mount failed", "");
    } else {
      Log(3, "SPIFFS Mount succesfull", "");
    }
    /*// vidage du fichier des mesures en attente d'envoi (doit être en commentaires)
    ficMes = SPIFFS.open(FIC_BUF, "w");                           // pour vider le fichier des mesures en attente
    ficMes.println("vide");                                       // pour vider le fichier des mesures en attente
    ficMes.close();                                               // pour vider le fichier des mesures en attente
    */
                                                                  // initialisation Serveur WiFi
    WiFi.mode(WIFI_AP_STA);                                       // mode mixte server et client
    WiFiManager wifiManager;                                      // WiFi option non fixe
    if (MEM_IDENTIFIANT == 0){wifiManager.resetSettings();}       // garder en mémoire ou non les anciens identifiants
    wifiManager.autoConnect(AUTO_CONNECT);                        // connexion automatique au réseau précédent si MEMIDENTIFIANT = 1
    wifiManager.setDebugOutput(String(MODE_LOG) == String("debug"));      // à revoir, marche pas
    theme = "wifi    ";
    Log(3, "Connected to : " + WiFi.SSID(), WiFi.psk());
    Log(0, "IP address : ", WiFi.localIP().toString());
                                                                  // initialisation token et date
    MajToken();
                                                                  // initialisation serveur web
    server.on("/mesures.json", HTTP_GET, SendMesureWeb);
    server.serveStatic("/", SPIFFS, "/index.html");
    server.serveStatic("/index.html", SPIFFS, "/index.html");
    server.serveStatic("/StyleIndex.css", SPIFFS, "/StyleIndex.css");
    server.begin();
#endif    
    StripAffiche("controleur démarré");
    Log(0, "controleur demarre en mode : MODE_LOG = ", String(MODE_LOG));
    
    //Blynk.begin("YF4nOYISynxxazzjW8aXMS1CrB3-H_B5", "Freebox-Lilith", "youwontforgetmyname");
  }
/*
 ****************************************************************************************************************
             Boucle 
 ****************************************************************************************************************
*/
  void loop() {
    
//--------------------------------------------- mise à jour des paramètres --------------------------------------------------------------------------------                                                                               
#ifdef RESEAUWIFI
    theme = "wifi    ";
    if ( (millis() - temps_ref_parametre) >= float(tempsCycle)/float(NB_MESURE) ) {     // mise à jour des variables(traitement des données du serveur)
      temps_ref_parametre = millis();
      AjustementVariables();
    }
    WiFiClient client;                                                                  // mise à jour des paramètres(traitement des requetes des pages web)
    server.handleClient();
#endif    
//--------------------------------------------- gestion des modes de fonctionnement --------------------------------------------------------------------------------                                                                               
    if (modeFonctionnement == "veille") {
      theme = "capteur ";
      StripAffiche("mode veille SDS");
#if SDS
      /*WorkingStateResult etatSDS = sds.sleep();
      if (etatSDS.isWorking()) {
        Log(1, "Probleme de mise en sommeil SDS", "");
      }*/
#endif
    } else {
#if SDS
      /*WorkingStateResult etatSDS = sds.wakeup();
      if (!etatSDS.isWorking()) {
        Log(1, "Probleme de reveil SDS", "");
      }*/
#endif
      theme = "global  ";
      if (modeFonctionnement == "economie") {
          niveauAffichage = niveauFaible;
      } else if (modeFonctionnement == "renforce"){
          niveauAffichage = niveauFort;        
      } else {
          niveauAffichage = niveauMoyen;        
      }
//--------------------------------------------- mise à jour des coordonnées GPS -------------------------------------------------------------------------------------------                                                                               
#if GPS
      while (GpsSerial.available() > 0) { gps.encode(GpsSerial.read());}
      if (gps.location.isUpdated()) {
        longitude = String(gps.location.lng(), 7);
        latitude  = String(gps.location.lat(), 7);
        Log(4, "Cooordonnées GPS : " + longitude + " " + latitude,"");
      }
#endif
//--------------------------------------------- boucle de réalisation des mesures intermédiaires --------------------------------------------------------------------------------                                                                                     
      if ( (millis() - temps_ref_mesure) >= float(tempsCycle)/float(NB_MESURE) ) {
          theme = "mesure  ";
          temps_ref_mesure = millis();
          StripAffiche("debut mesure");
          LireCapteur();                                      // lecture des données dans pm[]
          CalculMesure();                                     // ajout des valeurs instantanées dans mes.xxx
          StripAffiche("fin mesure");
          nbMesureElem ++;
      }
//--------------------------------------------- boucle principale de mesure --------------------------------------------------------------------------------                                                                               
      if ( nbMesureElem >= NB_MESURE ){
          theme = "mesure  ";
          nbMesureElem = 0;
#ifdef RESEAUWIFI
          RepriseEnvoiWifiData();                             // essai de renvoi des mesures à renvoyer stockées dans SPIFFS
#endif
          GenereMesure();                                     // calcul de la mesure moyenne dans mes.xxx
          if (MesureOk()) {
              PrMesure(3);                                     
#ifdef RESEAUWIFI
              EnvoiWifiData();                                // envoi sur le serveur (et stockage fichier si KO)
#else
              GroupeMesure();                                 // ajout de la mesure au groupe de mesure à envoyer par Sigfox   
#endif
          } 
          else {
              StripAffiche("mesure non envoyée"); delay(2000);
          }
          niveauBatterie = MesureBatterie();                  //  à mettre en place (mesure)
#ifdef COMPRESSION
          nbMesureGroupe ++;
#endif
          mesValeurLED = mes[mesureLED].valeur;
          UpdateLed();                                        // affichge du niveau sur les LED
          InitMesureRessenti();                               // dépend de la gestion du ressenti (maintenu ou non maintenu) -> à clarifier
      }
//--------------------------------------------- boucle d'envoi des données groupées --------------------------------------------------------------------------------                                                                               
#ifdef COMPRESSION
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
